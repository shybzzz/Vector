//******************************************************************************
// ������ E20-10.
// ���������� ��������� � ������������ ���������� ����� ������ � ���
// � ������������� ������� ���������� ������ �� ���� � �������� �������� �������.
// ���� �������������� � ������ ������ ������� ��� �� ������� 5000 ���.
//******************************************************************************
program ReadData;

{$APPTYPE CONSOLE}

uses
  SysUtils, Windows, Lusbapi;

const
	// ������� ������ �� DataStep �������� ����� ������� � ����
	NBlockToRead : WORD = 4*20;
	// ���-�� �������� �������
	ChannelsQuantity : WORD = $04;
	// ������� ����� ������
	AdcRate : double  = 5000.0;

type
	TShortrArray = array [0..1] of array of SHORT;

var
	// ������������� ������ �����
	hReadThread : THANDLE;
	ReadTid : DWORD;

	// ������������� ����� ������
	FileHandle: Integer;

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
	// ��������� ���������� ������ ���
	ap : ADC_PARS_E2010;
	// ����� ����������������� ����
   UserFlash : USER_FLASH_E2010;
	// ��������� �������� ����� ������
	DataState : DATA_STATE_E2010;
	// ���-�� �������� � ������� ReadData
	DataStep : DWORD = 1024*1024;

	// �������� �������-���������
	Counter, OldCounter : WORD;
	// ��������� �� ����� ��� ������
	Buffer : TShortrArray;

	// ����� ������ ��� ���������� ������ ����� ������
	ReadThreadErrorNumber : WORD;
	// ������ ���������� ������� ����� ������
	IsReadThreadComplete : boolean;

	// *** ��������������� ���������� ***
	i, j : WORD;
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
// �������� ���������� ���������� ���������� ������� �� ���� ������
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
					// ������ �������� ����� ��������� ������ ������
					ReadThreadErrorNumber := 3; Result := false; break;
				end
			else if IsEscKeyPressed() then
				begin
					// ��������� �������� (������ ������� ESC)
					ReadThreadErrorNumber := 4; Result := false; break;
				end
			else Sleep(20);
		end;
end;

//------------------------------------------------------------------------------
//      ������ ����������� � �������� ���������� ������
//             ��� ����� ������ c ������ E20-10
//------------------------------------------------------------------------------
function ReadThread(var param : pointer): DWORD;
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
	if not pModule.STOP_ADC() then begin ReadThreadErrorNumber := 1; IsReadThreadComplete := true; exit; end;

	// ��������� ����������� ��� ����� ������ ���������
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
			IoReq[i].TimeOut := Round(Int(DataStep/ap.KadrRate)) + 1000;
		end;

	// ������� ������� ������ ����������� ���� ������ � Buffer
	RequestNumber := 0;
	if not pModule.ReadData(@IoReq[RequestNumber]) then
		begin
			CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent); ReadThreadErrorNumber := 2; IsReadThreadComplete := true; exit;
		end;

	// � ������ ����� ��������� ���� ������
	if pModule.START_ADC() then
   	begin
			// ���� ����� ������
			for i := 1 to (NBlockToRead-1) do
				begin
					RequestNumber := RequestNumber xor $1;
					// ������� ������ �� ��������� ������ �������� ������
					if not pModule.ReadData(@IoReq[RequestNumber]) then
						begin
							ReadThreadErrorNumber := 2; break;
						end;

					// �������� ���������� ���������� ������� �� ���� ������
//					if not WaitingForRequestCompleted(IoReq[RequestNumber xor $1].Overlapped^) then break;
					if not WaitForSingleObject(IoReq[RequestNumber xor $1].Overlapped.hEvent, IoReq[RequestNumber xor $1].TimeOut) = WAIT_TIMEOUT then begin ReadThreadErrorNumber := $03; break; end;

					// ��������� �������� ������� ��������� �������� ����� ������
					if not pModule.GET_DATA_STATE(@DataState) then begin ReadThreadErrorNumber := 7; break; end;
					// ������ ����� ��������� ���� ������� ������������ ����������� ������ ������
					if (DataState.BufferOverrun = (1 shl BUFFER_OVERRUN_E2010)) then begin ReadThreadErrorNumber := 8; break; end;

					// ����� ���� ��������� ������ ������
					if FileWrite(FileHandle, Buffer[RequestNumber xor $1][0], DataStep*sizeof(SHORT)) = -1 then begin ReadThreadErrorNumber := $5; break; end;

					// ��� ������� ������ �������� - ��� ��������� ����������� ���������� ������
//					if i = 33 then Sleep(1000);

					// ���� �� ������ ��� ������������ ������� ���� ������?
					if ReadThreadErrorNumber <> 0 then break
					// ���� �� ��������� �������� (������ ������� ESC)?
					else if IsEscKeyPressed() then begin ReadThreadErrorNumber := 4; break; end
					// ��������� ����������
					else Sleep(20);
					// ����������� ������� ���������� ������ ������
					Inc(Counter);
				end
		end
	else ReadThreadErrorNumber := 6;

	// ��������� ������ ������
	if ReadThreadErrorNumber = 0 then
		begin
			// ��� ��������� �������� ����� ��������� ������ ������
			if WaitingForRequestCompleted(IoReq[RequestNumber].Overlapped^) then
				begin
					// �������� ������� ���������� ������ ������
		         Inc(Counter);
					// ����� ���� ��������� ������ ������
					if FileWrite(FileHandle, Buffer[RequestNumber][0], DataStep*sizeof(SHORT)) = -1 then ReadThreadErrorNumber := $5;
				end;
		end;

	// ��������� ���� ������ c ���
	// !!!�����!!! ���� ���������� ����������� ���������� � �����������
	// ���� �������� ������, �� ������� STOP_ADC() ������� ��������� �� �������,
	// ��� ����� 800 �� ����� ��������� ����� ��������� ������ ������.
	// ��� �������� ������� ����� ������ � 5 ��� ��� �������� ���������� �����
	// ������������ ������������� FIFO ������ ������, ������� ����� ������ 8 ��. 
	if not pModule.STOP_ADC() then ReadThreadErrorNumber := 1;
	// ���� ����� - ����������� ������������� ������� ������������ ����������� ������ ������
	if (DataState.BufferOverrun <> (1 shl BUFFER_OVERRUN_E2010)) then
		begin
			// ��������� �������� ������������� ��������� �������� ����� ������
			if not pModule.GET_DATA_STATE(@DataState) then ReadThreadErrorNumber := 7
			// ������ ����� ��������� ���� ������� ������������ ����������� ������ ������
		   else if (DataState.BufferOverrun = (1 shl BUFFER_OVERRUN_E2010)) then ReadThreadErrorNumber := 8;
		end;
	// ���� ����, �� ������ ��� ������������� ����������� �������
	if not CancelIo(ModuleHandle) then ReadThreadErrorNumber := 9;
	// ��������� �������������� �������
	CloseHandle(IoReq[0].Overlapped.hEvent); CloseHandle(IoReq[1].Overlapped.hEvent);
	// ����������
	Sleep(100);
	// ��������� ������ ��������� ������ ����� ������
	IsReadThreadComplete := true;

end;

//------------------------------------------------------------------------------
// ��������� ���������� ���������
//------------------------------------------------------------------------------
procedure AbortProgram(ErrorString: string; AbortionFlag : bool = true);
var
	i : WORD ;
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

	// ��������� ������������� ������ ����� ������
	if hReadThread = THANDLE(nil) then CloseHandle(hReadThread);
	// ������� ���� ������
	if FileHandle <> -1 then FileClose(FileHandle);
	// ��������� ������ ��-��� ������� ������
	for i := 0 to 1 do Buffer[i] := nil;

	// ���� ����� - ������� ��������� � �������
	if ErrorString <> ' ' then MessageBox(HWND(nil), pCHAR(ErrorString), '������!!!', MB_OK + MB_ICONINFORMATION);
	// ���� ����� - �������� ��������� ���������
	if AbortionFlag = true then halt;
end;

//------------------------------------------------------------------------------
// ����������� ������ ��������� �� ����� ������ ������ ����� ������
//------------------------------------------------------------------------------
procedure ShowThreadErrorMessage;
begin
	case ReadThreadErrorNumber of
		$0 : ;
		$1 : WriteLn(' ADC Thread: STOP_ADC() --> Bad! :(((');
		$2 : WriteLn(' ADC Thread: ReadData() --> Bad :(((');
		$3 : WriteLn(' ADC Thread: Waiting data Error! :(((');
		// ���� ��������� ���� ������ ��������, ��������� ���� ��������
		$4 : WriteLn(' ADC Thread: The program was terminated! :(((');
		$5 : WriteLn(' ADC Thread: Writing data file error! :(((');
		$6 : WriteLn(' ADC Thread: START_ADC() --> Bad :(((');
		$7 : WriteLn(' ADC Thread: GET_DATA_STATE() --> Bad :(((');
		$8 : WriteLn(' ADC Thread: BUFFER OVERRUN --> Bad :(((');
		$9 : WriteLn(' ADC Thread: Can''t cancel pending input and output (I/O) operations! :(((');
		else WriteLn(' ADC Thread: Unknown error! :(((');
	end;
end;

//******************************************************************************
//					  	� � � � � � � �    � � � � � � � � �
//******************************************************************************
begin
	// ������� ����� ������ ������ �����
	ReadThreadErrorNumber := 0;
	// ������� ������ ������������� ������ ����� ������
	IsReadThreadComplete := false;
	// ���� �������� ����� ��� :(
	FileHandle := -1;
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
	if UsbSpeed = USB11_LUSBAPI then Str := 'Full-Speed Mode (12 Mbit/s)' else Str := 'High-Speed Mode (480 Mbit/s)';
	WriteLn(Format('   USB is in %s', [Str]));

	// ����� ��� ���� ������ �� ���������������� ������� DLL ���������� Lusbapi.dll
	if not pModule.LOAD_MODULE(nil) then AbortProgram('�� ���� ��������� ������ E20-10!')
	else WriteLn(' LOAD_MODULE() --> OK');

	// �������� �������� ������
 	if not pModule.TEST_MODULE() then AbortProgram('������ � �������� ������ E20-10!')
	else WriteLn(' TEST_MODULE() --> OK');

	// ������ ������� ����� ������ ������������ �������� DSP
	if not pModule.GET_MODULE_DESCRIPTION(@ModuleDescription) then AbortProgram('�� ���� �������� ���������� � ������!')
	else WriteLn(' GET_MODULE_DESCRIPTION() --> OK');

	// ��������� ��������� ���������� ����������������� ����
	if not pModule.READ_FLASH_ARRAY(@UserFlash) then AbortProgram('�� ���� ��������� ���������������� ����!')
	else WriteLn(' READ_FLASH_ARRAY() --> OK');

	// ������� ������� ��������� ������ ����� ������
	if not pModule.GET_ADC_PARS(@ap) then AbortProgram('�� ���� �������� ������� ��������� ����� ������!')
	else WriteLn(' GET_ADC_PARS --> OK');

	// ��������� �������� ��������� ����� ������ � ������ E20-10
	if ModuleDescription.Module.Revision = BYTE(REVISIONS_E2010[REVISION_A_E2010]) then
		ap.IsAdcCorrectionEnabled := FALSE				// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.A)
	else
		begin
			ap.IsAdcCorrectionEnabled := TRUE; 				// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.B � ����)
			ap.SynchroPars.StartDelay := 0;
			ap.SynchroPars.StopAfterNKadrs := 0;
			ap.SynchroPars.SynchroAdMode := NO_ANALOG_SYNCHRO_E2010;
//			ap.SynchroPars.SynchroAdMode := ANALOG_SYNCHRO_ON_HIGH_LEVEL_E2010;
			ap.SynchroPars.SynchroAdChannel := $0;
			ap.SynchroPars.SynchroAdPorog := 0;
			ap.SynchroPars.IsBlockDataMarkerEnabled := $0;
		end;
	ap.SynchroPars.StartSource := INT_ADC_START_E2010;			// ���������� ����� ����� � ���
//	ap.SynchroPars.StartSource := EXT_ADC_START_ON_RISING_EDGE_E2010;		// ������� ����� ����� � ���
	ap.SynchroPars.SynhroSource := INT_ADC_CLOCK_E2010;			// ���������� �������� �������� ���
//	ap.OverloadMode := MARKER_OVERLOAD_E2010;			// �������� ����� ���������� ������� ������� ��� ������ �������� � ������� ��� (������ ��� Rev.A)
	ap.OverloadMode := CLIPPING_OVERLOAD_E2010;		// ������� �������� ����� ���������� ������� ������� ���� ����������� ������� ��� (������ ��� Rev.A)
	ap.ChannelsQuantity := ChannelsQuantity; 			// ���-�� �������� �������
	for i:=0 to (ap.ChannelsQuantity-1) do ap.ControlTable[i] := i;
	// ������� ����� ����� ������������� � ����������� �� �������� USB
	ap.AdcRate := AdcRate;														// ������� ��� ������ � ���
	if UsbSpeed = USB11_LUSBAPI then
		begin
			ap.InterKadrDelay := 0.01;		// ����������� �������� � ��
			DataStep := 256*1024;			// ������ �������
		end
	else
		begin
			ap.InterKadrDelay := 0.0;		// ����������� �������� � ��
			DataStep := 1024*1024;			// ������ �������
		end;
	// ���������� ������� ������
	for i:=0 to (ADC_CHANNELS_QUANTITY_E2010-1) do
		begin
			ap.InputRange[i] := ADC_INPUT_RANGE_3000mV_E2010; 	// ������� �������� 3�
			ap.InputSwitch[i] := ADC_INPUT_SIGNAL_E2010;			// �������� ����� - ������
		end;
	// ������� � ��������� ���������� ������ ��� ���������������� ������������ ���
	for i:=0 to (ADC_INPUT_RANGES_QUANTITY_E2010-1) do
		for j:=0 to (ADC_CHANNELS_QUANTITY_E2010-1) do
		begin
			// ������������� ��������
			ap.AdcOffsetCoefs[i][j] := ModuleDescription.Adc.OffsetCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
			// ������������� ��������
			ap.AdcScaleCoefs[i][j] := ModuleDescription.Adc.ScaleCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
		end;

	// ��������� � ������ ��������� ��������� �� ����� ������
	if not pModule.SET_ADC_PARS(@ap) then AbortProgram('�� ���� ���������� ��������� ����� ������!')
	else WriteLn(' SET_ADC_PARS --> OK');

	// ��������� �������� ������ ���-�� ������ ��� ������ ������
	for i := 0 to 1 do begin SetLength(Buffer[i], DataStep); ZeroMemory(Buffer[i], DataStep*SizeOf(SHORT)); end;

	// ��������� ������� ���� ��� ������ ��������� ������
	WriteLn('');
	FileHandle := FileCreate('Test.dat');
	if FileHandle = -1 then AbortProgram('�� ���� ������� ���� Test.dat ��� ������ ���������� ������!')
	else WriteLn(' Create file Test.dat ... OK');

	// �������� ����� ����� ������
	hReadThread := CreateThread(nil, $2000, @ReadThread, nil, 0, ReadTid);
	if hReadThread = THANDLE(nil) then AbortProgram('�� ���� ��������� ����� ����� ������!')
	else WriteLn(' Create ReadThread ... OK');

	// ��������� ��������� ������ ������ �� ����� ������ �� ������ ��������
	WriteLn('');
	Write  (' Module E20-10 (S/N ', StrPas(@ModuleDescription.Module.SerialNumber)); WriteLn(') is ready ... ');
	WriteLn('   Module Info:');
	WriteLn(Format('     Module  Revision   is ''%1.1s''', [StrPas(@ModuleDescription.Module.Revision)]));
	WriteLn(Format('     MCU Driver Version is %s (%s)', [StrPas(@ModuleDescription.Mcu.Version.FwVersion.Version), StrPas(@ModuleDescription.Mcu.Version.FwVersion.Date)]));
	WriteLn(Format('     PLD    Version     is %s (%s)', [StrPas(@ModuleDescription.Pld.Version.Version), StrPas(@ModuleDescription.Pld.Version.Date)]));
	WriteLn('   ADC parameters:');
	WriteLn('     ChannelsQuantity = ', ap.ChannelsQuantity);
	WriteLn(Format('     AdcRate = %5.3f kHz', [ap.AdcRate]));
	WriteLn(Format('     InterKadrDelay = %2.4f ms', [ap.InterKadrDelay]));
	WriteLn(Format('     KadrRate =  %5.3f kHz', [ap.KadrRate]));

{ !!!  �������� ���� ��������� �������� ����� ����� ������ !!!													}
	WriteLn(#10#13' You can press ESC key to terminate the program...'#10#13);
	repeat
		if Counter <> OldCounter then
			begin
				Write(Format(' Counter %3u from %3u'#13, [Counter, NBlockToRead]));
				OldCounter := Counter;
			end
		else Sleep(20);
	until IsReadThreadComplete;

	// ��� ��������� ������ ������ ����� ������
	WaitForSingleObject(hReadThread, INFINITE);

	// ��� ������ �������
	Write(#10#13#10#13);

	// ��������� ���� �� ������ ��� ����� ������
	AbortProgram(' ', false);
	if ReadThreadErrorNumber <> 0 then ShowThreadErrorMessage()
	else Write  (' The program was completed successfully!!!');

	exit;
end.

