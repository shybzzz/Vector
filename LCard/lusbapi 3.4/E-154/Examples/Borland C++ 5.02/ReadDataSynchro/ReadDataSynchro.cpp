//******************************************************************************
// ������ E-154.
// ���������� ��������� � ������������ ����������� �������� ������� �� ���� ������
// � ����������� ������� ���������� ������ �� ����.
// ���� �������������� � ������ ������ ������� ��� �� ������� 100 ���.
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

#define CHANNELS_QUANTITY			(0x4)

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// ������������� �����
HANDLE hFile;
DWORD FileBytesWritten;

// ������ ����������
DWORD DllVersion;
// ��������� �� ��������� ������
ILE154 *pModule;
// ���������� ����������
HANDLE ModuleHandle;
// �������� ������
char ModuleName[7];
// �������� ������ ���� USB
BYTE UsbSpeed;
// ��������� � ������ ����������� � ������
MODULE_DESCRIPTION_E154 ModuleDescription;
// ��������� ���������� ������ ��� ������
ADC_PARS_E154 ap;
// ��������� � ����������� ������� �� ����/����� ������
IO_REQUEST_LUSBAPI IoReq;

// ���-�� ���������� ��������
DWORD DataStep = 256*1024;
// ����� ������
SHORT *ReadBuffer;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
//	WORD DacSample;

	// ���� ������ �� �������� ��� ����� ������
	ReadBuffer = NULL;
	// ���� �������� ����� ��� :(
	hFile = INVALID_HANDLE_VALUE;

	// �������� ����� ��������
	clrscr();

	printf(" ***************************************\n");
	printf(" Module E-154                  			\n");
	printf(" Console example for ADC Synchro Stream \n");
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

	// ��������� ���������� ������ E-154 � ������ WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// ���-������ ����������?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E-154 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// ��������� ��������� ���������� ����������
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// ��������, ��� ��� 'E-154'
	if(strcmp(ModuleName, "E154")) AbortProgram(" The module is not 'E-154'\n");
	else printf(" The module is 'E-154'\n");

	// ��������� �������� �������� ������ ���� USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// ������ ��������� �������� ������ ���� USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// ������� ���������� �� ���� ������
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// ������� ������� ��������� ������ ���
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");
	// ��������� �������� ��������� ������ ���
	ap.ClkSource = INT_ADC_CLOCK_E154;							// ���������� ������ ���
	ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// ��� ���������� �������� �������� ���
	ap.InputMode = NO_SYNC_E154;		// ��� ������������� ����� ������
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 		// ���-�� �������� ������
	// ��������� ����������� �������
	for(i = 0x0; i < ap.ChannelsQuantity; i++)
		ap.ControlTable[i] = (WORD)(i | (ADC_INPUT_RANGE_5000mV_E154 << 0x6));
	ap.AdcRate = 100.0;							// ������� ������ ��� � ���
	ap.InterKadrDelay = 0.0;					// ����������� �������� � ��
	// ��������� ��������� ��������� ������ ��� � ������
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// ������� ������ ��� �����
	ReadBuffer = new SHORT[DataStep];
	if(!ReadBuffer) AbortProgram(" Can not allocate memory\n");

	// ������� ���� ��� ������ ���������� � ������ ������
	hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_WRITE_THROUGH, NULL);
	if(hFile == INVALID_HANDLE_VALUE) AbortProgram("\n Can't create file 'Test.dat'!\n");

	// ��������� ��������� ����� ������ ������ �� ������ ��������
	printf(" \n");
	printf(" Module E-154 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("     %s Mode\n", UsbSpeed ? "HIGH-SPEED (480 Mbit/s)" : "FULL-SPEED (12 Mbit/s)");
	printf("   Adc parameters:\n");
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.3f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.4f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.3f kHz\n\n", ap.KadrRate);

	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if(!pModule->STOP_ADC()) AbortProgram(" STOP_ADC() --> Bad\n");
	else printf(" STOP_ADC() --> OK\n");

	// ��������� ��������� IoReq
	IoReq.Buffer = ReadBuffer;					// ����� ������
	IoReq.NumberOfWordsToPass = DataStep;	// ���-�� ���������� ������
	IoReq.NumberOfWordsPassed = 0x0;
	IoReq.Overlapped = NULL;					// ���������� ������� �������
	IoReq.TimeOut = DataStep/ap.AdcRate + 1000;	// ������� ����������� ����� ������

	// �������� ���
	if(!pModule->START_ADC()) AbortProgram(" START_ADC() --> Bad\n");
	else printf(" START_ADC() --> OK\n");

	// ���������� ������ ������ �� ���� ������
	if(!pModule->ReadData(&IoReq)) AbortProgram(" ReadData() --> Bad\n");
	else
	{
		printf(" ReadData() --> OK\n");

		// ������ ������� � ���� ���������� ������
		DWORD FileBytesWritten = 0x0;
		if(!WriteFile(	hFile,								// handle to file to write to
	    					IoReq.Buffer,			 			// pointer to data to write to file
							2*IoReq.NumberOfWordsToPass,	// number of bytes to write
    						&FileBytesWritten,				// pointer to number of bytes written
					   	NULL			  						// pointer to structure needed for overlapped I/O
					   ))  AbortProgram(" WriteFile(Test.dat) --> Bad\n");
		else printf(" WriteFile(Test.dat) --> Ok\n");
	}

	// ��������� ������ ���
	if(!pModule->STOP_ADC()) AbortProgram(" STOP_ADC() --> Bad\n");
	else printf(" STOP_ADC() --> OK\n");

	// ������ �������� ������������� ����������� ������ �� ���� ������
	if(!CancelIo(ModuleHandle)) AbortProgram(" CancelIo() --> Bad\n");
	else printf(" CancelIo() --> OK\n");

	// �� ������!!!
	AbortProgram("\n The program was completed successfully!!!\n", false);
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
	if(ReadBuffer) { delete[] ReadBuffer; ReadBuffer = NULL; }
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

