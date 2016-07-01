//******************************************************************************
// ������ E-154.
// ���������� ��������� � �������� ����������� ������������ ����� ������� � ���
//	� ������� ������������ ������� ADC_SAMPLE()
// ���� �������������� � ������ ���� ������� ���
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

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

// ������� ���
SHORT AdcSample1, AdcSample2;
// ������ �������� ��������� ����������
//const WORD InputRangeIndex = ADC_INPUT_RANGE_10000mV_E154;
const WORD InputRangeIndex = ADC_INPUT_RANGE_5000mV_E154;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
   double AdcVolts[2];
   DWORD x, y;

	// �������� ����� ��������
	clrscr();

	printf(" *********************************\n");
	printf(" Module E-154                     \n");
	printf(" Console example AdcSample stream \n");
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
	// ��������, ��� ��� 'E14-154'
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

	// ��������� ��������� ������ �� ������ ��������
	printf(" \n\n");
	printf(" Module E-154 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("   Adc parameters:\n");
	printf("     Input Range  = %6.2f Volt\n", ADC_INPUT_RANGES_E154[InputRangeIndex]);


	printf("\n Press any key if you want to terminate this program...\n");
	// ���� ������������� ���������� ������� ADC_SAMPLE �
	// ����������� ���������� ������ �� ������ ������
	printf("\n\n");
	printf(" ADC Channel:                 1        2                     1        2\n");
   x=wherex();
   y=wherey();

	while(!kbhit())
	{
		if(!pModule->ADC_SAMPLE(&AdcSample1, (WORD)(0x00  | (InputRangeIndex << 6)))) { printf("\n\n  ADC_SAMPLE(, 0) --> Bad\n");	break; }
    		else if(!pModule->ADC_SAMPLE(&AdcSample2, (WORD)(0x01  | (InputRangeIndex << 6)))) { printf("\n\n  ADC_SAMPLE(, 1) --> Bad\n");	break; }

      // ����������� ���� ��� � ������ ��������� ������������� �����������
      if(!pModule->ProcessOnePoint(AdcSample1, &AdcVolts[0], (WORD)(0x00  | (InputRangeIndex << 6)), TRUE, TRUE))
       { printf("\n\n  PreocessOnePoint() --> Bad\n");	break; }
      if(!pModule->ProcessOnePoint(AdcSample2, &AdcVolts[1], (WORD)(0x01  | (InputRangeIndex << 6)), TRUE, TRUE))
      	{ printf("\n\n  ProcessOtPoint() --> Bad\n");	break; }

  	   gotoxy(x,y);
		printf(" AdcSample Data (ADC code): %5d   %5d;  (ADC volts): %7.3f  %7.3f\r", AdcSample1, AdcSample2,  AdcVolts[0], AdcVolts[1]);
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

