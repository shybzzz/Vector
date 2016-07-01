//******************************************************************************
// Модуль E14-140.
// Консольная программа с примером организации однократного ввода отсчёта с АЦП
//	с помощью интерфейсной функции ADC_SAMPLE().
// Ввод осуществляется с первый двух каналов АЦП.
//******************************************************************************
program DacSample;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

const
	InputRangeIndex : WORD  = ADC_INPUT_RANGE_2500mV_E140;
//	InputRangeIndex : WORD  = ADC_INPUT_RANGE_10000mV_E140;

var
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

	// отсчёты для АЦП
	AdcSample0, AdcSample1 : SHORT;

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
// аварийное завершение программы
//------------------------------------------------------------------------------
procedure AbortProgram(ErrorString: string; AbortionFlag : bool = true);
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

	// если нужно - выводим сообщение с ошибкой
	if ErrorString <> ' ' then MessageBox(HWND(nil), pCHAR(ErrorString), 'ОШИБКА!!!', MB_OK + MB_ICONINFORMATION);
	// если нужно - аварийно завершаем программу
	if AbortionFlag = true then halt;
end;

//******************************************************************************
//					  	О С Н О В Н А Я    П Р О Г Р А М М А
//******************************************************************************
begin
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
	if ModuleHandle = INVALID_HANDLE_VALUE then AbortProgram('Не удалось получить дескриптор устройства!')
	else WriteLn(' GetModuleHandle() --> OK');

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

	// отобразим параметры работы модуля по вводу данных на экране монитора
	WriteLn('');
	Write  (' Module E14-140 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s %s', [StrPas(@ModuleDescription.Mcu.Version.Version), StrPas(@ModuleDescription.Mcu.Version.Date)]));
	WriteLn('   ADC parameters:');
	WriteLn(Format('     Input Range = %6.2f Volt', [ADC_INPUT_RANGES_E140[InputRangeIndex]]));
	// вставим пустую строчку
	Write(#10#13); WriteLn('   Press ESC key to terminate the program ...');
   Write(#10#13);
	WriteLn('   ADC Channel:                    1         2');

	// Цикл перманетной работы функции ADC_SAMPLE().
	while true do
	   begin
			// выведем очередной отсчёт на первый канал АЦП
			if not pModule.ADC_SAMPLE(@AdcSample0, 0 or (0 shl 4) or (InputRangeIndex shl 6)) then begin WriteLn(#10#13' ADC_SAMPLE(0, ) -> Bad!'); break; end
			// выведем очередной отсчёт на второй канал АЦП
			else if not pModule.ADC_SAMPLE(@AdcSample1, 1 or (1 shl 4) or (InputRangeIndex shl 6)) then begin WriteLn(#10#13' ADC_SAMPLE(0, ) -> Bad!'); break; end
			// выходим из цикла по нажатию клавиши ESC
			else if IsEscKeyPressed() then break;
			Write(Format('   AdcSample Data (ADC code): = %5d     %5d', [AdcSample0, AdcSample1])); Write(#13);
		end;

	// две пустые строчки
	Write(#10#13#10#13);

	// посмотрим были ли ошибки при сборе данных
	AbortProgram(' ', false);
	// завершение приложения
	exit;
end.

