//******************************************************************************
// ������ E20-10.
// ���������� ��������� � ������������ ������������ ����� ������ �� ��� ������ ���
//******************************************************************************
program DacSample;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

const
	// ���-�� �������� � ������� (������� 32) ��� �. ReadData
	DataStep : DWORD = 64*1024;

var
	// ��������� ������ E20-10
	pModule : ILE2010;
	// ������ ���������� Lusbapi.dll
	DllVersion : DWORD;
	// ������������� ����������
	ModuleHandle : THandle;
	// �������� ������
	ModuleName: String;
	// �������� ������ ���� USB
	UsbSpeed : BYTE;
	// ��������� � ������ ����������� � ������
	ModuleDescription : MODULE_DESCRIPTION_E2010;

	// ������� ��� ���
	DacSample0, DacSample1 : SHORT;
	// ������� �����
	CurrentTime : double;

	// *** ��������������� ���������� ***
	i : WORD;
	Flag : WORD;
	// ������� ���������� �������� � ������ 
   DacSampleCounter,	DacSampleCounterModulo : DWORD;
	// ���������� ������������ ���������� ����� ���������� - ����������
	InputConsoleHandle : THandle;
	// ���������� ������������ ���������� ������ ���������� - �������
	OutputConsoleHandle : THandle;
	// ��� �������� ������������ �������� ����
	MaxX, MaxY : DWORD;
 	// ��������������� �������
	Str : string;

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
procedure AbortProgram(ErrorString: string; AbortionFlag : bool = true);
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
	if ErrorString <> ' ' then MessageBox(HWND(nil), pCHAR(ErrorString), '������!!!', MB_OK + MB_ICONINFORMATION);
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

	// ��������� �������� ��������� �� ��������� ��� ������ E20-10
	pModule := CreateLInstance(pCHAR('e2010'));
	if pModule = nil then AbortProgram('�� ���� ����� ��������� ������ E20-10!')
	else WriteLn(' Module Interface --> OK');

	// ��������� ���������� ������ E20-10 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for i := 0 to (MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI-1) do if pModule.OpenLDevice(i) then break;
	// ���-������ ����������?
	if i = MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI then AbortProgram('�� ������� ���������� ������ E20-10 � ������ 127 ����������� ������!')
	else WriteLn(Format(' OpenLDevice(%u) --> OK', [i]));

	// ������� ������������� ����������
	ModuleHandle := pModule.GetModuleHandle();
	if ModuleHandle = INVALID_HANDLE_VALUE then AbortProgram('�� ������� �������� ���������� ����������!')
	else WriteLn(' GetModuleHandle() --> OK');

	// ��������� �������� ������ � ������� ����������� �����
	ModuleName := '0123456';
	if not pModule.GetModuleName(pCHAR(ModuleName)) then AbortProgram('�� ���� ��������� �������� ������!')
	else WriteLn(' GetModuleName() --> OK');
	// ��������, ��� ��� ������ E20-10
	if Boolean(AnsiCompareStr(ModuleName, 'E20-10')) then AbortProgram('������������ ������ �� �������� E20-10!')
	else WriteLn(' The module is ''E20-10''');

	// ��������� �������� �������� ������ ���� USB
	if not pModule.GetUsbSpeed(@UsbSpeed) then AbortProgram(' �� ���� ���������� �������� ������ ���� USB')
	else WriteLn(' GetUsbSpeed() --> OK\n');
	// ������ ��������� �������� ������ ���� USB
	if UsbSpeed = USB11_LUSBAPI then
		begin
	   	Str := 'Full-Speed Mode (12 Mbit/s)';
			DacSampleCounterModulo := 50;
		end
	else
		begin
			Str := 'High-Speed Mode (480 Mbit/s)';
			DacSampleCounterModulo := 300;
		end;
	WriteLn(Format('   USB is in %s', [Str]));

	// ������ ������� ����� ������ ������������ �������� DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('�� ���� �������� ���������� � ������!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// �������� ���� �� �� ������ ���
	if ModuleDescription.Dac.Active = BOOL(DAC_INACCESSIBLED_E2010) then AbortProgram(' DAC is absent on this module E20-10!');

	// ��������� ��������� ������ ������ �� ����� ������ �� ������ ��������
	WriteLn('');
	Write  (' Module E20-10 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s (%s)', [StrPas(@ModuleDescription.Mcu.Version.FwVersion.Version), StrPas(@ModuleDescription.Mcu.Version.FwVersion.Date)]));
	// ������� ������ �������
	Write(#10#13);
	WriteLn('   Now SINUS signal is on the first DAC channel and ');
	WriteLn('     -SINUS signal is on the second DAC channel.');
	WriteLn('      (Press ESC key to terminate the program)');

	// ���� ����������� ������ ������� DAC_SAMPLE().
	// ��� ���� �� ������ ������ ��� ����� ��������� �����,
	// � �� ������ ������ ��� ����� ��������� -�����.
	// �.�. ������� �� ������� ��� ����� ��������� � ����������� ;)
	Flag := 0;
	CurrentTime := 0.0;
	DacSampleCounter := 0;
	while true do
	   begin
			DacSample0 := Round(2000.0*sin(2.0*PI*2.5*CurrentTime));
			DacSample1 := Round(-DacSample0);
			CurrentTime := CurrentTime + 1.0/3000.0;

			// ������� ��������� ������ �� ������ ����� ���
			if not pModule.DAC_SAMPLE(@DacSample0, 0) then begin WriteLn(#10#13' DAC_SAMPLE(0, ) -> Bad!'); break; end
			// ������� ��������� ������ �� ������ ����� ���
			else if not pModule.DAC_SAMPLE(@DacSample1, 1) then begin WriteLn(#10#13' DAC_SAMPLE(0, ) -> Bad!'); break; end
			// ������� �� ����� �� ������� ������� ESC
			else if IsEscKeyPressed() then break;

			// ������� ������ �������� � �������
			Inc(DacSampleCounter);
			if not BOOL(DacSampleCounter mod DacSampleCounterModulo) then
			   begin
					Flag := Flag xor $1;
					if BOOL(Flag) then Write('                                             ')
					else Write(' ............................................');
					Write(#13);
				end;
		end;

	// ������� ��� ������ ���
	DacSample0 := 0; DacSample1 := 0;
	if not pModule.DAC_SAMPLE(@DacSample0, 0) then begin WriteLn(#10#13' DAC_SAMPLE(0, ) -> Bad!'); end
	else if not pModule.DAC_SAMPLE(@DacSample1, 1) then begin WriteLn(#10#13' DAC_SAMPLE(0, ) -> Bad!'); end;

	// ��� ������ �������
	Write(#10#13#10#13);

	// ��������� ���� �� ������ ��� ����� ������
	AbortProgram(' ', false);
	// ���������� ����������
	exit;
end.

