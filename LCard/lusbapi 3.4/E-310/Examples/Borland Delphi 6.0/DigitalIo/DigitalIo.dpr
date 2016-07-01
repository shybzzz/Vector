//******************************************************************************
// ������ E-310.
// ���������� ��������� � �������� ����������� ������ � ��������� �������
//******************************************************************************
program DigitalIo;

{$APPTYPE CONSOLE}

{%File '..\AdcData\AdcData.dpr'}

uses
  SysUtils,
  Windows,
  Lusbapi;

var
	// ��������� ������ E-310
	pModule : ILE310;
	// ������ ���������� Lusbapi.dll
	DllVersion : DWORD;
	// ������������� ����������
	ModuleHandle : THandle;
	// �������� ������
	ModuleName: AnsiString;
	// �������� ������ ���� USB
	UsbSpeed : BYTE;
	// ��������� � ������ ����������� � ������
	ModuleDescription : MODULE_DESCRIPTION_E310;

	// ����� �������� �����
	TtlLinesPattern : WORD;
	// ��������� �������� �����
	TtlOut, TtlIn : WORD;

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
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram(' �� ���� �������� ���������� � ������!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// ��������� ����� �������� �����:
	// ������ 4 ����� - ��������
	// ������ 4 ����� - �������
	TtlLinesPattern := $000F;
	// ���������� �������� �����
	if not pModule.CONFIG_TTL_LINES(TtlLinesPattern) then AbortProgram(' �� ���� ������������� �������� �����!')
	else WriteLn(' CONFIG_TTL_LINES() --> OK');

	// ��������� �� ������ �������� ��������� ������ ������ �� ������ ������
	WriteLn('');
	Write  (' Module E-310 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s (%s)', [StrPas(@ModuleDescription.Mcu.Version.FwVersion.Version), StrPas(@ModuleDescription.Mcu.Version.FwVersion.Date)]));
	WriteLn('   Digital lines parameters:');
	WriteLn('     Outputs: 4');
	WriteLn('     Inputs : 4');

	//
	WriteLn(#10#13' You can press ESC key to terminate the program...'#10#13);
	//
	WriteLn('                              In          Out');

	// !!!  �������� ���� ��������� � ��������� ������� ������� ESC !!!
	while true do
		begin
			// ��������� ���������� �������� �����
			if not pModule.TTL_OUT(@TtlOut) then AbortProgram(' �� ���� �������� ������ � ���!');
			// ��������� �������� ��������� �������� �����
			if not pModule.TTL_IN(@TtlIn) then AbortProgram(' �� ���� �������� ������ � ���!');
			// ������� �� ����� �� ������� ������� ESC
			if IsEscKeyPressed() then break;

			// �������������� ��������� �������� �������� �����
			Inc(TtlOut); TtlOut := TtlOut and TtlLinesPattern;
			// ��������� �� ������ ��������� �������� �����
			Write(Format(' Digital Lines States:       0x%0.2X         0x%0.2X', [TtlIn, TtlOut])); Write(#13);

			// ����������
			Sleep(300);
		end;

	// ��� ������ �������
	Write(#10#13#10#13);

	// ��������� ���� �� ������ ��� ����� ������
	AbortProgram(' ', false);

	exit;
end.

