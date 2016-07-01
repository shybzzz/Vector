//******************************************************************************
// ������ E-310.
// ���������� ��������� � �������� ����������� ������������ ����� ������� � ���
//	� ������� ������������ ������� GET_ADC_DATA()
// ���� �������������� � ������� ������� ���
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
// ��������� ���������� ������ ��� ������
ADC_PARS_E310 AdcPars;
// ��������� �������� ���
ADC_DATA_E310 AdcData;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
   DWORD x, y;

	// �������� ����� ��������
	clrscr();

	printf(" *********************************\n");
	printf(" Module E-310.							\n");
	printf(" ADC console example.					\n");
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

	// ������� ������� ��������� ������ ���
	if(!pModule->GET_ADC_PARS(&AdcPars)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");

	// ��������� �������� ��������� ������ ���
	AdcPars.AdcStartSource = INTERNAL_ADC_START_E310;		// ���������� ������ ���
	AdcPars.ChannelsMask =	(0x1 << ADC_CHANNEL_1_E310) | (0x1 << ADC_CHANNEL_2_E310) |
									(0x1 << ADC_CHANNEL_3_E310) | (0x1 << ADC_CHANNEL_4_E310);

	// ��������� ��������� ��������� ������ ��� � ������
	if(!pModule->SET_ADC_PARS(&AdcPars)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// ��������� ��������� ������ �� ������ ��������
	printf(" \n\n");
	printf(" Module E-310 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("   Adc parameters:\n");
	printf("     ADC start source is %s\n", AdcPars.AdcStartSource ? "external" : "internal");
	printf("     Input Range = %6.2f Volt\n", AdcPars.InputRange);

	printf("\n Press any key if you want to terminate this program...\n");
	// ���� ������������� ���������� ������� GET_ADC_DATA �
	// ����������� ���������� ������ �� ������ ������
//	printf("\n\n");
	printf(" ADC Channel:                1         2         3         4\n");
	x=wherex();
	y=wherey();
	while(!kbhit())
	{
		// ��������� �������� ������ � ���
		if(!pModule->GET_ADC_DATA(&AdcData)) { printf("\n\n  GET_ADC_DATA() --> Bad\n");	break; }

		// ��������� �� ������ ���� ���
		gotoxy(x, y);
		printf(" Adc Data (ADC code):");
		for(i = 0x0; i < ADC_CHANNEL_QUANTITY_E310; i++)
			printf("%10d", AdcData.DataInCode[i]);

		// ��������� �� ������ ������
		gotoxy(x, y + 0x1);
		printf(" Adc Data (V):       ");
		for(i = 0x0; i < ADC_CHANNEL_QUANTITY_E310; i++)
			printf("%10.1f", AdcData.DataInV[i]);

		Sleep(50);
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

