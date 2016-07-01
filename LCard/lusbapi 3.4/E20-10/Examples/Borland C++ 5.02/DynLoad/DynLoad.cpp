//******************************************************************************
// ������ E20-10.
// ���������� ��������� � �������� ������������ �������� ���������� Lusbapi.dll
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "LoadDll.h"				// ������������ �������� ����������
#include "Lusbapi.h"				// �������� ������

#define CHANNELS_QUANTITY			(0x4)

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// ��������� �� ����� ������������ �������� DLL
TLoadDll *pLoadDll;

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
// ������� ������ ��� � ���
const double AdcRate = 10000.0;
// ������ �������� ��������� ����������
const WORD InputRangeIndex = ADC_INPUT_RANGE_3000mV_E2010;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i, j;

	// �������� ����� ��������
	clrscr();

	printf(" ******************************************\n");
	printf(" Module E20-10                            \n");
	printf(" Console example for DLL dynamic loading 	\n");
	printf(" ******************************************\n");

	// ������������ �������� ���������� "Lusbapi.dll"
	pLoadDll = new TLoadDll("Lusbapi.dll");
	if(!pLoadDll) AbortProgram(" Can't alloc 'TLoadDll' object!!!\n");
	// �������� ������ �� �� ��������� ����������?
	if(!pLoadDll->GetDllHinstance()) AbortProgram(" 'Lusbapi.dll' Dynamic Loading --> Bad\n");
	else printf(" 'Lusbapi.dll' Dynamic Loading --> OK\n");

	// ����� ������� ��������� ������ ����������
	pGetDllVersion GetDllVersion = (pGetDllVersion)pLoadDll->CallGetDllVersion();
	if(!GetDllVersion) AbortProgram(" Address of GetDllVersion() --> Bad\n");
	else printf(" Address of GetDllVersion() --> OK\n");

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

	// ����� ������� ��������� ���������� ������
	pCreateInstance CreateLInstance = (pCreateInstance)pLoadDll->CallCreateLInstance();
	if(!CreateLInstance) AbortProgram(" Address of CreateLInstance() --> Bad!!!\n");
	else printf(" Address of CreateLInstance() --> OK\n");

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

	// ��� ��� ���� ������ �� ���������������� ������� ������� DLL ����������
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
	ap.SynchroPars.SynhroSource = INT_ADC_CLOCK_E2010;			// ���������� �������� �������� ���
	ap.SynchroPars.StartDelay = 0x0;									// �������� ������ ����� ������ � ������ �������� (��� Rev.B � ����)
	ap.SynchroPars.StopAfterNKadrs = 0x0;							// ������� ����� ������ ����� �������� ���-�� ������ �������� (��� Rev.B � ����)
	ap.SynchroPars.SynchroAdMode = NO_ANALOG_SYNCHRO_E2010;	// ��� ���������� ������������� (��� Rev.B � ����)
//	ap.SynchroPars.SynchroAdMode = ANALOG_SYNCHRO_ON_HIGH_LEVEL_E2010;
	ap.SynchroPars.SynchroAdChannel = 0x0;							// ����� ���������� ������������� (��� Rev.B � ����)
	ap.SynchroPars.SynchroAdPorog = 0;								// ����� ���������� ������������� � ����� ��� (��� Rev.B � ����)
	ap.SynchroPars.IsBlockDataMarkerEnabled = 0x0;				// ������������ ������ ����� ������ (������, ��������, ��� ���������� ������������� ����� �� ������) (��� Rev.B � ����)
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 		// ������ �������� ������
	// ��������� ����������� �������
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i);
	// ������� ����� ����� ������������� � ����������� �� �������� USB
	ap.AdcRate = AdcRate;													// ������� ������ ��� � ���
	if(UsbSpeed == USB11_LUSBAPI) ap.InterKadrDelay = 0.01;		// ����������� �������� � ��
	else ap.InterKadrDelay = 0.0;
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		ap.InputRange[i] = ADC_INPUT_RANGE_3000mV_E2010;  	// ������� ��������� 3�
		ap.InputSwitch[i] = ADC_INPUT_SIGNAL_E2010;			// �������� ����� - ������
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

	// ��������� ���������� �� ������
	printf(" \n");
	printf(" Module E20-10 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("     PLD    Version     is %s (%s)\n", ModuleDescription.Pld.Version.Version, ModuleDescription.Pld.Version.Date);
	printf("   Adc parameters:\n");
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.2f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.5f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.2f kHz\n", ap.KadrRate);

	// .....
	// ����� ����� �������������� ���� �������� ������ � �������,
	// ��������, ���� ���� ����� ������
	// .....

	// ��� ���������� ��������� - ��������� ��������� ������
	printf("\n");
	AbortProgram(" The program was completed successfully!!!\n", false);
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
	// ��������� ����������
	if(pLoadDll) { delete pLoadDll; pLoadDll = NULL; }

	// ������� ����� ���������
	if(ErrorString) printf(ErrorString);

	// ��������� ������� ����������
	if(kbhit()) { while(kbhit()) getch(); }

	// ���� ����� - �������� ��������� ���������
	if(AbortionFlag) exit(0x1);
	// ��� �������� ������� �� �������
	else return;
}

