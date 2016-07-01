//******************************************************************************
// Модуль E14-440.
// Консольная программа с организацией потокового вывода данных на ЦАП
// Тип выводимого сигнала: синус с частотой 1.23 кГц и амплитудой 2000 кодов ЦАП
// Вывод осуществляется на частоте 125 кГц.
//******************************************************************************
program WriteData;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

const
	// кол-во отсчетов в запросе (кратное 256) для ф. WriteData
	DataStep : DWORD = 128*1024;
	// номер канала ЦАП для вывода данных: 0 или 1
	DacNumber : WORD = $0;
	// частота вывода данных
	DacRate : double  = 125.0;
	// параметры выводимого синусоидального сигнала
	SignalFrequency : double = 1.23;			// в кГц
	SignalAmplitude : double = 2000.0;		// в кодах ЦАП


type
	TShortrArray = array [0..1] of array of SHORT;

var
	// идентификатор потока вывода
	hWriteThread : THANDLE;
	WriteTid : DWORD;

	// интерфейс модуля E14-440
	pModule : ILE440;
	// версия библиотеки Lusbapi.dll
	DllVersion : DWORD;
	// идентификатор устройства
	ModuleHandle : THandle;
	// название модуля
	ModuleName: String;
	// скорость работы шины USB
	UsbSpeed : BYTE;
	// структура с полной информацией о модуле
	ModuleDescription : MODULE_DESCRIPTION_E440;
	// структура параметров работы АЦП
	dp : DAC_PARS_E440;

	// экранный счетчик-индикатор
	Counter, OldCounter : DWORD;
	// указатель на буфер для данных
	Buffer : TShortrArray;

	// текущее время лдля расчёта сигнала
	CurrentTime : double;				// в млс

	// номер ошибки при выполнения потока вывода данных
	WriteThreadErrorNumber : WORD;
	// флажок завершения потоков вывода данных
	IsWriteThreadComplete : boolean;

	// *** вспомогательные переменные ***
	i : WORD;
	// Дескриптор стандартного устройства ввода компьютера - клавиатура
	InputConsoleHandle : THandle;
	// Дескриптор стандартного устройства вывода компьютера - дисплей
	OutputConsoleHandle : THandle;
	// Для хранения максимальных размеров окна
	MaxX, MaxY : DWORD;
 	// вспомогательная строчка
	Str : string;

//------------------------------------------------------------------------------
// Инициализация глобальных переменных ввода/вывода
//------------------------------------------------------------------------------
procedure InitConsoleHandles;
var
	Coord : TCoord;  // Для хранения/установки позиции экрана
begin
	// получаем дескриптор стандартного устройства ввода компьютера - клавиатура
	InputConsoleHandle := GetStdHandle(STD_INPUT_HANDLE);
	// Получаем дескриптор стандартного устройства вывода компьютера - экран
	OutputConsoleHandle := GetStdHandle(STD_OUTPUT_HANDLE);
	// Получаем максимальные размеры окна
	Coord := GetLargestConsoleWindowSize(OutputConsoleHandle);
	MaxX := Coord.X;
	MaxY := Coord.Y;
end;

//------------------------------------------------------------------------------
// Функция обнаружения нажатия клавиши ESCAPE
//------------------------------------------------------------------------------
function IsEscKeyPressed : boolean;
var
	KbrdBuffer: INPUT_RECORD;
	Count: DWORD;

begin
	Result := false;
	GetNumberOfConsoleInputEvents(InputConsoleHandle, Count);
	if Count > 0 then
		begin
			ReadConsoleInput(InputConsoleHandle, KbrdBuffer, 1, Count);
			case KbrdBuffer.EventType of
				KEY_EVENT:
					case KbrdBuffer.Event.KeyEvent.wVirtualKeyCode of
	            VK_ESCAPE: Result := true;
            // Другие клавиши
          end;
       // Другие типы событий
      end;
    end;
end;

//------------------------------------------------------------------------------
// Установка курсора в координаты X, Y
//------------------------------------------------------------------------------
procedure GotoXY(X, Y : Word);
var
	Coord : TCoord;  // Для хранения/установки позиции экрана
begin
	Coord.X := X; Coord.Y := Y;
	SetConsoleCursorPosition(OutputConsoleHandle, Coord);
end;

//------------------------------------------------------------------------------
// Очистка экрана - заполнение его пробелами
//------------------------------------------------------------------------------
procedure ClearScreen;
var
	Coord : TCoord;	// Для хранения/установки позиции экрана
	NOAW  : DWORD;		// Для хранения результатов некоторых функций
begin
	Coord.X := 0; Coord.Y := 0;
	FillConsoleOutputCharacter(OutputConsoleHandle, ' ', MaxX*MaxY, Coord, NOAW);
	GotoXY(0, 0);
end;

//------------------------------------------------------------------------------
// ожидание завершения выполнения очередного запроса на вывод данных
//------------------------------------------------------------------------------
function WaitingForRequestCompleted(var WriteOv : OVERLAPPED) : boolean;
var
	BytesTransferred : DWORD;
begin
	Result := true;
	while true do
	   begin
			if GetOverlappedResult(ModuleHandle, WriteOv, BytesTransferred, FALSE) then break
			else if (GetLastError() <>  ERROR_IO_INCOMPLETE) then
				begin
					// ошибка ожидания вывода очередной порции данных
					WriteThreadErrorNumber := 3; Result := false; break;
				end
			else if IsEscKeyPressed() then
				begin
					// программа прервана (нажата клавиша ESC)
					WriteThreadErrorNumber := 4; Result := false; break;
				end
			else Sleep(20);
		end;
end;

//------------------------------------------------------------------------------
// функция формирование первой порции данных синусоидального
// сигнала и заполнение ею FIFO буфера ЦАП в DSP модуля
//------------------------------------------------------------------------------
function FillDacBufer : boolean;
var
	i : WORD;
begin
	Result := true;
	// формируем данные для целого FIFO буфера вывода
	for i := 0 to (dp.DacFifoLength-1) do
		begin
			Buffer[0][i] := Round(SignalAmplitude * sin(2.0*pi*SignalFrequency*CurrentTime));
			Buffer[0][i] := Buffer[0][i] and WORD($0FFF);
			Buffer[0][i] := Buffer[0][i] or (DacNumber shl 12);
			CurrentTime := CurrentTime + 1.0/dp.DacRate;
		end;
	// заполняем целиком FIFO буфер вывода в DSP модуля
	if not pModule.PUT_DM_ARRAY(dp.DacFifoBaseAddress, dp.DacFifoLength, Pointer(Buffer[0])) then
//	if not pModule.PUT_DM_ARRAY(dp.DacFifoBaseAddress, dp.DacFifoLength, @Buffer[0][0]) then
									begin WriteThreadErrorNumber := 7; Result := false; end;
end;

//------------------------------------------------------------------------------
// формирование очередной порции данных синусоидального сигнала
//------------------------------------------------------------------------------
procedure MakeDacData(RequestNumber : WORD);
var
	i : DWORD;
begin
	for i := 0 to (DataStep-1) do
		begin
			Buffer[RequestNumber][i] := Round(SignalAmplitude * sin(2.0*pi*SignalFrequency*CurrentTime));
			Buffer[RequestNumber][i] := Buffer[RequestNumber][i] and WORD($0FFF);
			Buffer[RequestNumber][i] := Buffer[RequestNumber][i] or (DacNumber shl 12);
			CurrentTime := CurrentTime + 1.0/dp.DacRate;
		end;
end;

//------------------------------------------------------------------------------
//      фукция запускаемая в качестве отдельного потока
//         для вывода данных на ЦАП модуля E14-440
//------------------------------------------------------------------------------
function WriteThread(var param : pointer): DWORD;
var
	i : WORD ;
	RequestNumber : WORD;
	// массив OVERLAPPED структур из двух элементов
	ReadOv : array[0..1] of OVERLAPPED;
	// массив структур с параметрами запроса на ввод/вывод данных
	IoReq : array[0..1] of IO_REQUEST_LUSBAPI;

begin
	Result := 0;
	// остановим работу ЦАП и одновременно сбросим USB-канал записи данных
	if not pModule.STOP_DAC() then	begin	WriteThreadErrorNumber := 1; IsWriteThreadComplete := true; exit; end;

	// заполняем FIFO буфер ЦАП сформированными данными сигнала
	if not FillDacBufer() then begin	IsWriteThreadComplete := true; exit; end;

	// формируем необходимые для вывода данных структуры
	for i := 0 to 1 do
		begin
			// инициализация структуры типа OVERLAPPED
			ZeroMemory(@ReadOv[i], sizeof(OVERLAPPED));
			// создаём событие для асинхронного запроса
			ReadOv[i].hEvent := CreateEvent(nil, FALSE , FALSE, nil);
			// формируем структуру IoReq
			IoReq[i].Buffer := Pointer(Buffer[i]);
			IoReq[i].NumberOfWordsToPass := DataStep;
			IoReq[i].NumberOfWordsPassed := 0;
			IoReq[i].Overlapped := @ReadOv[i];
			IoReq[i].TimeOut := Round(Int(DataStep/dp.DacRate)) + 1000;
		end;

	// формируем в буфере первую порцию данных для вывода на ЦАП
	MakeDacData($0);
	// а также же и вторую порцию данных для вывода на ЦАП
	MakeDacData($1);

	// заранее закажем первый асинхронный вывод данных на ЦАП
	RequestNumber := 0;
	if not pModule.WriteData(@IoReq[RequestNumber]) then
		begin
			CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent); WriteThreadErrorNumber := 2; IsWriteThreadComplete := true; exit;
		end;

	// а теперь можно запускать вывод данных на ЦАП
	if pModule.START_DAC() then
   	begin
			// перманентный цикл вывода данных на ЦАП
			while true do
				begin
					RequestNumber := RequestNumber xor $1;
					// сделаем запрос на вывод очередной порции данных
					if not pModule.WriteData(@IoReq[RequestNumber]) then
						begin
							WriteThreadErrorNumber := 2; break;
						end;

					// ожидание завершения запроса на вывод очередной порции данных
					if not WaitingForRequestCompleted(IoReq[RequestNumber xor $1].Overlapped^) then break;
					// формируем очередную порцию данных для вывода на ЦАП
					MakeDacData(RequestNumber xor $1);

					// были ли ошибки или пользователь прервал вывод данных
					if WriteThreadErrorNumber <> 0 then break
					// была ли программа прервана (нажата клавиша ESC)?
					else if IsEscKeyPressed() then begin WriteThreadErrorNumber := 4; break; end
					// небольшая задержечка               
					else Sleep(20);
					// увеличиваем счётчик выведенныхых блоков данных
					Inc(Counter);
				end
		end
	else WriteThreadErrorNumber := 5;

	// остановим вывод данных
	if not pModule.STOP_DAC() then WriteThreadErrorNumber := 1;
	// если надо, то прервём незавершённый асинхронный запрос
	if not CancelIo(ModuleHandle) then WriteThreadErrorNumber := 6;
	// освободим идентификаторы событий
	CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent);
	// задержечка   
	Sleep(100);   
	// установим флажок окончания потока вывода данных
	IsWriteThreadComplete := true;

end;

//------------------------------------------------------------------------------
// аварийное завершение программы
//------------------------------------------------------------------------------
procedure AbortProgram(ErrorString: string; AbortionFlag : bool = true);
var
	i : WORD ;
	// однократный отсчёт для ЦАП
	DacSample : SHORT;
begin
	// освободим интерфейс модуля
	if pModule <> nil then
		begin
			// выведем нулевой отсчёт на первый канал ЦАП
			DacSample := 0;
			if not pModule.DAC_SAMPLE(@DacSample, 0) then WriteLn(' DAC_SAMPLE() --> Bad')
			// а также на второй канал ЦАП
			else if not pModule.DAC_SAMPLE(@DacSample, 1) then WriteLn(' DAC_SAMPLE() --> Bad');

			// освободим интерфейс модуля
			if not pModule.ReleaseLInstance() then  WriteLn(' ReleaseLInstance() --> Bad')
			else WriteLn(' ReleaseLInstance() --> OK');
			// обнулим указатель на интерфейс модуля
			pModule := nil;
		end;

	// освободим идентификатор потока сбора данных
	if hWriteThread = THANDLE(nil) then CloseHandle(hWriteThread);
	// освободим память из-под буферов данных
	for i := 0 to 1 do Buffer[i] := nil;

	// если нужно - выводим сообщение с ошибкой
	if ErrorString <> ' ' then MessageBox(HWND(nil), pCHAR(ErrorString), 'ОШИБКА!!!', MB_OK + MB_ICONINFORMATION);
	// если нужно - аварийно завершаем программу
	if AbortionFlag = true then halt;
end;

//------------------------------------------------------------------------------
// отображение ошибок возникших во время работы потока вывода данных
//------------------------------------------------------------------------------
procedure ShowThreadErrorMessage;
begin
	case WriteThreadErrorNumber of
		$0 : ;
		$1 : WriteLn(' DAC Thread: STOP_DAC() --> Bad! :(((');
		$2 : WriteLn(' DAC Thread: WriteData() --> Bad :(((');
		$3 : WriteLn(' DAC Thread: Waiting data Error! :(((');
		// если программа была злобно прервана, предъявим ноту протеста
		$4 : WriteLn(' DAC Thread: The program was terminated! :(((');
		$5 : WriteLn(' DAC Thread: START_DAC() --> Bad :(((');
		$6 : WriteLn(' DAC Thread: Can''t complete input and output (I/O) operations! :(((');
		$7 : WriteLn(' DAC Thread: Can''t fill FIFO DAC buffer in DSP! :(((');
		else WriteLn(' DAC Thread: Unknown error! :(((');
	end;
end;

//******************************************************************************
//					  	О С Н О В Н А Я    П Р О Г Р А М М А
//******************************************************************************
begin
	// сбросим флаги ошибки потока ввода
	WriteThreadErrorNumber := 0;
	// сбросим флажок завершённости потока сбора данных
	IsWriteThreadComplete := false;
	// сбросим счётчики
	Counter := $0; OldCounter := $FFFF;

	// инициализация дескрипторов ввода и вывода
	InitConsoleHandles();
	// очистим экран дисплея
	ClearScreen();

	// проверим версию используемой DLL библиотеки
	DllVersion := GetDllVersion;
	if DllVersion <> CURRENT_VERSION_LUSBAPI then
		begin
			Str := 'Неверная версия DLL библиотеки Lusbapi.dll! ' + #10#13 +
					 '           Текущая: ' + IntToStr(DllVersion shr 16) +  '.' + IntToStr(DllVersion and $FFFF) + '.' +
					 ' Требуется: ' + IntToStr(CURRENT_VERSION_LUSBAPI shr 16) + '.' + IntToStr(CURRENT_VERSION_LUSBAPI and $FFFF) + '.';
			AbortProgram(Str);
		end
	else WriteLn(' DLL Version --> OK');

	// попробуем получить указатель на интерфейс для модуля E14-440
	pModule := CreateLInstance(pCHAR('e440'));
	if pModule = nil then AbortProgram('Не могу найти интерфейс модуля E14-440!')
	else WriteLn(' Module Interface --> OK');

	// попробуем обнаружить модуль E14-440 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for i := 0 to (MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI-1) do if pModule.OpenLDevice(i) then break;
	// что-нибудь обнаружили?
	if i = MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI then AbortProgram('Не удалось обнаружить модуль E14-440 в первых 127 виртуальных слотах!')
	else WriteLn(Format(' OpenLDevice(%u) --> OK', [i]));

	// получим идентификатор устройства
	ModuleHandle := pModule.GetModuleHandle();

	// прочитаем название модуля в текущем виртуальном слоте
	ModuleName := '0123456';
	if not pModule.GetModuleName(pCHAR(ModuleName)) then AbortProgram('Не могу прочитать название модуля!')
	else WriteLn(' GetModuleName() --> OK');
	// проверим, что это модуль E14-440
	if Boolean(AnsiCompareStr(ModuleName, 'E440')) then AbortProgram('Обнаруженный модуль не является E14-440!')
	else WriteLn(' The module is ''E14-440''');

	// попробуем получить скорость работы шины USB
	if not pModule.GetUsbSpeed(@UsbSpeed) then AbortProgram(' Не могу определить скорость работы шины USB')
	else WriteLn(' GetUsbSpeed() --> OK\n');
	// теперь отобразим скорость работы шины USB
	if UsbSpeed = USB11_LUSBAPI then Str := 'Full-Speed Mode (12 Mbit/s)' else Str := 'High-Speed Mode (480 Mbit/s)';
	WriteLn(Format('   USB is in %s', [Str]));

	// Код драйвера DSP возьмём из соответствующего ресурса DLL библиотеки Lusbapi.dll
	if not pModule.LOAD_MODULE(nil) then AbortProgram('Не могу загрузить модуль E14-440!')
	else WriteLn(' LOAD_MODULE() --> OK');

	// проверим загрузку модуля
 	if not pModule.TEST_MODULE() then AbortProgram('Ошибка в загрузке модуля E14-440!')
	else WriteLn(' TEST_MODULE() --> OK');

	// теперь получим номер версии загруженного драйвера DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('Не могу получить информацию о модуле!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// проверим есть ли на модуле ЦАП
	if ModuleDescription.Dac.Active = BOOL(DAC_INACCESSIBLED_E440) then AbortProgram(' DAC is absent on this module E14-440!');

	// получим текущие параметры работы вывода данных
	if not pModule.GET_DAC_PARS(@dp) then AbortProgram('Не могу получить текущие параметры вывода данных!')
	else WriteLn(' GET_DAC_PARS --> OK');
	// установим желаемые параметры вывода данных на модуль E14-440
	dp.DacRate := DacRate;								// частота вывода данных в кГц
	dp.DacFifoBaseAddress := $3000;					// базовый адрес FIFO буфера ЦАП в DSP модуля
	dp.DacFifoLength := MAX_DAC_FIFO_SIZE_E440;	// длина FIFO буфера ЦАП в DSP модуля
	// передадим в модуль требуемые параметры по выводу данных
	if not pModule.SET_DAC_PARS(@dp) then AbortProgram('Не могу установить параметры вывода данных!')
	else WriteLn(' SET_DAC_PARS --> OK');

	// попробуем выделить нужное кол-во памяти под буфера данных
	for i := 0 to 1 do SetLength(Buffer[i], DataStep);

	// запустим поток вывода данных
	WriteLn('');
	hWriteThread := CreateThread(nil, $2000, @WriteThread, nil, 0, WriteTid);
	if hWriteThread = THANDLE(nil) then AbortProgram('Не могу запустить поток вывода данных!')
	else WriteLn(' Create WriteThread ... OK');

	// отобразим на экране монитора параметры работы модуля по выводу данных
	WriteLn('');
	Write  (' Module E14-440 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s (%s)', [StrPas(@ModuleDescription.Mcu.Version.Version), StrPas(@ModuleDescription.Mcu.Version.Date)]));
	WriteLn(Format('     LBIOS   Version    is %s (%s)', [StrPas(@ModuleDescription.Dsp.Version.Version), StrPas(@ModuleDescription.Dsp.Version.Date)]));
	WriteLn('   Dac parameters:');
	WriteLn(Format('     DacRate = %5.3f kHz', [dp.DacRate]));
	WriteLn('   Signal parameters:');
	WriteLn(Format('     SignalFrequency  = %2.2f kHz', [SignalFrequency]));
	WriteLn(Format('     SignalAmplitude  = %1.3f V', [SignalAmplitude*5.0/2000.0]));

{ !!!  Бесконечный цикл программы по выводу данных !!!													}
	// строчка номера используемого канала ЦАП
	if DacNumber = 0 then Str := 'first' else Str := 'second';
	WriteLn(Format(#10#13'   Now SINUS signal is on the %s DAC channel', [Str]));
	WriteLn(' (you can press ESC key to terminate the program)');
	repeat
		if Counter <> OldCounter then
			begin
				Write(Format(' Counter %8u'#13, [Counter]));
				OldCounter := Counter;
			end
		else Sleep(20);
	until IsWriteThreadComplete;

	// ждём окончания работы потока вывода данных
	WaitForSingleObject(hWriteThread, INFINITE);

	// две пустые строчки
	Write(#10#13#10#13);

	// посмотрим были ли ошибки при сборе данных
	AbortProgram(' ', false);
	if WriteThreadErrorNumber <> 0 then ShowThreadErrorMessage()
	else Write  (' The program was completed successfully!!!');

	exit;
end.

