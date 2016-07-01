//******************************************************************************
// Модуль E14-140.
// Консольная программа с организацией потокового ввода данных с АЦП
// с одновременной записью получаемых данных на диск в реальном масштабе времени.
// Ввод осуществляется с первых четырёх каналов АЦП на частоте 100 кГц.
//******************************************************************************
program ReadData;

{$APPTYPE CONSOLE}

uses
  SysUtils,
  Windows,
  Lusbapi;

const
	// кол-во отсчетов в запросе (кратное 32) для ф. ReadData
	DataStep : DWORD = 64*1024;
	// столько блоков по DataStep отсчётов нужно собрать в файл
	NBlockToRead : WORD = 4*20;
	// кол-во активных каналов
	ChannelsQuantity : WORD = $04;
	// частота ввода данных
	AdcRate : double  = 100.0;

type
	TShortrArray = array [0..1] of array of SHORT;

var
	// идентификатор потока ввода
	hReadThread : THANDLE;
	ReadTid : DWORD;

	// Идентификатор файла данных
		FileHandle: Integer;

	// интерфейс модуля E14-140
	pModule : ILE140;
	// версия библиотеки Lusbapi.dll
	DllVersion : DWORD;
	// идентификатор устройства
	ModuleHandle : THandle;
	// название модуля
	ModuleName: String;
	// скорость работы шины USB
	UsbSpeed : BYTE;
	// структура с полной информацией о модуле
	ModuleDescription : MODULE_DESCRIPTION_E140;
	// структура параметров работы АЦП
	ap : ADC_PARS_E140;

	// экранный счетчик-индикатор
	Counter, OldCounter : WORD;
	// указатель на буфер для данных
	Buffer : TShortrArray;

	// номер ошибки при выполнения потока сбора данных
	ReadThreadErrorNumber : WORD;
	// флажок завершения потоков ввода данных
	IsReadThreadComplete : boolean;

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
	Buffer: INPUT_RECORD;
	Count: DWORD;

begin
	Result := false;
	GetNumberOfConsoleInputEvents(InputConsoleHandle, Count);
	if Count > 0 then
		begin
			ReadConsoleInput(InputConsoleHandle, Buffer, 1, Count);
			case Buffer.EventType of
				KEY_EVENT:
					case Buffer.Event.KeyEvent.wVirtualKeyCode of
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
// ожидание завершения выполнения очередного запроса на сбор данных
//------------------------------------------------------------------------------
function WaitingForRequestCompleted(var ReadOv : OVERLAPPED) : boolean;
var 	BytesTransferred : DWORD;
begin
	Result := true;
	while true do
	   begin
			if GetOverlappedResult(ModuleHandle, ReadOv, BytesTransferred, FALSE) then break
			else if (GetLastError() <>  ERROR_IO_INCOMPLETE) then
				begin
					// ошибка ожидания ввода очередной порции данных
					ReadThreadErrorNumber := 3; Result := false; break;
				end
			else if IsEscKeyPressed() then
				begin
					// программа прервана (нажата клавиша ESC)
					ReadThreadErrorNumber := 4; Result := false; break;
				end
			else Sleep(20);
		end;
end;

//------------------------------------------------------------------------------
//      фукция запускаемая в качестве отдельного потока
//             для сбора данных c модуля E14-140
//------------------------------------------------------------------------------
function ReadThread(var param : pointer): DWORD;
var
	i : WORD ;
	RequestNumber : WORD;
	// массив OVERLAPPED структур из двух элементов
	ReadOv : array[0..1] of OVERLAPPED;
	// массив структур с параметрами запроса на ввод/вывод данных
	IoReq : array[0..1] of IO_REQUEST_LUSBAPI;

begin
	Result := 0;
	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if not pModule.STOP_ADC() then begin ReadThreadErrorNumber := 1; IsReadThreadComplete := true; exit; end;

	// формируем необходимые для сбора данных структуры
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
			IoReq[i].TimeOut := Round(Int(DataStep/ap.AdcRate)) + 1000;
		end;

	// заранее закажем первый асинхронный сбор данных в Buffer
	RequestNumber := 0;
	if not pModule.ReadData(@IoReq[RequestNumber]) then
		begin
			CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent); ReadThreadErrorNumber := 2; IsReadThreadComplete := true; exit;
		end;

	// а теперь можно запускать сбор данных
	if pModule.START_ADC() then
   	begin
			// цикл сбора данных
			for i := 1 to (NBlockToRead-1) do
				begin
					RequestNumber := RequestNumber xor $1;
					// сделаем запрос на очередную порции вводимых данных
					if not pModule.ReadData(@IoReq[RequestNumber]) then
						begin
							ReadThreadErrorNumber := 2; break;
						end;

					// ожидание выполнение очередного запроса на сбор данных
//					if not WaitingForRequestCompleted(IoReq[RequestNumber xor $1].Overlapped^) then break;
					if not WaitForSingleObject(IoReq[RequestNumber xor $1].Overlapped.hEvent, IoReq[RequestNumber xor $1].TimeOut) = WAIT_TIMEOUT then begin ReadThreadErrorNumber := $03; break; end;
					// пишем файл очередную порцию данных
					if FileWrite(FileHandle, Buffer[RequestNumber xor $1][0], DataStep*sizeof(SHORT)) = -1 then begin ReadThreadErrorNumber := $5; break; end;

					// были ли ошибки или пользователь прервал ввод данных?
					if ReadThreadErrorNumber <> 0 then break
					// была ли программа прервана (нажата клавиша ESC)?
					else if IsEscKeyPressed() then begin ReadThreadErrorNumber := 4; break; end
					// небольшая задержечка               
					else Sleep(20);
					// увеличиваем счётчик полученных блоков данных
					Inc(Counter);
				end
		end
	else ReadThreadErrorNumber := 6;

	// последняя порция данных
	if ReadThreadErrorNumber = 0 then
		begin
			// ждём окончания операции сбора последней порции данных
			if WaitingForRequestCompleted(IoReq[RequestNumber].Overlapped^) then
				begin
					// увеличим счётчик полученных блоков данных
		         Inc(Counter);
					// пишем файл последную порцию данных
					if FileWrite(FileHandle, Buffer[RequestNumber][0], DataStep*sizeof(SHORT)) = -1 then ReadThreadErrorNumber := $5;
				end;
		end;

	// остановим сбор данных
	if not pModule.STOP_ADC() then ReadThreadErrorNumber := 1;
	// если надо, то прервём незавершённый асинхронный запрос
	if not CancelIo(ModuleHandle) then ReadThreadErrorNumber := 7;
	// освободим идентификаторы событий
	CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent);
	// задержечка   
	Sleep(100);   
	// установим флажок окончания потока сбора данных
	IsReadThreadComplete := true;

end;

//------------------------------------------------------------------------------
// аварийное завершение программы
//------------------------------------------------------------------------------
procedure AbortProgram(ErrorString: string; AbortionFlag : bool = true);
var
	i : WORD ;
begin
	// освободим интерфейс модуля
	if pModule <> nil then
		begin
			// освободим интерфейс модуля
			if not pModule.ReleaseLInstance() then  WriteLn(' ReleaseLInstance() --> Bad')
			else WriteLn(' ReleaseLInstance() --> OK');
			// обнулим указатель на интерфейс модуля
			pModule := nil;
		end;

	// освободим идентификатор потока сбора данных
	if hReadThread = THANDLE(nil) then CloseHandle(hReadThread);
	// закроем файл данных
	if FileHandle <> -1 then FileClose(FileHandle);
	// освободим память из-под буферов данных
	for i := 0 to 1 do Buffer[i] := nil;

	// если нужно - выводим сообщение с ошибкой
	if ErrorString <> ' ' then MessageBox(HWND(nil), pCHAR(ErrorString), 'ОШИБКА!!!', MB_OK + MB_ICONINFORMATION);
	// если нужно - аварийно завершаем программу
	if AbortionFlag = true then halt;
end;

//------------------------------------------------------------------------------
// отображение ошибок возникших во время работы потока сбора данных
//------------------------------------------------------------------------------
procedure ShowThreadErrorMessage;
begin
	case ReadThreadErrorNumber of
		$0 : ;
		$1 : WriteLn(' ADC Thread: STOP_ADC() --> Bad! :(((');
		$2 : WriteLn(' ADC Thread: ReadData() --> Bad :(((');
		$3 : WriteLn(' ADC Thread: Waiting data Error! :(((');
		// если программа была злобно прервана, предъявим ноту протеста
		$4 : WriteLn(' ADC Thread: The program was terminated! :(((');
		$5 : WriteLn(' ADC Thread: Writing data file error! :(((');
		$6 : WriteLn(' ADC Thread: START_ADC() --> Bad :(((');
		$7 : WriteLn(' ADC Thread: Can''t cancel ending input and output (I/O) operations! :(((');
		else WriteLn(' ADC Thread: Unknown error! :(((');
	end;
end;

//******************************************************************************
//					  	О С Н О В Н А Я    П Р О Г Р А М М А
//******************************************************************************
begin
	// сбросим флаги ошибки потока ввода
	ReadThreadErrorNumber := 0;
	// сбросим флажок завершённости потока сбора данных
	IsReadThreadComplete := false;
	// пока откытого файла нет :(
	FileHandle := -1;
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

	// попробуем получить указатель на интерфейс для модуля E14-140
	pModule := CreateLInstance(pCHAR('e140'));
	if pModule = nil then AbortProgram('Не могу найти интерфейс модуля E14-140!')
	else WriteLn(' Module Interface --> OK');

	// попробуем обнаружить модуль E14-140 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for i := 0 to (MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI-1) do if pModule.OpenLDevice(i) then break;
	// что-нибудь обнаружили?
	if i = MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI then AbortProgram('Не удалось обнаружить модуль E14-140 в первых 127 виртуальных слотах!')
	else WriteLn(Format(' OpenLDevice(%u) --> OK', [i]));

	// получим идентификатор устройства
	ModuleHandle := pModule.GetModuleHandle();

	// прочитаем название модуля в текущем виртуальном слоте
	ModuleName := '0123456';
	if not pModule.GetModuleName(pCHAR(ModuleName)) then AbortProgram('Не могу прочитать название модуля!')
	else WriteLn(' GetModuleName() --> OK');
	// проверим, что это модуль E14-140
	if Boolean(AnsiCompareStr(ModuleName, 'E140')) then AbortProgram('Обнаруженный модуль не является E14-140!')
	else WriteLn(' The module is ''E14-140''');

	// попробуем получить скорость работы шины USB
	if not pModule.GetUsbSpeed(@UsbSpeed) then AbortProgram(' Не могу определить скорость работы шины USB')
	else WriteLn(' GetUsbSpeed() --> OK\n');
	// теперь отобразим скорость работы шины USB
	if UsbSpeed = USB11_LUSBAPI then Str := 'Full-Speed Mode (12 Mbit/s)' else Str := 'High-Speed Mode (480 Mbit/s)';
	WriteLn(Format('   USB is in %s', [Str]));

	// теперь получим номер версии загруженного драйвера DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('Не могу получить информацию о модуле!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// проверим прошивку MCU модуля
	DecimalSeparator := '.';
   if (AnsiChar(ModuleDescription.Module.Revision) = REVISIONS_E140[REVISION_B_E140]) AND
      (StrToFloat(pAnsiChar(@ModuleDescription.Mcu.Version.Version[0])) < 3.05)
      	then AbortProgram('Для модуля E14-140(Rev.''B'') версия прошивки ARM должна быть 3.05 или выше!');

	// получим текущие параметры работы ввода данных
	if not pModule.GET_ADC_PARS(@ap) then AbortProgram('Не могу получить текущие параметры ввода данных!')
	else WriteLn(' GET_ADC_PARS --> OK');
	// установим желаемые параметры ввода данных с модуля E14-140
	ap.ClkSource := INT_ADC_CLOCK_E140;							// внутренний запуск АЦП
	ap.EnableClkOutput := ADC_CLOCK_TRANS_DISABLED_E140; 	// без трансляции тактовых импульсо АЦП
	ap.InputMode := NO_SYNC_E140;									// без синхронизации ввода данных
	ap.ChannelsQuantity := ChannelsQuantity; 					// кол-во активных каналов
	for i:=0 to (ap.ChannelsQuantity-1) do ap.ControlTable[i] := i or (ADC_INPUT_RANGE_2500mV_E140 shl $6);
	ap.AdcRate := AdcRate;						// частота ввода данных в кГц
	ap.InterKadrDelay := 0.0;					// межкадровая задержка - пока всегда устанавливать в 0.0
	// передадим в модуль требуемые параметры по вводу данных
	if not pModule.SET_ADC_PARS(@ap) then AbortProgram('Не могу установить параметры ввода данных!')
	else WriteLn(' SET_ADC_PARS --> OK');

	// попробуем выделить нужное кол-во памяти под буфера данных
	for i := 0 to 1 do begin SetLength(Buffer[i], DataStep); ZeroMemory(Buffer[i], DataStep*SizeOf(SHORT)); end;

	// попробуем открыть файл для записи собранных данных
	WriteLn('');
	FileHandle := FileCreate('Test.dat');
	if FileHandle = -1 then AbortProgram('Не могу создать файл Test.dat для записи собираемых данных!')
	else WriteLn(' Create file Test.dat ... OK');

	// запустим поток сбора данных
	hReadThread := CreateThread(nil, $2000, @ReadThread, nil, 0, ReadTid);
	if hReadThread = THANDLE(nil) then AbortProgram('Не могу запустить поток сбора данных!')
	else WriteLn(' Create ReadThread ... OK');

	// отобразим параметры работы модуля по вводу данных на экране монитора
	WriteLn('');
	Write  (' Module E14-140 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s (%s)', [StrPas(@ModuleDescription.Mcu.Version.Version), StrPas(@ModuleDescription.Mcu.Version.Date)]));
	WriteLn('   ADC parameters:');
	WriteLn('     ChannelsQuantity = ', ap.ChannelsQuantity);
	WriteLn(Format('     AdcRate = %5.3f kHz', [ap.AdcRate]));
	WriteLn(Format('     InterKadrDelay = %2.4f ms', [ap.InterKadrDelay]));
	WriteLn(Format('     KadrRate =  %5.3f kHz', [ap.KadrRate]));

{ !!!  Основной цикл программы ожидания конца сбора данных !!!													}
	WriteLn(#10#13' You can press ESC key to terminate the program...'#10#13);
	repeat
		if Counter <> OldCounter then
			begin
				Write(Format(' Counter %3u from %3u'#13, [Counter, NBlockToRead]));
				OldCounter := Counter;
			end
		else Sleep(20);
	until IsReadThreadComplete;

	// ждём окончания работы потока ввода данных
	WaitForSingleObject(hReadThread, INFINITE);

	// две пустые строчки
	Write(#10#13#10#13);

	// посмотрим были ли ошибки при сборе данных
	AbortProgram(' ', false);
	if ReadThreadErrorNumber <> 0 then ShowThreadErrorMessage()
	else Write  (' The program was completed successfully!!!');

	exit;
end.

