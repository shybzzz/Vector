//******************************************************************************
// ������ E14-440.
// ���������� ��������� � ������������ ���������� ������ ������ �� ���
// ��� ���������� �������: ����� � �������� 1.23 ��� � ���������� 2000 ����� ���
// ����� �������������� �� ������� 125 ���.
//******************************************************************************
program WriteData;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

const
	// ���-�� �������� � ������� (������� 256) ��� �. WriteData
	DataStep : DWORD = 128*1024;
	// ����� ������ ��� ��� ������ ������: 0 ��� 1
	DacNumber : WORD = $0;
	// ������� ������ ������
	DacRate : double  = 125.0;
	// ��������� ���������� ��������������� �������
	SignalFrequency : double = 1.23;			// � ���
	SignalAmplitude : double = 2000.0;		// � ����� ���


type
	TShortrArray = array [0..1] of array of SHORT;

var
	// ������������� ������ ������
	hWriteThread : THANDLE;
	WriteTid : DWORD;

	// ��������� ������ E14-440
	pModule : ILE440;
	// ������ ���������� Lusbapi.dll
	DllVersion : DWORD;
	// ������������� ����������
	ModuleHandle : THandle;
	// �������� ������
	ModuleName: String;
	// �������� ������ ���� USB
	UsbSpeed : BYTE;
	// ��������� � ������ ����������� � ������
	ModuleDescription : MODULE_DESCRIPTION_E440;
	// ��������� ���������� ������ ���
	dp : DAC_PARS_E440;

	// �������� �������-���������
	Counter, OldCounter : DWORD;
	// ��������� �� ����� ��� ������
	Buffer : TShortrArray;

	// ������� ����� ���� ������� �������
	CurrentTime : double;				// � ���

	// ����� ������ ��� ���������� ������ ������ ������
	WriteThreadErrorNumber : WORD;
	// ������ ���������� ������� ������ ������
	IsWriteThreadComplete : boolean;

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
// �������� ���������� ���������� ���������� ������� �� ����� ������
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
					// ������ �������� ������ ��������� ������ ������
					WriteThreadErrorNumber := 3; Result := false; break;
				end
			else if IsEscKeyPressed() then
				begin
					// ��������� �������� (������ ������� ESC)
					WriteThreadErrorNumber := 4; Result := false; break;
				end
			else Sleep(20);
		end;
end;

//------------------------------------------------------------------------------
// ������� ������������ ������ ������ ������ ���������������
// ������� � ���������� �� FIFO ������ ��� � DSP ������
//------------------------------------------------------------------------------
function FillDacBufer : boolean;
var
	i : WORD;
begin
	Result := true;
	// ��������� ������ ��� ������ FIFO ������ ������
	for i := 0 to (dp.DacFifoLength-1) do
		begin
			Buffer[0][i] := Round(SignalAmplitude * sin(2.0*pi*SignalFrequency*CurrentTime));
			Buffer[0][i] := Buffer[0][i] and WORD($0FFF);
			Buffer[0][i] := Buffer[0][i] or (DacNumber shl 12);
			CurrentTime := CurrentTime + 1.0/dp.DacRate;
		end;
	// ��������� ������� FIFO ����� ������ � DSP ������
	if not pModule.PUT_DM_ARRAY(dp.DacFifoBaseAddress, dp.DacFifoLength, Pointer(Buffer[0])) then
//	if not pModule.PUT_DM_ARRAY(dp.DacFifoBaseAddress, dp.DacFifoLength, @Buffer[0][0]) then
									begin WriteThreadErrorNumber := 7; Result := false; end;
end;

//------------------------------------------------------------------------------
// ������������ ��������� ������ ������ ��������������� �������
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
//      ������ ����������� � �������� ���������� ������
//         ��� ������ ������ �� ��� ������ E14-440
//------------------------------------------------------------------------------
function WriteThread(var param : pointer): DWORD;
var
	i : WORD ;
	RequestNumber : WORD;
	// ������ OVERLAPPED �������� �� ���� ���������
	ReadOv : array[0..1] of OVERLAPPED;
	// ������ �������� � ����������� ������� �� ����/����� ������
	IoReq : array[0..1] of IO_REQUEST_LUSBAPI;

begin
	Result := 0;
	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if not pModule.STOP_DAC() then	begin	WriteThreadErrorNumber := 1; IsWriteThreadComplete := true; exit; end;

	// ��������� FIFO ����� ��� ��������������� ������� �������
	if not FillDacBufer() then begin	IsWriteThreadComplete := true; exit; end;

	// ��������� ����������� ��� ������ ������ ���������
	for i := 0 to 1 do
		begin
			// ������������� ��������� ���� OVERLAPPED
			ZeroMemory(@ReadOv[i], sizeof(OVERLAPPED));
			// ������ ������� ��� ������������ �������
			ReadOv[i].hEvent := CreateEvent(nil, FALSE , FALSE, nil);
			// ��������� ��������� IoReq
			IoReq[i].Buffer := Pointer(Buffer[i]);
			IoReq[i].NumberOfWordsToPass := DataStep;
			IoReq[i].NumberOfWordsPassed := 0;
			IoReq[i].Overlapped := @ReadOv[i];
			IoReq[i].TimeOut := Round(Int(DataStep/dp.DacRate)) + 1000;
		end;

	// ��������� � ������ ������ ������ ������ ��� ������ �� ���
	MakeDacData($0);
	// � ����� �� � ������ ������ ������ ��� ������ �� ���
	MakeDacData($1);

	// ������� ������� ������ ����������� ����� ������ �� ���
	RequestNumber := 0;
	if not pModule.WriteData(@IoReq[RequestNumber]) then
		begin
			CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent); WriteThreadErrorNumber := 2; IsWriteThreadComplete := true; exit;
		end;

	// � ������ ����� ��������� ����� ������ �� ���
	if pModule.START_DAC() then
   	begin
			// ������������ ���� ������ ������ �� ���
			while true do
				begin
					RequestNumber := RequestNumber xor $1;
					// ������� ������ �� ����� ��������� ������ ������
					if not pModule.WriteData(@IoReq[RequestNumber]) then
						begin
							WriteThreadErrorNumber := 2; break;
						end;

					// �������� ���������� ������� �� ����� ��������� ������ ������
					if not WaitingForRequestCompleted(IoReq[RequestNumber xor $1].Overlapped^) then break;
					// ��������� ��������� ������ ������ ��� ������ �� ���
					MakeDacData(RequestNumber xor $1);

					// ���� �� ������ ��� ������������ ������� ����� ������
					if WriteThreadErrorNumber <> 0 then break
					// ���� �� ��������� �������� (������ ������� ESC)?
					else if IsEscKeyPressed() then begin WriteThreadErrorNumber := 4; break; end
					// ��������� ����������               
					else Sleep(20);
					// ����������� ������� ������������ ������ ������
					Inc(Counter);
				end
		end
	else WriteThreadErrorNumber := 5;

	// ��������� ����� ������
	if not pModule.STOP_DAC() then WriteThreadErrorNumber := 1;
	// ���� ����, �� ������ ������������� ����������� ������
	if not CancelIo(ModuleHandle) then WriteThreadErrorNumber := 6;
	// ��������� �������������� �������
	CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent);
	// ����������   
	Sleep(100);   
	// ��������� ������ ��������� ������ ������ ������
	IsWriteThreadComplete := true;

end;

//------------------------------------------------------------------------------
// ��������� ���������� ���������
//------------------------------------------------------------------------------
procedure AbortProgram(ErrorString: string; AbortionFlag : bool = true);
var
	i : WORD ;
	// ����������� ������ ��� ���
	DacSample : SHORT;
begin
	// ��������� ��������� ������
	if pModule <> nil then
		begin
			// ������� ������� ������ �� ������ ����� ���
			DacSample := 0;
			if not pModule.DAC_SAMPLE(@DacSample, 0) then WriteLn(' DAC_SAMPLE() --> Bad')
			// � ����� �� ������ ����� ���
			else if not pModule.DAC_SAMPLE(@DacSample, 1) then WriteLn(' DAC_SAMPLE() --> Bad');

			// ��������� ��������� ������
			if not pModule.ReleaseLInstance() then  WriteLn(' ReleaseLInstance() --> Bad')
			else WriteLn(' ReleaseLInstance() --> OK');
			// ������� ��������� �� ��������� ������
			pModule := nil;
		end;

	// ��������� ������������� ������ ����� ������
	if hWriteThread = THANDLE(nil) then CloseHandle(hWriteThread);
	// ��������� ������ ��-��� ������� ������
	for i := 0 to 1 do Buffer[i] := nil;

	// ���� ����� - ������� ��������� � �������
	if ErrorString <> ' ' then MessageBox(HWND(nil), pCHAR(ErrorString), '������!!!', MB_OK + MB_ICONINFORMATION);
	// ���� ����� - �������� ��������� ���������
	if AbortionFlag = true then halt;
end;

//------------------------------------------------------------------------------
// ����������� ������ ��������� �� ����� ������ ������ ������ ������
//------------------------------------------------------------------------------
procedure ShowThreadErrorMessage;
begin
	case WriteThreadErrorNumber of
		$0 : ;
		$1 : WriteLn(' DAC Thread: STOP_DAC() --> Bad! :(((');
		$2 : WriteLn(' DAC Thread: WriteData() --> Bad :(((');
		$3 : WriteLn(' DAC Thread: Waiting data Error! :(((');
		// ���� ��������� ���� ������ ��������, ��������� ���� ��������
		$4 : WriteLn(' DAC Thread: The program was terminated! :(((');
		$5 : WriteLn(' DAC Thread: START_DAC() --> Bad :(((');
		$6 : WriteLn(' DAC Thread: Can''t complete input and output (I/O) operations! :(((');
		$7 : WriteLn(' DAC Thread: Can''t fill FIFO DAC buffer in DSP! :(((');
		else WriteLn(' DAC Thread: Unknown error! :(((');
	end;
end;

//******************************************************************************
//					  	� � � � � � � �    � � � � � � � � �
//******************************************************************************
begin
	// ������� ����� ������ ������ �����
	WriteThreadErrorNumber := 0;
	// ������� ������ ������������� ������ ����� ������
	IsWriteThreadComplete := false;
	// ������� ��������
	Counter := $0; OldCounter := $FFFF;

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

	// ��������� �������� ��������� �� ��������� ��� ������ E14-440
	pModule := CreateLInstance(pCHAR('e440'));
	if pModule = nil then AbortProgram('�� ���� ����� ��������� ������ E14-440!')
	else WriteLn(' Module Interface --> OK');

	// ��������� ���������� ������ E14-440 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for i := 0 to (MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI-1) do if pModule.OpenLDevice(i) then break;
	// ���-������ ����������?
	if i = MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI then AbortProgram('�� ������� ���������� ������ E14-440 � ������ 127 ����������� ������!')
	else WriteLn(Format(' OpenLDevice(%u) --> OK', [i]));

	// ������� ������������� ����������
	ModuleHandle := pModule.GetModuleHandle();

	// ��������� �������� ������ � ������� ����������� �����
	ModuleName := '0123456';
	if not pModule.GetModuleName(pCHAR(ModuleName)) then AbortProgram('�� ���� ��������� �������� ������!')
	else WriteLn(' GetModuleName() --> OK');
	// ��������, ��� ��� ������ E14-440
	if Boolean(AnsiCompareStr(ModuleName, 'E440')) then AbortProgram('������������ ������ �� �������� E14-440!')
	else WriteLn(' The module is ''E14-440''');

	// ��������� �������� �������� ������ ���� USB
	if not pModule.GetUsbSpeed(@UsbSpeed) then AbortProgram(' �� ���� ���������� �������� ������ ���� USB')
	else WriteLn(' GetUsbSpeed() --> OK\n');
	// ������ ��������� �������� ������ ���� USB
	if UsbSpeed = USB11_LUSBAPI then Str := 'Full-Speed Mode (12 Mbit/s)' else Str := 'High-Speed Mode (480 Mbit/s)';
	WriteLn(Format('   USB is in %s', [Str]));

	// ��� �������� DSP ������ �� ���������������� ������� DLL ���������� Lusbapi.dll
	if not pModule.LOAD_MODULE(nil) then AbortProgram('�� ���� ��������� ������ E14-440!')
	else WriteLn(' LOAD_MODULE() --> OK');

	// �������� �������� ������
 	if not pModule.TEST_MODULE() then AbortProgram('������ � �������� ������ E14-440!')
	else WriteLn(' TEST_MODULE() --> OK');

	// ������ ������� ����� ������ ������������ �������� DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('�� ���� �������� ���������� � ������!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// �������� ���� �� �� ������ ���
	if ModuleDescription.Dac.Active = BOOL(DAC_INACCESSIBLED_E440) then AbortProgram(' DAC is absent on this module E14-440!');

	// ������� ������� ��������� ������ ������ ������
	if not pModule.GET_DAC_PARS(@dp) then AbortProgram('�� ���� �������� ������� ��������� ������ ������!')
	else WriteLn(' GET_DAC_PARS --> OK');
	// ��������� �������� ��������� ������ ������ �� ������ E14-440
	dp.DacRate := DacRate;								// ������� ������ ������ � ���
	dp.DacFifoBaseAddress := $3000;					// ������� ����� FIFO ������ ��� � DSP ������
	dp.DacFifoLength := MAX_DAC_FIFO_SIZE_E440;	// ����� FIFO ������ ��� � DSP ������
	// ��������� � ������ ��������� ��������� �� ������ ������
	if not pModule.SET_DAC_PARS(@dp) then AbortProgram('�� ���� ���������� ��������� ������ ������!')
	else WriteLn(' SET_DAC_PARS --> OK');

	// ��������� �������� ������ ���-�� ������ ��� ������ ������
	for i := 0 to 1 do SetLength(Buffer[i], DataStep);

	// �������� ����� ������ ������
	WriteLn('');
	hWriteThread := CreateThread(nil, $2000, @WriteThread, nil, 0, WriteTid);
	if hWriteThread = THANDLE(nil) then AbortProgram('�� ���� ��������� ����� ������ ������!')
	else WriteLn(' Create WriteThread ... OK');

	// ��������� �� ������ �������� ��������� ������ ������ �� ������ ������
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

{ !!!  ����������� ���� ��������� �� ������ ������ !!!													}
	// ������� ������ ������������� ������ ���
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

	// ��� ��������� ������ ������ ������ ������
	WaitForSingleObject(hWriteThread, INFINITE);

	// ��� ������ �������
	Write(#10#13#10#13);

	// ��������� ���� �� ������ ��� ����� ������
	AbortProgram(' ', false);
	if WriteThreadErrorNumber <> 0 then ShowThreadErrorMessage()
	else Write  (' The program was completed successfully!!!');

	exit;
end.

