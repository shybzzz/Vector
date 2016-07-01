//******************************************************************************
// ������ E14-440.
// ���������� ��������� � ������������ ����������� �������� ������� �� ���� ������
// � ����������� ������� ���������� ������ �� ����.
// ���� �������������� � ������ ������ ������� ��� �� ������� 400 ���.
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// ������������� �����
HANDLE hFile;
DWORD FileBytesWritten;

// ������ ����������
DWORD DllVersion;
// ��������� �� ��������� ������
ILE440 *pModule;
// ���������� ����������
HANDLE ModuleHandle;
// �������� ������
char ModuleName[7];
// �������� ������ ���� USB
BYTE UsbSpeed;
// ��������� � ������ ����������� � ������
MODULE_DESCRIPTION_E440 ModuleDescription;
// ��������� ���������� ������ ��� ������
ADC_PARS_E440 ap;
// ��������� � ����������� ������� �� ����/����� ������
IO_REQUEST_LUSBAPI IoReq;

// ���-�� ���������� �������� (������� 32) ��� �. ReadData()
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
	printf(" Module E14-440                 			\n");
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
	pModule = static_cast<ILE440 *>(CreateLInstance("e440"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// ��������� ���������� ������ E14-440 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// ���-������ ����������?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E14-440 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// ��������� ��������� ���������� ����������
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// ��������, ��� ��� 'E14-440'
	if(strcmp(ModuleName, "E440")) AbortProgram(" The module is not 'E14-440'\n");
	else printf(" The module is 'E14-440'\n");

	// ��������� �������� �������� ������ ���� USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// ������ ��������� �������� ������ ���� USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// ��� LBIOS'� ������ �� ���������������� ������� ������� DLL ����������
	if(!pModule->LOAD_MODULE()) AbortProgram(" LOAD_MODULE() --> Bad\n");
	else printf(" LOAD_MODULE() --> OK\n");

	// �������� �������� ������
 	if(!pModule->TEST_MODULE()) AbortProgram(" TEST_MODULE() --> Bad\n");
	else printf(" TEST_MODULE() --> OK\n");

	// ������� ���������� �� ���� ������
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// ������� ������� ��������� ������ ���
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");

	// ��������� �������� ��������� ������ ���
	ap.IsCorrectionEnabled = true; 			// �������� ������������� ������ �� ������ �������� DSP
	ap.InputMode = NO_SYNC_E440;				// ������� ���� ������ ���� ������ ������������� �����
	ap.ChannelsQuantity = 0x4;					// ������ �������� ������
	// ��������� ����������� �������
	for(i = 0x0; i < ap.ChannelsQuantity; i++)
		ap.ControlTable[i] = (WORD)(i | (ADC_INPUT_RANGE_2500mV_E440 << 0x6));
	ap.AdcRate = 400.0;							// ������� ������ ��� � ���
	ap.InterKadrDelay = 0.0;					// ����������� �������� � ��
	ap.AdcFifoBaseAddress = 0x0;			  	// ������� ����� FIFO ������ ��� � DSP ������
	ap.AdcFifoLength = MAX_ADC_FIFO_SIZE_E440;	// ����� FIFO ������ ��� � DSP ������
	// ����� ������������ ��������� ������������� ������������, ������� ��������� � ���� ������
	for(i = 0x0; i < ADC_CALIBR_COEFS_QUANTITY_E440; i++)
	{
		ap.AdcOffsetCoefs[i] =  ModuleDescription.Adc.OffsetCalibration[i];
		ap.AdcScaleCoefs[i] =  ModuleDescription.Adc.ScaleCalibration[i];
	}

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
	printf(" Module E14-440 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("     LBIOS   Version    is %s (%s)\n", ModuleDescription.Dsp.Version.Version, ModuleDescription.Dsp.Version.Date);
	printf("     %s Mode\n", UsbSpeed ? "HIGH-SPEED (480 Mbit/s)" : "FULL-SPEED (12 Mbit/s)");
	printf("   Adc parameters:\n");
	printf("     Data Correction is %s\n", ap.IsCorrectionEnabled ? "enabled" : "disabled");
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

