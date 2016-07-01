//******************************************************************************
// ������ E14-154.
// ���������� ��������� � ������������ ���������� ����� ������ � ���
// � ������������� ������� ���������� ������ �� ���� � �������� �������� �������.
// ���� �������������� � ������ ������� ������� ��� �� ������� ������ ��� 100 ���
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

#define CHANNELS_QUANTITY			(0x4)

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);
// ������� ������ ����� ������ � ���
DWORD WINAPI ServiceReadThread(PVOID /*Context*/);
// ������� ������ ��������� � ��������
void ShowThreadErrorMessage(void);

// ������������� �����
HANDLE hFile;
DWORD FileBytesWritten;

// ������������� ������ ����� ������
HANDLE hReadThread;
DWORD ReadTid;

// ������ ����������
DWORD DllVersion;
// ��������� �� ��������� ������
ILE154 *pModule;
// �������� ������
char ModuleName[7];
// �������� ������ ���� USB
BYTE UsbSpeed;
// ��������� � ������ ����������� � ������
MODULE_DESCRIPTION_E154 ModuleDescription;
// ��������� ���������� ������ ��� ������
ADC_PARS_E154 ap;

// ���-�� ���������� �������� (������� 32) ��� �. ReadData()
DWORD DataStep = 64*1024;
// ����� �������� NDataBlock ������ �� DataStep �������� � ������
const WORD NDataBlock = 60;
// ����� ������
SHORT *AdcBuffer;

// ������ ���������� ������ ������ ����� ������
bool IsReadThreadComplete, IsThreadStopping;
// ����� ������ ��� ���������� ����� ������
WORD ReadThreadErrorNumber;

// �������� �������-���������
DWORD Counter = 0x0, OldCounter = 0xFFFFFFFF;

WORD TtlIn;


//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
	WORD DacSample;
   DWORD FifoOverflowFlag, FifoSize, MaxFifoBytesUsed;
   double FifoMaxPercentLoad;
   DWORD x, y;

	// ������� ������ ���������� ������ ����� ������
	IsReadThreadComplete = false;
	// ���� ������ �� �������� ��� ����� ������
	AdcBuffer = NULL;
	// ���� �� ������ ����� ����� ������
	hReadThread = NULL;
	// ���� �������� ����� ��� :(
	hFile = INVALID_HANDLE_VALUE;
	// ������� ���� ������ ������ ����� ������
	ReadThreadErrorNumber = 0x0;

	// �������� ����� ��������
	clrscr();

	printf(" ***************************************\n");
	printf(" Module E14-154                         \n");
	printf(" Console example for ADC Stream Reading \n");
	printf(" ***************************************\n\n");

	// �������� ������ ������������ ���������� Lusbapi.dll
	if((DllVersion = GetDllVersion()) != CURRENT_VERSION_LUSBAPI)
	{
		char String[128];
		sprintf(String, " Lusbapi.dll Version Error!!!\n   Current: %1u.%1u. Required: %1u.%1u",
											DllVersion >> 0x10, DllVersion & 0xFFFF,
											CURRENT_VERSION_LUSBAPI >> 0x10, CURRENT_VERSION_LUSBAPI & 0xFFFF);

		AbortProgram(String);
	}
	else printf(" Lusbapi.dll Version --> OK\n");

	// ��������� �������� ��������� �� ���������
	pModule = static_cast<ILE154 *>(CreateLInstance("e154"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// ��������� ���������� ������ E14-154 � ������ WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// ���-������ ����������?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E14-154 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// ��������, ��� ��� 'E14-154'
	if(strcmp(ModuleName, "E154")) AbortProgram(" The module is not 'E14-154'\n");
	else printf(" The module is 'E14-154'\n");

	// ��������� �������� �������� ������ ���� USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// ������ ��������� �������� ������ ���� USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// ������� ���������� �� ���� ������
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { printf(" GET_MODULE_DESCRIPTION() --> Bad\n"); exit(1); }
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// ������� ������� ��������� ������ ���
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");
	// ��������� �������� ��������� ���
	ap.ClkSource = INT_ADC_CLOCK_E154;							// ���������� ������ ���
	ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// ��� ���������� �������� �������� ���
	ap.InputMode = NO_SYNC_E154;		// ��� ������������� ����� ������
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 		// ���-�� �������� ������
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i | (ADC_INPUT_RANGE_5000mV_E154 << 0x6));
	ap.AdcRate = 120.0;					// ������� ������ ��� � ���
	ap.InterKadrDelay = 0.0;			// ����������� �������� � ��
	// ��������� ��������� ��������� ������ ��� � ������
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");

	// ������� ������ ��� �����
	AdcBuffer = new SHORT[2*DataStep];
	if(!AdcBuffer) AbortProgram(" Can not allocate memory\n");

	// ������� ���� ��� ������ ���������� � ������ ������
	printf(" \n");
	hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
	if(hFile == INVALID_HANDLE_VALUE) AbortProgram("\n Can't create file 'Test.dat'!\n");

	// ������ � ��������� ����� ����� ������
   IsThreadStopping=false;
	hReadThread = CreateThread(0, 0x2000, ServiceReadThread, 0, 0, &ReadTid);
	if(!hReadThread) AbortProgram(" ServiceReadThread() --> Bad\n");
	else printf(" ServiceReadThread() --> OK\n");

	// ��������� ��������� ����� ������ ������ �� ������ ��������
	printf(" \n");
	printf(" Module E14-154 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("   Adc parameters:\n");
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.3f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.4f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.3f kHz\n", ap.KadrRate);

	// ���� ������ ���������� ������ � �������� ��������� ������ ����������
	printf("\n Press any key if you want to terminate this program...\n\n");
   x=wherex();
   y=wherey();
	while(!IsReadThreadComplete)
	{
		if(OldCounter != Counter) { gotoxy(x, y); printf(" Counter %3u from %3u\r", Counter, NDataBlock); OldCounter = Counter; }
		else Sleep(20);
      if(IsThreadStopping != true)
      	{
   	   if(!pModule->TTL_IN(&TtlIn)) AbortProgram("\n\n  TTL_IN() --> Bad\n");
			if(!pModule->ENABLE_TTL_OUT(TRUE)) AbortProgram(" ������ ���������� �������� �������� �����");
 			if(!pModule->TTL_OUT(0xFFFF)) AbortProgram(" ������ ��������� �������� �������� �����");
			if(!pModule->TTL_OUT(0x0000)) AbortProgram(" ������ ��������� �������� �������� �����");
			if(!pModule->DAC_SAMPLE((SHORT *)&DacSample, 0x0)) AbortProgram(" ������ ��������� ������ ���");
	      if(!pModule->FIFO_STATUS(&FifoOverflowFlag, &FifoMaxPercentLoad, &FifoSize, &MaxFifoBytesUsed)) { AbortProgram(" FIFO_STATUS() --> Bad\n"); exit(1); }
   	   gotoxy(x,y+1);
	      printf(" FifoOverflow=%1u, FifoLoad=%7.2f%%, FifoSize=%5u, MaxFifoUsed=%5u", FifoOverflowFlag, FifoMaxPercentLoad, FifoSize, MaxFifoBytesUsed);
      	}
	}

	// ��� ��������� ������ ������ ����� ������
	WaitForSingleObject(hReadThread, INFINITE);

 	if(!pModule->STOP_ADC()) AbortProgram(" ������ STOP_ADC()");

	// �������� ���� �� ������ ���������� ������ ����� ������
	printf("\n\n");
	if(ReadThreadErrorNumber) { AbortProgram(NULL, false); ShowThreadErrorMessage(); }

	AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------
// �����, � ������� �������������� ���� ������
//------------------------------------------------------------------------
DWORD WINAPI ServiceReadThread(PVOID /*Context*/)
{
	WORD RequestNumber;
	DWORD i;
	// ������������� ������� �� ���� �������
	HANDLE ReadEvent[2];
	// ������ OVERLAPPED �������� �� ���� ���������
	OVERLAPPED ReadOv[2];
//	DWORD BytesTransferred[2];
	IO_REQUEST_LUSBAPI IoReq[2];

	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if(!pModule->STOP_ADC()) { ReadThreadErrorNumber = 0x1; IsReadThreadComplete = true; return 0x0; }

	// ��������� ����������� ��� ����� ������ ���������
	for(i = 0x0; i < 0x2; i++)
	{
		// ������ �������
		ReadEvent[i] = CreateEvent(NULL, FALSE , FALSE, NULL);
		// ������������� ��������� ���� OVERLAPPED
		ZeroMemory(&ReadOv[i], sizeof(OVERLAPPED)); ReadOv[i].hEvent = ReadEvent[i];
		// ��������� ��������� IoReq
		IoReq[i].Buffer = AdcBuffer + i*DataStep;
		IoReq[i].NumberOfWordsToPass = DataStep;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &ReadOv[i];
		IoReq[i].TimeOut = DataStep/ap.AdcRate + 1000;
	}

	// ������ ��������������� ������ �� ���� ������
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// �������� ���
	if(pModule->START_ADC())
	{
		// ���� ����� ������
		for(i = 0x1; i < NDataBlock; i++)
		{
			// ������� ������ �� ��������� ������ ������
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }

			// ��� ���������� �������� ����� ���������� ������ ������
			if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; break; }

			// ������� ���������� ������ ������ � ����
			if(!WriteFile(	hFile,													// handle to file to write to
		    					IoReq[RequestNumber^0x1].Buffer,					// pointer to data to write to file
								2*DataStep,	 											// number of bytes to write
	    						&FileBytesWritten,									// pointer to number of bytes written
						   	NULL			  											// pointer to structure needed for overlapped I/O
							   )) { ReadThreadErrorNumber = 0x4; break; }

			if(ReadThreadErrorNumber) break;
			else if(kbhit()) { ReadThreadErrorNumber = 0x5; break; }
			else Sleep(20);
			Counter++;
		}
      IsThreadStopping=true;
		// ��������� ������ ������
		if(!ReadThreadErrorNumber)
		{
			RequestNumber ^= 0x1;
			// ��� ��������� �������� ����� ��������� ������ ������
			if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) ReadThreadErrorNumber = 0x3;
			// ������� ��������� ������ ������ � ����
			if(!WriteFile(	hFile,													// handle to file to write to
		    					IoReq[RequestNumber^0x1].Buffer,					// pointer to data to write to file
								2*DataStep,	 											// number of bytes to write
	    						&FileBytesWritten,									// pointer to number of bytes written
						   	NULL			  											// pointer to structure needed for overlapped I/O
							   )) ReadThreadErrorNumber = 0x4;
			Counter++;
		}
	}
	else { ReadThreadErrorNumber = 0x6; }

	// ������ �������� ������������� ����������� ������ �� ���� ������
	if(!CancelIo(pModule->GetModuleHandle())) { ReadThreadErrorNumber = 0x7; }
	// ��������� ��� �������������� �������
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadEvent[i]);
	// ��������� ��������
	Sleep(100);
	// ��������� ������ ���������� ������ ������ ����� ������
	IsReadThreadComplete = true;
	// ������ ����� �������� �������� �� ������
	return 0x0;
}

//------------------------------------------------------------------------
// ��������� ��������� � �������
//------------------------------------------------------------------------
void ShowThreadErrorMessage(void)
{
	switch(ReadThreadErrorNumber)
	{
		case 0x1:
			printf(" ADC Thread: STOP_ADC() --> Bad\n");
			break;

		case 0x2:
			printf(" ADC Thread: ReadData() --> Bad\n");
			break;

		case 0x3:
			printf(" ADC Thread: Timeout is occured!\n");
			break;

		case 0x4:
			printf(" ADC Thread: Writing file error!\n");
			break;

		case 0x5:
			// ���� ��������� ���� ������ ��������, ��������� ���� ��������
			printf(" ADC Thread: The program was terminated!\n");
			break;

		case 0x6:
			printf(" ADC Thread: START_ADC() --> Bad\n");
			break;

		case 0x7:
			printf(" ADC Thread: Can't cancel ending input and output (I/O) operations!\n");
			break;

		default:
			printf(" Unknown error!\n");
			break;
	}

	return;
}

//------------------------------------------------------------------------
// ��������� ���������� ���������
//------------------------------------------------------------------------
void AbortProgram(char *ErrorString, bool AbortionFlag)
{
	// ��������� ��������� ������
	if(pModule)
	{
		// ��������� ��������� ������
		if(!pModule->ReleaseLInstance()) printf(" ReleaseLInstance() --> Bad\n");
		else printf(" ReleaseLInstance() --> OK\n");
		// ������� ��������� �� ��������� ������
		pModule = NULL;
	}

	// ��������� ������ ������
	if(AdcBuffer) { delete[] AdcBuffer; AdcBuffer = NULL; }
	// ��������� ������������� ������ ����� ������
	if(hReadThread) { CloseHandle(hReadThread); hReadThread = NULL; }
	// ��������� ������������� ����� ������
	if(hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE; }

	// ������� ����� ���������
	if(ErrorString) printf(ErrorString);

	// ��������� ������� ����������
	if(kbhit()) { while(kbhit()) getch(); }

	// ���� ����� - �������� ��������� ���������
	if(AbortionFlag) exit(0x1);
	// ��� �������� ������� �� �������   
	else return;
}

