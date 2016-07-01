//******************************************************************************
// ������ E-154.
// ���������� ��������� � �������� ����������� ������������ ����� ����� ��������
//	� ��� � ������� ������������ ������� ADC_KADR()
// ���� �������������� � ������ ������� ������� ���
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

#define CHANNELS_QUANTITY			(0x4)

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

// ����� ������
SHORT AdcBuffer[CHANNELS_QUANTITY];
// ������ �������� ��������� ����������
const WORD InputRangeIndex = ADC_INPUT_RANGE_5000mV_E154;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
   double volts_array[16];
   DWORD x, y;

	// �������� ����� ��������
	clrscr();

	printf(" Module E-154   Console example for AdcKadr Stream \n");

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

	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	// ��������, ��� ��� 'E-154'
	if(strcmp(ModuleName, "E154")) AbortProgram(" The module is not 'E-154'\n");

	// ��������� �������� �������� ������ ���� USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	// ������ ��������� �������� ������ ���� USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// ������� ���������� �� ���� ������
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");

	// ������� ������� ��������� ������ ���
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");
	// ��������� �������� ��������� ������ ���
	ap.ClkSource = INT_ADC_CLOCK_E154;				// ���������� ������ ���
	ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// ��� ���������� �������� �������� ���
	ap.InputMode = NO_SYNC_E154;						// ��� ������������� ����� ������
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 		// ������ �������� ������
	// ��������� ����������� �������
	for(i = 0x0; i < ap.ChannelsQuantity; i++)
		ap.ControlTable[i] = (WORD)(i | (InputRangeIndex << 0x6));
	ap.AdcRate = 100.0;									// ������� ������ ��� � ���
	ap.InterKadrDelay = 0.0;							// ����������� �������� � ��
	// ��������� ��������� ��������� ������ ��� � ������
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");

	// ��������� ��������� ����� ������ ������ �� ������ ��������
	printf(" \n");
	printf(" Module E-154 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("   Adc parameters:\n");
	printf("     Input Range  = %6.2f Volt\n", ADC_INPUT_RANGES_E154[InputRangeIndex]);
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.3f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.4f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.3f kHz\n", ap.KadrRate);

	printf("\n Press any key if you want to terminate this program...\n");
	// ���� ������������� ���������� ������� ADC_KADR �
	// ����������� ���������� ������ �� ������ ������
 //	printf("\n");
	printf(" ADC Channel:         1         2         3         4\n");
   x=wherex();
   y=wherey();
	while(!kbhit())
	{
      // ������� ������ � ������ ���
		if(!pModule->ADC_KADR(AdcBuffer)) AbortProgram("\n\n  ADC_KADR() --> Bad\n");

      // ��������� �� ������ ���� ���
  	   gotoxy(x,y);
//	   gotoxy(1,32);
		printf(" Kadr Data (code):  %5d     %5d     %5d     %5d\n", AdcBuffer[0], AdcBuffer[1], AdcBuffer[2], AdcBuffer[3]);

      // ��������� �� ������ �������� �� ������� ���, ������������ � ������ � �������������� ������������� �������������
      if(!pModule->ProcessArray(AdcBuffer, volts_array, CHANNELS_QUANTITY, TRUE, TRUE)) AbortProgram("\n\n  ProcessArray() --> Bad\n");
  	   gotoxy(x,y+1);
//	   gotoxy(1,33);
 //		printf(" Kadr Data (volts):%7.3f   %7.3f   %7.3f    %7.3f", volts_array[0], volts_array[1], volts_array[2], volts_array[3]);
		Sleep(20);
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

