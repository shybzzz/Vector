//******************************************************************************
// ������ E20-10.
// ���������� ��������� � ������������ ����������� �������� ������� �� ���� ������
// � ����������� ������� ���������� ������ �� ����.
// ���� �������������� � ������ ������ ������� ��� �� ������� 400 ���.
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
ILE2010 *pModule;
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
// ��������� � ����������� ������� �� ����/����� ������
IO_REQUEST_LUSBAPI IoReq;

// ���-�� ���������� �������� (������� 32) ��� �. ReadData()
DWORD DataStep = 1024*1024;
// ������� ����� � ���
const double AdcRate = 10000.0;
// ����� ������
SHORT * AdcBuffer;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i, j;
//	WORD DacSample;

	// ���� ������ �� �������� ��� ����� ������
	AdcBuffer = NULL;
	// ���� �������� ����� ��� :(
	hFile = INVALID_HANDLE_VALUE;

	// �������� ����� ��������
	clrscr();

	printf(" ***************************************\n");
	printf(" Module E20-10                 			\n");
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
	pModule = static_cast<ILE2010 *>(CreateLInstance("e2010"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// ��������� ���������� ������ E20-10 � ������ MAX_VIRTUAL_SLOtS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// ���-������ ����������?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E20-10 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// ��������� ��������� ���������� ����������
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// ��������, ��� ��� 'E20-10'
	if(strcmp(ModuleName, "E20-10")) AbortProgram(" The module is not 'E20-10'\n");
	else printf(" The module is 'E20-10'\n");

	// ��������� �������� �������� ������ ���� USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// ������ ��������� �������� ������ ���� USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// ��� ������ �� ���������������� ������� ������� DLL ����������
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
	if(ModuleDescription.Module.Revision == REVISIONS_E2010[REVISION_A_E2010])
		ap.IsAdcCorrectionEnabled = FALSE;		// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.A)
	else
		ap.IsAdcCorrectionEnabled = TRUE;		// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.B � ����)
	ap.SynchroPars.StartSource = INT_ADC_START_E2010;			// ���������� ����� ����� � ���
//	ap.SynchroPars.StartSource = EXT_ADC_START_ON_RISING_EDGE_E2010;		// ������� ����� ����� � ���
	ap.SynchroPars.SynhroSource = INT_ADC_CLOCK_E2010;			// ���������� �������� �������� ���
	ap.SynchroPars.StartDelay = 0x0;									// �������� ������ ����� ������ � ������ �������� (��� Rev.B � ����)
	ap.SynchroPars.StopAfterNKadrs = 0x0;							// ������� ����� ������ ����� �������� ���-�� ������ �������� (��� Rev.B � ����)
	ap.SynchroPars.SynchroAdMode = NO_ANALOG_SYNCHRO_E2010;	// ��� ���������� ������������� (��� Rev.B � ����)
//	ap.SynchroPars.SynchroAdMode = ANALOG_SYNCHRO_ON_HIGH_LEVEL_E2010;
	ap.SynchroPars.SynchroAdChannel = 0x0;							// ����� ���������� ������������� (��� Rev.B � ����)
	ap.SynchroPars.SynchroAdPorog = 0;								// ����� ���������� ������������� � ����� ��� (��� Rev.B � ����)
	ap.SynchroPars.IsBlockDataMarkerEnabled = 0x0;				// ������������ ������ ����� ������ (������, ��������, ��� ���������� ������������� ����� �� ������) (��� Rev.B � ����)
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 						// ������ �������� ������
	// ��������� ����������� �������
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i);
	// ������� ����� ����� ������������� � ����������� �� �������� USB
	ap.AdcRate = AdcRate;	  												// ������� ������ ��� � ���
	if(UsbSpeed == USB11_LUSBAPI) ap.InterKadrDelay = 0.01;		// ����������� �������� � ��
	else ap.InterKadrDelay = 0.0;											
	// �������� �������� ���������� � ��� ����������� �������� �������   
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
/*		if(i)
		{
			ap.InputRange[i] = ADC_INPUT_RANGE_3000mV_E2010;  	// ������� ��������� 3�
			ap.InputSwitch[i] = ADC_INPUT_ZERO_E2010;				// �������� ����� - �����
		}
		else
		{*/
			ap.InputRange[i] = ADC_INPUT_RANGE_3000mV_E2010;  	// ������� ��������� 3�
			ap.InputSwitch[i] = ADC_INPUT_SIGNAL_E2010;			// �������� ����� - ������
//		}
	}
	// ������� � ��������� ���������� ������ ��� ���������������� ������������ ���
	for(i = 0x0; i < ADC_INPUT_RANGES_QUANTITY_E2010; i++)
		for(j = 0x0; j  < ADC_CHANNELS_QUANTITY_E2010; j++)
		{
			// ������������� ��������
			ap.AdcOffsetCoefs[i][j] = ModuleDescription.Adc.OffsetCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
			// ������������� ��������
			ap.AdcScaleCoefs[i][j] = ModuleDescription.Adc.ScaleCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
		}
	// ��������� ��������� ��������� ������ ��� � ������
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// ������� ������ ��� �����
	AdcBuffer = new SHORT[DataStep];
	if(!AdcBuffer) AbortProgram(" Can not allocate memory\n");

	// ������� ���� ��� ������ ���������� � ������ ������
	hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_WRITE_THROUGH, NULL);
	if(hFile == INVALID_HANDLE_VALUE) AbortProgram("\n Can't create file 'Test.dat'!\n");

	// ��������� ��������� ����� ������ ������ �� ������ ��������
	printf(" \n");
	printf(" Module E20-10 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("     PLD   Version      is %s (%s)\n", ModuleDescription.Pld.Version.Version, ModuleDescription.Pld.Version.Date);
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
	IoReq.Buffer = AdcBuffer;					// ����� ������
	IoReq.NumberOfWordsToPass = DataStep;	// ���-�� ���������� ������
	IoReq.NumberOfWordsPassed = 0x0;
	IoReq.Overlapped = NULL;					// ���������� ������� �������
	IoReq.TimeOut = DataStep/ap.KadrRate + 1000;	// ������� ����������� ����� ������ 		

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
	if(AdcBuffer) { delete[] AdcBuffer; AdcBuffer = NULL; }
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

