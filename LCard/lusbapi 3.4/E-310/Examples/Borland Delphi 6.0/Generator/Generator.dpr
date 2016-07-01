//******************************************************************************
// Модуль E-310.
// Консольная программа с примером организации работы с генератором
//******************************************************************************
program Generator;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

var
	// интерфейс модуля E-310
	pModule : ILE310;
	// версия библиотеки Rtusbapi.dll
	DllVersion : DWORD;
	// идентификатор устройства
	ModuleHandle : THandle;
	// название модуля
	ModuleName: AnsiString;
	// скорость работы шины USB
	UsbSpeed : BYTE;
	// структура с полной информацией о модуле
	ModuleDescription : MODULE_DESCRIPTION_E310;
	// структура параметрами работы генератора
	GeneratorPars : GENERATOR_PARS_E310;

	// *** вспомогательные переменные ***
	i : WORD;
	// счётчик выведенных отсчётов
	Counter : DWORD;
	// Дескриптор стандартного устройства ввода компьютера - клавиатура
	InputConsoleHandle : THandle;
	// Дескриптор стандартного устройства вывода компьютера - дисплей
	OutputConsoleHandle : THandle;
	// Для хранения максимальных размеров окна
	MaxX, MaxY : DWORD;
	// вспомогательная строчка
	Str : AnsiString;

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
procedure AbortProgram(ErrorString: AnsiString; AbortionFlag : bool = true);
begin
	// освободим интерфейс модуля
	if pModule <> nil then
		begin
			// остановим генератор
			if not pModule.STOP_GENERATOR() then  WriteLn(' STOP_GENERATOR() --> Bad')
			else WriteLn(' STOP_GENERATOR() --> OK');
			// освободим интерфейс модуля
			if not pModule.ReleaseLInstance() then  WriteLn(' ReleaseLInstance() --> Bad')
			else WriteLn(' ReleaseLInstance() --> OK');
			// обнулим указатель на интерфейс модуля
			pModule := nil;
		end;

	// если нужно - выводим сообщение с ошибкой
	if ErrorString <> ' ' then MessageBox(HWND(nil), pAnsiChar(ErrorString), 'ОШИБКА!!!', MB_OK + MB_ICONINFORMATION);
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

	// попробуем получить указатель на интерфейс для модуля E-310
	pModule := CreateLInstance(pAnsiChar('e310'));
	if pModule = nil then AbortProgram('Не могу найти интерфейс модуля E-310!')
	else WriteLn(' Module Interface --> OK');

	// попробуем обнаружить модуль E-310 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for i := 0 to (MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI-1) do if pModule.OpenLDevice(i) then break;
	// что-нибудь обнаружили?
	if i = MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI then AbortProgram('Не удалось обнаружить модуль E-310 в первых 127 виртуальных слотах!')
	else WriteLn(Format(' OpenLDevice(%u) --> OK', [i]));

	// получим идентификатор устройства
	ModuleHandle := pModule.GetModuleHandle();
	if ModuleHandle = INVALID_HANDLE_VALUE then AbortProgram('Не удалось получить дескриптор устройства!')
	else WriteLn(' GetModuleHandle() --> OK');

	// прочитаем название модуля в текущем виртуальном слоте
	ModuleName := '0123456789ABCDEF';
	if not pModule.GetModuleName(pAnsiChar(ModuleName)) then AbortProgram('Не могу прочитать название модуля!')
	else WriteLn(' GetModuleName() --> OK');
	// проверим, что это модуль E-310
	if Boolean(AnsiCompareStr(ModuleName, 'E-310')) then AbortProgram('Обнаруженный модуль не является E-310!')
	else WriteLn(' The module is ''E-310''');

	// попробуем получить скорость работы шины USB
	if not pModule.GetUsbSpeed(@UsbSpeed) then AbortProgram(' Не могу определить скорость работы шины USB')
	else WriteLn(' GetUsbSpeed() --> OK\n');
	// теперь отобразим скорость работы шины USB
	if UsbSpeed = USB11_LUSBAPI then Str := 'Full-Speed Mode (12 Mbit/s)' else Str := 'High-Speed Mode (480 Mbit/s)';
	WriteLn(Format('   USB is in %s', [Str]));

	// теперь получим номер версии загруженного драйвера DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('Не могу получить информацию о модуле!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// получим текущие параметры работы генератора
	if not pModule.GET_GENERATOR_PARS(@GeneratorPars) then AbortProgram(' Не могу получить текущие параметры работы генератора!')
	else WriteLn(' GET_GENERATOR_PARS() --> OK');

	// установим желаемые параметры работы генератора
	GeneratorPars.StartFrequency 			:= 10.0;
	GeneratorPars.FrequencyIncrements	:= 1.0;
	GeneratorPars.NumberOfIncrements		:= 250;
	GeneratorPars.IncrementIntervalPars.BaseIntervalType		:= CLOCK_PERIOD_INCREMENT_INTERVAL_E310;
	GeneratorPars.IncrementIntervalPars.MultiplierIndex		:= INCREMENT_INTERVAL_MULTIPLIERS_500_E310;
	GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber	:= 250;
	GeneratorPars.MasterClock				:= 50000.0;
	GeneratorPars.MasterClockSource		:= INTERNAL_MASTER_CLOCK_E310;
	GeneratorPars.CyclicAutoScanType		:= CYCLIC_TRIANGLE_AUTO_SCAN_E310;//NO_CYCLIC_AUTOSCAN_E310;
	GeneratorPars.IncrementType			:= AUTO_INCREMENT_E310;
	GeneratorPars.CtrlLineType				:= INTERNAL_CTRL_LINE_E310;
	GeneratorPars.InterrupLineType		:= INTERNAL_INTERRUPT_LINE_E310;
	GeneratorPars.SquareWaveOutputEna	:= false;
	GeneratorPars.SynchroOutEna			:= true;//false;
	GeneratorPars.SynchroOutType			:= SYNCOUT_AT_END_OF_SCAN_E310;
	GeneratorPars.AnalogOutputsPars.SignalType			:= SINUSOIDAL_ANALOG_OUTPUT_E310;
	GeneratorPars.AnalogOutputsPars.GainIndex				:= ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310;
	GeneratorPars.AnalogOutputsPars.Output10OhmOffset	:= 0.0;
	GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource := INTERNAL_OUTPUT_10_OHM_OFFSET_E310;

	// передадим требуемые параметры работы генератора в модуль
	if not pModule.SET_GENERATOR_PARS(@GeneratorPars) then AbortProgram(' Не могу установить параметры работы генератора!')
	else WriteLn(' SET_GENERATOR_PARS() --> OK');

	// запускаем генератор
	if not pModule.START_GENERATOR() then AbortProgram(' Не могу запустить генератор!')
	else WriteLn(' START_GENERATOR() --> OK');

	// отобразим параметры работы модуля по вводу данных на экране монитора
	WriteLn('');
	Write  (' Module E-310 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s (%s)', [StrPas(@ModuleDescription.Mcu.Version.FwVersion.Version), StrPas(@ModuleDescription.Mcu.Version.FwVersion.Date)]));
	WriteLn('   Generator parameters: ');
	// вставим пустую строчку
	Write(#10#13);

	// !!!  Основной цикл программы с ожиданием нажатия клавиши ESC !!!
	WriteLn(#10#13' You can press ESC key to terminate the program...'#10#13);
	Counter := 0;
	Write(' Time: ', Counter);	Write(' s'#13);
	while true do
		begin
			// выходим из цикла по нажатию клавиши ESC
			if IsEscKeyPressed() then break;

			// задержечка
			Sleep(1000);
			// отобразим время
			Inc(Counter);
			Write(' Time: ', Counter);	Write(' s'#13);
		end;

	// две пустые строчки
	Write(#10#13#10#13);

	// посмотрим были ли ошибки при сборе данных
	AbortProgram(' ', false);
	// завершение приложения
	exit;
end.

