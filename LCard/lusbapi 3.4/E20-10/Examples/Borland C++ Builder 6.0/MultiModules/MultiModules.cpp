//******************************************************************************
// ������ E20-10.
// ���������� ��������� ���������� ��������� ���� ������ ������������
// � ���������� �������.
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "Lusbapi.h"

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);
// ������� ������ ����� ������
DWORD WINAPI ServiceReadThread(PVOID ThreadIndex);
// �������� ���������� ������������ ������� �� ���� ������
BOOL WaitingForIoRequestCompleted(OVERLAPPED *ReadOv, DWORD ThreadTid);

// ���-�� ������������ ����������� ������
const WORD MaxVirtualSoltsQuantity	= 0x7;
// ���-�� ������������ ������� ��� ������� ������
const WORD ADC_CHANNELS_QUANTITY 		= 0x4;
// ������� ������ ��� ��� ����� ������
const double AdcRate = 2500.0;

// �������������� ������
HANDLE 	ReadThreadHandle[MaxVirtualSoltsQuantity];
DWORD		ReadThreadIndex[MaxVirtualSoltsQuantity];
DWORD		ReadThreadTid[MaxVirtualSoltsQuantity];

// ������ ���������� �� ��������� ������� ���� E20-10
ILE2010 *pModules[MaxVirtualSoltsQuantity];
// ������ ����������
DWORD DllVersion;
// ���������� ����������
HANDLE ModuleHandle;
// �������� ������
char ModuleName[7];
// �������� ������ ���� USB
BYTE UsbSpeed;
// ��������� � ������ ����������� � ������
MODULE_DESCRIPTION_E2010 ModuleDescription;
// ��������� ���������� ������ ��� ������
ADC_PARS_E2010 ap;
// ���-�� ������� ������������ ������� ���� E20-10
WORD ModulesQuantity;

// ������ ������� �� ���� ������ ReadData()
DWORD DataStep[MaxVirtualSoltsQuantity];
// ������ ������� ��� ���������� ������
SHORT *Buffer[MaxVirtualSoltsQuantity];
// ������-������� ���������� ���������� ���������������� ������
bool IsReadThreadTerminated[MaxVirtualSoltsQuantity];
// ����� ������ ������� �� �������
WORD ReadThreadErrorNumber[MaxVirtualSoltsQuantity];
// ����� ������ ���������� �������� ���������
WORD MainErrorNumber;

// �������� ���������� ������ �������� ���������
WORD XCoordCounter, YCoordCounter;
// ������� �������
WORD Counter[MaxVirtualSoltsQuantity];

// ����������� ������
CRITICAL_SECTION cs;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i, j, k;

	// ����������������� ��������� � ����������
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		// ������� ��������� �� ���������
		pModules[i] = NULL;
		// ������� ������ ���������� ������ ����� ������
		IsReadThreadTerminated[i] = false;
		// ���� ������ �� �������� ��� ����� ������
		Buffer[i] = NULL;
		// ���� �� ������ ����� ����� ������
		ReadThreadHandle[i] = NULL;
		// ������� ���� ������ ������ ����� ������
		ReadThreadErrorNumber[i] = 0x0;
	}

	// �������� ����� ��������
	clrscr();

	printf(" ***********************************\n");
	printf(" Module E20-10                     \n");
	printf(" Console example for Multi Modules  \n");
	printf(" ***********************************\n\n");

	// ������������� ����������� ������
	InitializeCriticalSection(&cs);

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

	printf("\n E20-10 modules List:\n");
	// �������� ������� ������������ �������
	ModulesQuantity = 0x0;
	// ��������� ������ MaxVirtualSoltsQuantity ����������� ������ � ������� ������� ���� E20-10
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		// ������� ��������� �� ���������
		pModules[i] = NULL;
		// ������� ��������� �� ���������
		pModules[i] = static_cast<ILE2010 *>(CreateLInstance("e2010"));
		if(!pModules[i]) AbortProgram(" Module Interface --> Bad\n");
		// ��������� ���������� ������ ���� E20-10 � ��������������� ����������� �����
		else if(!pModules[i]->OpenLDevice(i))  { pModules[i]->ReleaseLInstance(); pModules[i] = NULL; continue; }
		// ��������� �������� ������ � ������� ����������� �����
		else if(!pModules[i]->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
		// ��������, ��� ��� E20-10
		else if(strcmp(ModuleName, "E20-10")) AbortProgram(" The detected module is not E20-10. The appication is terminated.\n");
		// ��������� ������ �������� USB ��� �������� ������
		else if(!pModules[i]->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
		// �������� ������� ����� ���� �� �������� Lusnapi.dll
		else if(!pModules[i]->LOAD_MODULE()) AbortProgram(" LOAD_MODULE() --> Bad\n");
		// �������� �������� ���� ������
	 	else if(!pModules[i]->TEST_MODULE()) AbortProgram(" TEST_MODULE() --> Bad\n");
		// ������� ���������� �� ���� ������
		else if(!pModules[i]->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");

		// ������� ������� ��������� ������ ���
		if(!pModules[i]->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
		if(ModuleDescription.Module.Revision == REVISIONS_E2010[REVISION_A_E2010])
			ap.IsAdcCorrectionEnabled = FALSE;		// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.A)
		else
			ap.IsAdcCorrectionEnabled = TRUE;		// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.B � ����)
		// ��������� �������� ��������� ���
		ap.SynchroPars.StartSource = INT_ADC_START_E2010;			// ���������� ����� ���
//		ap.SynchroPars.StartSource = EXT_ADC_START_ON_RISING_EDGE_E2010;		// ������� ����� ���
		ap.SynchroPars.SynhroSource = INT_ADC_CLOCK_E2010;			// ���������� �������������� ���
		ap.SynchroPars.StartDelay = 0x0;									// �������� ������ ����� ������ � ������ �������� (��� Rev.B � ����)
		ap.SynchroPars.StopAfterNKadrs = 0x0;							// ������� ����� ������ ����� �������� ���-�� ������ �������� (��� Rev.B � ����)
		ap.SynchroPars.SynchroAdMode = NO_ANALOG_SYNCHRO_E2010;	// ��� ���������� ������������� (��� Rev.B � ����)
//		ap.SynchroPars.SynchroAdMode = ANALOG_SYNCHRO_ON_HIGH_LEVEL_E2010;
		ap.SynchroPars.SynchroAdChannel = 0x0;							// ����� ���������� ������������� (��� Rev.B � ����)
		ap.SynchroPars.SynchroAdPorog = 0;								// ����� ���������� ������������� � ����� ��� (��� Rev.B � ����)
		ap.SynchroPars.IsBlockDataMarkerEnabled = 0x0;				// ������������ ������ ����� ������ (������, ��������, ��� ���������� ������������� ����� �� ������) (��� Rev.B � ����)
		ap.ChannelsQuantity = ADC_CHANNELS_QUANTITY; 				// ���-�� �������� �������
		for(j = 0x0; j < ADC_CHANNELS_QUANTITY; j++) ap.ControlTable[j] = (WORD)(j);
		// ������� ����� ����� ������������� � ����������� �� �������� USB
		ap.AdcRate = AdcRate; 													// ������� ������ ��� � ���
		if(UsbSpeed == USB11_LUSBAPI)
		{
			ap.InterKadrDelay = 0.01;		// ����������� �������� � ��
			DataStep[i] = 256*1024;			// ������ �������
		}
		else
		{
			ap.InterKadrDelay = 0.0;		// ����������� �������� � ��
			DataStep[i] = 1024*1024;		// ������ �������
		}
		// ����������� ������� ������
		for(j = 0x0; j < ADC_CHANNELS_QUANTITY_E2010; j++)
		{
			ap.InputRange[j] = ADC_INPUT_RANGE_3000mV_E2010;  	// ������� ��������� 3�
			ap.InputSwitch[j] = ADC_INPUT_SIGNAL_E2010;			// �������� ����� - ������
		}
		// ������� � ��������� ���������� ������ ��� ���������������� ������������ ���
		for(j = 0x0; j < ADC_INPUT_RANGES_QUANTITY_E2010; j++)
			for(k = 0x0; k  < ADC_CHANNELS_QUANTITY_E2010; k++)
			{
				// ������������� ��������
				ap.AdcOffsetCoefs[j][k] = ModuleDescription.Adc.OffsetCalibration[k + j*ADC_CHANNELS_QUANTITY_E2010];
				// ������������� ��������
				ap.AdcScaleCoefs[j][k] = ModuleDescription.Adc.ScaleCalibration[k + j*ADC_CHANNELS_QUANTITY_E2010];
			}
		// ��������� ��������� ��������� ������ ��� � ������
		if(!pModules[i]->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");

		// ������� ������ ��� ������� �����
		Buffer[i] = new SHORT[2*DataStep[i]];
		if(!Buffer[i]) AbortProgram(" Cannot allocate buffer memory.\n");

		// �������������� ������� ������������ �������
		ModulesQuantity++;
		printf("   %2u. Virtual Slot %2u. Module (S/N %s, %s) is ready ... \n", ModulesQuantity, i, ModuleDescription.Module.SerialNumber, UsbSpeed ? "HIGH-SPEED (480 Mbit/s)" : "FULL-SPEED (12 Mbit/s)");
	}

	// ��������: �� ����� ���� �����-������ ������?
	if(!ModulesQuantity) { printf("   Empty...\n");  AbortProgram("\n Can't detect any E20-10 module  :(((\n"); }

	// ������� ���������� �������
	XCoordCounter = (WORD)wherex();
	YCoordCounter = (WORD)wherey(); YCoordCounter += (WORD)0x3;

	// ��������� ��������� ����� ����� ������ ��� ������� �� ������������ ������
	MainErrorNumber = 0x0;
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		// ������� � ��������� ������ ����� ������
		if(!pModules[i]) continue;
		ReadThreadIndex[i] = i;
		ReadThreadHandle[i] = CreateThread(0, 0x2000, ServiceReadThread, &ReadThreadIndex[i], 0, &ReadThreadTid[i]);
		if(!ReadThreadHandle[i]) AbortProgram("\n CreateThread() --> Bad\n");
	}

	// ���� ���������� ������ ������� �� ����� ������� ����� ������� ����������
	printf("\n  Press any key to terminate the program ...\n\n");
	while(true)
	{
		if(kbhit()) { MainErrorNumber = 0x1; break; }
		Sleep(20);
	}

	// ���� ��������� ���� ������ ��������, ��������� ���� ��������
	if(MainErrorNumber == 0x1) AbortProgram(" The program was terminated successfully!\n", false);
	else AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------
// ����� ����� ������ � ������ E20-10
//------------------------------------------------------------------------
DWORD WINAPI ServiceReadThread(PVOID ThreadIndex)
{
	WORD i;
	// ����� �������
	WORD RequestNumber;
	// ����� �������� ������
	DWORD ti = *(DWORD *)ThreadIndex;
	// ������ OVERLAPPED ��������
	OVERLAPPED ReadOv[2];
	// ������ �������� � ����������� ������� �� ����/����� ������
	IO_REQUEST_LUSBAPI IoReq[2];

	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if(pModules[ti]->STOP_ADC())
	{
		// ��������� ����������� ��� ����� ������ ���������
		for(i = 0x0; i < 0x2; i++)
		{
			// ������������� ��������� ���� OVERLAPPED
			ZeroMemory(&ReadOv[i], sizeof(OVERLAPPED));
			// ������ ������� ��� ������������ �������
			ReadOv[i].hEvent = CreateEvent(NULL, FALSE , FALSE, NULL);
			// ��������� ��������� IoReq
			IoReq[i].Buffer = Buffer[ti] + i*DataStep[ti];
			IoReq[i].NumberOfWordsToPass = DataStep[ti];
			IoReq[i].NumberOfWordsPassed = 0x0;
			IoReq[i].Overlapped = &ReadOv[i];
			IoReq[i].TimeOut = DataStep[ti]/ap.KadrRate + 1000;
		}

		// ���������� ������� �������� ������
		EnterCriticalSection(&cs);
			gotoxy(XCoordCounter, YCoordCounter + 1*ti);
			printf("  Counter[%2u]: %8u", ti+0x1, Counter[ti] = 0x0);
		LeaveCriticalSection(&cs);

		// ������ ��������������� ������ �� ���� ������
		RequestNumber = 0x0;
		if(!pModules[ti]->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber[ti] = 0x2; goto ReadThreadFinish; }

		// ������ ��������� ���� ������
		if(pModules[ti]->START_ADC())
		{
			// ���� ����� ������
			while(!IsReadThreadTerminated[ti])
			{
				// ������� ������ �� ��������� ������ ������
				RequestNumber ^= 0x1;
				if(!pModules[ti]->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber[ti] = 0x2; break; }
				if(IsReadThreadTerminated[ti]) break;

				// ��� ���������� �������� ����� ���������� ������ ������
				if(!WaitingForIoRequestCompleted(IoReq[RequestNumber^0x1].Overlapped, ti)) break;
//				if(WaitForSingleObject(ReadOv[RequestNumber^0x1].hEvent, IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { /*ReadThreadErrorNumber = 0x3;*/ break; }
				if(IsReadThreadTerminated[ti]) break;

				// ��������� �������� �������� ������
				Counter[ti]++;
				// ��������� ��������� �������� ������� ������ ��� ���������� ������
				if(!IsReadThreadTerminated[ti] && !ReadThreadErrorNumber[ti])
				{
					EnterCriticalSection(&cs);
						gotoxy(XCoordCounter, YCoordCounter + 1*ti);
						printf("  Counter[%2u]: %8u", ti+0x1, Counter[ti]);
					LeaveCriticalSection(&cs);
				}

				// ���� �� �����-������ ������ � ������ ������� ������
				if(IsReadThreadTerminated[ti]) break;
				else if(ReadThreadErrorNumber[ti]) break;
				else Sleep(20);
			}
		}
		else ReadThreadErrorNumber[ti] = 0x4;
	}
	else ReadThreadErrorNumber[ti] = 0x6;

ReadThreadFinish:
	// ��������� ���� ������
	if(!pModules[ti]->STOP_ADC()) ReadThreadErrorNumber[ti] = 0x6;
	// ������, ���� �����, ����������� ������
	if(!CancelIo(pModules[ti]->GetModuleHandle())) ReadThreadErrorNumber[ti] = 0x7;
	// ��������� ��� �������������� �������
	for(WORD i = 0x0; i < 0x2; i++) CloseHandle(IoReq[i].Overlapped->hEvent);
	// ��������� ��������� �������� ������� ������ ��� ������� ������
	if(ReadThreadErrorNumber[ti])
	{
		EnterCriticalSection(&cs);
			gotoxy(XCoordCounter, YCoordCounter + 1*ti);
			printf("  Counter[%2u]: Thread Error!", ti+0x1, Counter[ti]);
		LeaveCriticalSection(&cs);
	}
	// ������ �������� ����� ����� �� ������
	return 0x0;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL WaitingForIoRequestCompleted(OVERLAPPED *ReadOv, DWORD ThreadTid)
{
	// ��� ���������� ���������� �������
	while(!IsReadThreadTerminated[ThreadTid])
	{
		if(HasOverlappedIoCompleted(ReadOv)) break;
		else if(IsReadThreadTerminated[ThreadTid]) break;
		else Sleep(20);
	}
	return TRUE;
}

//------------------------------------------------------------------------
// ����� ��������� �� ������
//------------------------------------------------------------------------
void AbortProgram(char *ErrorString, bool AbortionFlag)
{
	WORD i;

	// ��������� �� �����
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		if(ReadThreadHandle[i] != NULL)
		{
			// ������� ������ �����
			IsReadThreadTerminated[i] = true;
			// ��� ��������� ������ ������� ������
			WaitForSingleObject(ReadThreadHandle[i], INFINITE);
			// ��������� ������������� ������� ������
			CloseHandle(ReadThreadHandle[i]);
		}
		// ��������� ������ ������
		if(Buffer[i]) { delete[] Buffer[i]; Buffer[i] = NULL; }
		// ��������� ������
		if(pModules[i]) { pModules[i]->ReleaseLInstance(); pModules[i] = NULL; }
	}

	// ��������� ����������� ������
	DeleteCriticalSection(&cs);

	// �������� ��������� �������
	if(ModulesQuantity) gotoxy(XCoordCounter, YCoordCounter + ModulesQuantity + 0x2);
	else printf("\n\n");

	// ������� ����� ���������
	if(ErrorString) printf(ErrorString);

	// ���� ����� - �������� ��������� ���������
	if(AbortionFlag) exit(0x1);
	else return;
}

