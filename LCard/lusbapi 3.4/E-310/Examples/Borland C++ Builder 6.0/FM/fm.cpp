//******************************************************************************
// ������ E-310.
// ���������� ��������� � �������� ����������� ������ � ������������
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// ������ ����������
DWORD DllVersion;
// ��������� �� ��������� ������
ILE310 *pModule;
// ���������� ����������
HANDLE ModuleHandle;
// �������� ������
char ModuleName[16];
// �������� ������ ���� USB
BYTE UsbSpeed;
// ��������� � ������ ����������� � ������
MODULE_DESCRIPTION_E310 ModuleDescription;
// ��������� ���������� ������ ������������
FM_PARS_E310 FmPars;
// ��������� ������� ������������
FM_SAMPLE_E310 FmSample;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// �������� ����� ��������
	clrscr();

	printf(" *********************************\n");
	printf(" Module E-310.							\n");
	printf(" FM console example.					\n");
	printf(" *********************************\n\n");

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
	pModule = static_cast<ILE310 *>(CreateLInstance("e310"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// ��������� ���������� ������ E-310 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// ���-������ ����������?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E-310 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// ��������� ��������� ���������� ����������
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// ��������, ��� ��� 'E-310'
	if(strcmp(ModuleName, "E-310")) AbortProgram(" The module is not 'E-310'\n");
	else printf(" The module is 'E-310'\n");

	// ��������� �������� �������� ������ ���� USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// ������ ��������� �������� ������ ���� USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// ������� ���������� �� ���� ������
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// -=== ��������� ����������� ===-
	// �������� ������� ��������� �����������
	if(!pModule->GET_FM_PARS(&FmPars)) AbortProgram(" GET_FM_PARS() --> Bad\n");
	else printf(" GET_FM_PARS() --> OK\n");

	FmPars.Mode				= PERIOD_MODE_FM_E310;		// �����: ��������� ������� �������� �������
	FmPars.InputDivider	= INPUT_DIVIDER_1_FM_E310;	// �������� ������� 1:1 �������� �������
//	FmPars.InputDivider	= INPUT_DIVIDER_8_FM_E310;	// �������� ������� 1:8 �������� �������
	FmPars.BaseClockRateDivIndex = BASE_CLOCK_DIV_02_INDEX_FM_E310;	// �������� ������� �������
//	FmPars.BaseClockRateDivIndex = BASE_CLOCK_DIV_08_INDEX_FM_E310;	// �������� ������� �������
	FmPars.Offset			= 0.0;//2.5;					// �������� ������� ����������� � �

	// ������������� ��������� �����������
	if(!pModule->SET_FM_PARS(&FmPars)) AbortProgram(" SET_FM_PARS() --> Bad\n");
	else printf(" SET_FM_PARS() --> OK\n");
	// -=============================-

	// ��������� ����������
	if(!pModule->START_FM()) AbortProgram(" START_FM() --> Bad\n");
	else printf(" START_FM() --> OK\n");

	// ��������� ��������� ������ �� ������ ��������
	printf(" \n\n");
	printf(" Module E-310 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("   FM parameters:\n");
	printf("     Input Divider is %s\n",		FmPars.InputDivider ? "1:1" : "1:8");
	printf("     Clock Rate  = %10.4f kHz\n",	FmPars.ClockRate/1000.0);

	printf("\n Press any key if you want to terminate this program...\n");
	// ���� ������������� ���������� ������� GET_ADC_DATA �
	// ����������� ���������� ������ �� ������ ������
	printf("\n\n");
	while(!kbhit())
	{
		// ������� �������� ������ �����������
		if(!pModule->FM_SAMPLE(&FmSample)) { printf("\n\n  FM_SAMPLE() --> Bad\n");	break; }
		// ��������� �� ������ ������ �����������
		if(FmSample.IsActual)
			printf(" FM Sample: %10.3f kHz\r", FmSample.Frequency);
		else
			printf(" FM Sample: --------------\r");
		// ����������
		Sleep(200);
	}

	// ��������� ��������� ������
	printf("\n\n");
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
		// ��������� ����������
		if(!pModule->STOP_FM()) printf(" STOP_FM() --> Bad\n");
		else printf(" STOP_FM() --> OK\n");
		// ��������� ��������� ������
		if(!pModule->ReleaseLInstance()) printf(" ReleaseLInstance() --> Bad\n");
		else printf(" ReleaseLInstance() --> OK\n");
		// ������� ��������� �� ��������� ������
		pModule = NULL;
	}

	// ������� ����� ���������
	if(ErrorString) printf(ErrorString);

	// ��������� ������� ����������
	if(kbhit()) { while(kbhit()) getch(); }

	// ���� ����� - �������� ��������� ���������
	if(AbortionFlag) exit(0x1);
	// ��� �������� ������� �� �������
	else return;
}
