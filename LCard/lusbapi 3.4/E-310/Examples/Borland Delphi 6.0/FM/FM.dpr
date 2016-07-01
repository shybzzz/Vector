//******************************************************************************
// ������ E-310.
// ���������� ��������� � �������� ����������� ������ � ������������
//******************************************************************************
program AdcData;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

var
	// ��������� ������ E-310
	pModule : ILE310;
	// ������ ���������� Rtusbapi.dll
	DllVersion : DWORD;
	// ������������� ����������
	ModuleHandle : THandle;
	// �������� ������
	ModuleName: AnsiString;
	// �������� ������ ���� USB
	UsbSpeed : BYTE;
	// ��������� � ������ ����������� � ������
	ModuleDescription : MODULE_DESCRIPTION_E310;
	// ��������� ���������� ������ ������������
	FmPars : FM_PARS_E310;
	// ��������� ������� ������������
	FmSample : FM_SAMPLE_E310;

	// *** ��������������� ���������� ***
	i : WORD;
	// ���������� ������������ ���������� ����� ���������� - ����������
	InputConsoleHandle : THandle;
	// ���������� ������������ ���������� ������ ���������� - �������
	OutputConsoleHandle : THandle;
	// ��� �������� ������������ �������� ����
	MaxX, MaxY : DWORD;
	// ��������������� �������
	Str : AnsiString;

//------------------------------------------------------------------------------
// ������������� ���������� ���������� �����/������
//------------------------------------------------------------------------------
procedure InitConsoleHandles;
var
	Coord : TCoord;  // ��� ��������/��������� ������� ������
begin
	// �������� ���������� ������������ ���������� ����� ���������� - ����������
	InputConsoleHandle := GetStdHandle(STD_INPUT_HANDLE);
	// �������� ���������� ������������ ���������� ������ ���������� - �����
	OutputConsoleHandle := GetStdHandle(STD_OUTPUT_HANDLE);
	// �������� ������������ ������� ����
	Coord := GetLargestConsoleWindowSize(OutputConsoleHandle);
	MaxX := Coord.X;
	MaxY := Coord.Y;
end;

//------------------------------------------------------------------------------
// ������� ����������� ������� ������� ESCAPE
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
				// ������ �������
			 end;
		 // ������ ���� �������
		end;
	 end;
end;

//------------------------------------------------------------------------------
// ��������� ������� � ���������� X, Y
//------------------------------------------------------------------------------
procedure GotoXY(X, Y : Word);
var
	Coord : TCoord;  // ��� ��������/��������� ������� ������
begin
	Coord.X := X; Coord.Y := Y;
	SetConsoleCursorPosition(OutputConsoleHandle, Coord);
end;

//------------------------------------------------------------------------------
// ��������� ������� � ���������� X, Y
//------------------------------------------------------------------------------
procedure GetXY(var X : Word; var Y : Word);
var
  sbi: TConsoleScreenBufferInfo;
begin
	GetConsoleScreenBufferInfo(OutputConsoleHandle, sbi);
	X := sbi.dwCursorPosition.X;
	Y := sbi.dwCursorPosition.Y;
end;

//------------------------------------------------------------------------------
// ������� ������ - ���������� ��� ���������
//------------------------------------------------------------------------------
procedure ClearScreen;
var
	Coord : TCoord;	// ��� ��������/��������� ������� ������
	NOAW  : DWORD;		// ��� �������� ����������� ��������� �������
begin
	Coord.X := 0; Coord.Y := 0;
	FillConsoleOutputCharacter(OutputConsoleHandle, ' ', MaxX*MaxY, Coord, NOAW);
	GotoXY(0, 0);
end;

//------------------------------------------------------------------------------
// ��������� ���������� ���������
//------------------------------------------------------------------------------
procedure AbortProgram(ErrorString: AnsiString; AbortionFlag : bool = true);
begin
	// ��������� ��������� ������
	if pModule <> nil then
		begin
			// ��������� ����������
			if not pModule.STOP_FM() then  WriteLn(' STOP_FM() --> Bad')
			else WriteLn(' STOP_FM() --> OK');
			// ��������� ��������� ������
			if not pModule.ReleaseLInstance() then  WriteLn(' ReleaseLInstance() --> Bad')
			else WriteLn(' ReleaseLInstance() --> OK');
			// ������� ��������� �� ��������� ������
			pModule := nil;
		end;

	// ���� ����� - ������� ��������� � �������
	if ErrorString <> ' ' then MessageBox(HWND(nil), pAnsiChar(ErrorString), '������!!!', MB_OK + MB_ICONINFORMATION);
	// ���� ����� - �������� ��������� ���������
	if AbortionFlag = true then halt;
end;

//******************************************************************************
//					  	� � � � � � � �    � � � � � � � � �
//******************************************************************************
begin
	// ������������� ������������ ����� � ������
	InitConsoleHandles();
	// ������� ����� �������
	ClearScreen();

	// �������� ������ ������������ DLL ����������
	DllVersion := GetDllVersion;
	if DllVersion <> CURRENT_VERSION_LUSBAPI then
		begin
			Str := '�������� ������ DLL ���������� Lusbapi.dll! ' + #10#13 +
						'           �������: ' + IntToStr(DllVersion shr 16) +  '.' + IntToStr(DllVersion and $FFFF) + '.' +
						' ���������: ' + IntToStr(CURRENT_VERSION_LUSBAPI shr 16) + '.' + IntToStr(CURRENT_VERSION_LUSBAPI and $FFFF) + '.';
			AbortProgram(Str);
		end
	else WriteLn(' DLL Version --> OK');

	// ��������� �������� ��������� �� ��������� ��� ������ E-310
	pModule := CreateLInstance(pAnsiChar('e310'));
	if pModule = nil then AbortProgram('�� ���� ����� ��������� ������ E-310!')
	else WriteLn(' Module Interface --> OK');

	// ��������� ���������� ������ E-310 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for i := 0 to (MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI-1) do if pModule.OpenLDevice(i) then break;
	// ���-������ ����������?
	if i = MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI then AbortProgram('�� ������� ���������� ������ E-310 � ������ 127 ����������� ������!')
	else WriteLn(Format(' OpenLDevice(%u) --> OK', [i]));

	// ������� ������������� ����������
	ModuleHandle := pModule.GetModuleHandle();
	if ModuleHandle = INVALID_HANDLE_VALUE then AbortProgram('�� ������� �������� ���������� ����������!')
	else WriteLn(' GetModuleHandle() --> OK');

	// ��������� �������� ������ � ������� ����������� �����
	ModuleName := '0123456789ABCDEF';
	if not pModule.GetModuleName(pAnsiChar(ModuleName)) then AbortProgram('�� ���� ��������� �������� ������!')
	else WriteLn(' GetModuleName() --> OK');
	// ��������, ��� ��� ������ E-310
	if Boolean(AnsiCompareStr(ModuleName, 'E-310')) then AbortProgram('������������ ������ �� �������� E-310!')
	else WriteLn(' The module is ''E-310''');

	// ��������� �������� �������� ������ ���� USB
	if not pModule.GetUsbSpeed(@UsbSpeed) then AbortProgram(' �� ���� ���������� �������� ������ ���� USB')
	else WriteLn(' GetUsbSpeed() --> OK\n');
	// ������ ��������� �������� ������ ���� USB
	if UsbSpeed = USB11_LUSBAPI then Str := 'Full-Speed Mode (12 Mbit/s)' else Str := 'High-Speed Mode (480 Mbit/s)';
	WriteLn(Format('   USB is in %s', [Str]));

	// ������ ������� ����� ������ ������������ �������� DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('�� ���� �������� ���������� � ������!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// -=== ��������� ����������� ===-
	// �������� ������� ��������� �����������
	if not pModule.GET_FM_PARS(@FmPars) then AbortProgram(' �� ���� �������� ������� ��������� ������ �����������!')
	else WriteLn(' GET_FM_PARS() --> OK');

	FmPars.Mode				:= PERIOD_MODE_FM_E310;			// �����: ��������� ������� �������� �������
	FmPars.InputDivider	:= INPUT_DIVIDER_1_FM_E310;	// �������� ������� 1:1 �������� �������
//	FmPars.InputDivider	:= INPUT_DIVIDER_8_FM_E310;	// �������� ������� 1:8 �������� �������
	FmPars.BaseClockRateDivIndex := BASE_CLOCK_DIV_02_INDEX_FM_E310;	// �������� ������� �������
//	FmPars.BaseClockRateDivIndex = BASE_CLOCK_DIV_08_INDEX_FM_E310;	// �������� ������� �������
	FmPars.Offset			:= 0.0;//2.5;						// �������� ������� ����������� � �

	// ������������� ��������� �����������
	if not pModule.SET_FM_PARS(@FmPars) then AbortProgram(' �� ���� ���������� ��������� ������ �����������!')
	else WriteLn(' SET_FM_PARS() --> OK');
	// -=============================-

	// ��������� ����������
	if not pModule.START_FM() then AbortProgram(' �� ���� ��������� ����������!')
	else WriteLn(' START_FM() --> OK');


	// ��������� ��������� ������ ������ �� ����� ������ �� ������ ��������
	WriteLn('');
	Write  (' Module E-310 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s (%s)', [StrPas(@ModuleDescription.Mcu.Version.FwVersion.Version), StrPas(@ModuleDescription.Mcu.Version.FwVersion.Date)]));
	WriteLn('   FM parameters:');
	if FmPars.InputDivider = INPUT_DIVIDER_1_FM_E310 then WriteLn('     Input Divider is 1:1')
	else WriteLn('     Input Divider is 1:8');
	WriteLn(Format('     Clock Rate = %10.4f kHz', [FmPars.ClockRate/1000.0]));

	//
	WriteLn(#10#13' You can press ESC key to terminate the program...'#10#13);

	// !!!  �������� ���� ��������� � ��������� ������� ������� ESC !!!
	while true do
		begin
			// ��������� �������� ������ �����������
			if not pModule.FM_SAMPLE(@FmSample) then AbortProgram(' �� ���� �������� ������ �����������!');
			// ������� �� ����� �� ������� ������� ESC
			if IsEscKeyPressed() then break;

			// ��������� �� ������ ���� ���
			if FmSample.IsActual = TRUE then Write(Format(' FM Sample: %10.3f kHz '#13, [FmSample.Frequency]))
			else Write(' FM Sample:  --------------'#13);

			// ����������
			Sleep(200);
		end;

	// ��� ������ �������
	Write(#10#13#10#13);

	// ��������� ���� �� ������ ��� ����� ������
	AbortProgram(' ', false);

	exit;
end.

