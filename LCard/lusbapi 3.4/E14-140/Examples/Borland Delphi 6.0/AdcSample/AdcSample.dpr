//******************************************************************************
// ������ E14-140.
// ���������� ��������� � �������� ����������� ������������ ����� ������� � ���
//	� ������� ������������ ������� ADC_SAMPLE().
// ���� �������������� � ������ ���� ������� ���.
//******************************************************************************
program DacSample;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

const
	InputRangeIndex : WORD  = ADC_INPUT_RANGE_2500mV_E140;
//	InputRangeIndex : WORD  = ADC_INPUT_RANGE_10000mV_E140;

var
	// ��������� ������ E14-140
	pModule : ILE140;
	// ������ ���������� Lusbapi.dll
	DllVersion : DWORD;
	// ������������� ����������
	ModuleHandle : THandle;
	// �������� ������
	ModuleName: String;
	// �������� ������ ���� USB
	UsbSpeed : BYTE;
	// ��������� � ������ ����������� � ������
	ModuleDescription : MODULE_DESCRIPTION_E140;

	// ������� ��� ���
	AdcSample0, AdcSample1 : SHORT;

	// *** ��������������� ���������� ***
	i : WORD;
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

	// ��������� �������� ��������� �� ��������� ��� ������ E14-140
	pModule := CreateLInstance(pCHAR('e140'));
	if pModule = nil then AbortProgram('�� ���� ����� ��������� ������ E14-140!')
	else WriteLn(' Module Interface --> OK');

	// ��������� ���������� ������ E14-140 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for i := 0 to (MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI-1) do if pModule.OpenLDevice(i) then break;
	// ���-������ ����������?
	if i = MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI then AbortProgram('�� ������� ���������� ������ E14-140 � ������ 127 ����������� ������!')
	else WriteLn(Format(' OpenLDevice(%u) --> OK', [i]));

	// ������� ������������� ����������
	ModuleHandle := pModule.GetModuleHandle();
	if ModuleHandle = INVALID_HANDLE_VALUE then AbortProgram('�� ������� �������� ���������� ����������!')
	else WriteLn(' GetModuleHandle() --> OK');

	// ��������� �������� ������ � ������� ����������� �����
	ModuleName := '0123456';
	if not pModule.GetModuleName(pCHAR(ModuleName)) then AbortProgram('�� ���� ��������� �������� ������!')
	else WriteLn(' GetModuleName() --> OK');
	// ��������, ��� ��� ������ E14-140
	if Boolean(AnsiCompareStr(ModuleName, 'E140')) then AbortProgram('������������ ������ �� �������� E14-140!')
	else WriteLn(' The module is ''E14-140''');

	// ��������� �������� �������� ������ ���� USB
	if not pModule.GetUsbSpeed(@UsbSpeed) then AbortProgram(' �� ���� ���������� �������� ������ ���� USB')
	else WriteLn(' GetUsbSpeed() --> OK\n');
	// ������ ��������� �������� ������ ���� USB
	if UsbSpeed = USB11_LUSBAPI then Str := 'Full-Speed Mode (12 Mbit/s)' else Str := 'High-Speed Mode (480 Mbit/s)';
	WriteLn(Format('   USB is in %s', [Str]));

	// ������ ������� ����� ������ ������������ �������� DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('�� ���� �������� ���������� � ������!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// ��������� ��������� ������ ������ �� ����� ������ �� ������ ��������
	WriteLn('');
	Write  (' Module E14-140 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s %s', [StrPas(@ModuleDescription.Mcu.Version.Version), StrPas(@ModuleDescription.Mcu.Version.Date)]));
	WriteLn('   ADC parameters:');
	WriteLn(Format('     Input Range = %6.2f Volt', [ADC_INPUT_RANGES_E140[InputRangeIndex]]));
	// ������� ������ �������
	Write(#10#13); WriteLn('   Press ESC key to terminate the program ...');
   Write(#10#13);
	WriteLn('   ADC Channel:                    1         2');

	// ���� ����������� ������ ������� ADC_SAMPLE().
	while true do
	   begin
			// ������� ��������� ������ �� ������ ����� ���
			if not pModule.ADC_SAMPLE(@AdcSample0, 0 or (0 shl 4) or (InputRangeIndex shl 6)) then begin WriteLn(#10#13' ADC_SAMPLE(0, ) -> Bad!'); break; end
			// ������� ��������� ������ �� ������ ����� ���
			else if not pModule.ADC_SAMPLE(@AdcSample1, 1 or (1 shl 4) or (InputRangeIndex shl 6)) then begin WriteLn(#10#13' ADC_SAMPLE(0, ) -> Bad!'); break; end
			// ������� �� ����� �� ������� ������� ESC
			else if IsEscKeyPressed() then break;
			Write(Format('   AdcSample Data (ADC code): = %5d     %5d', [AdcSample0, AdcSample1])); Write(#13);
		end;

	// ��� ������ �������
	Write(#10#13#10#13);

	// ��������� ���� �� ������ ��� ����� ������
	AbortProgram(' ', false);
	// ���������� ����������
	exit;
end.

