#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "Lusbapi.h"

// ��������� ����� �� ���������
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

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

// ������� ��� ���
SHORT DacSample0, DacSample1;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// �������� ����� ��������
	clrscr();

	printf(" *************************************\n");
	printf(" Module E20-10                        \n");
	printf(" Console example for DAC_SAMPLE Stream\n");
	printf(" *************************************\n\n");

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

	// ��� ��� ���� ������ �� ���������������� ������� ������� DLL ����������
	if(!pModule->LOAD_MODULE()) AbortProgram(" LOAD_MODULE() --> Bad\n");
	else printf(" LOAD_MODULE() --> OK\n");

	// �������� �������� ������
 	if(!pModule->TEST_MODULE()) AbortProgram(" TEST_MODULE() --> Bad\n");
	else printf(" TEST_MODULE() --> OK\n");

	// ������� ���������� �� ���� ������
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// �������� ���� �� �� ������ ���
	if(ModuleDescription.Dac.Active == DAC_INACCESSIBLED_E2010) AbortProgram(" DAC is absent on this module E20-10!\n");

	// ������ ��������� ��������� ��� �� �������
	printf(" \n");
	printf(" Module E20-10 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("     PLD    Version     is %s (%s)\n", ModuleDescription.Pld.Version.Version, ModuleDescription.Pld.Version.Date);
	printf("\n  Now SINUS signal is on the first DAC channel and \n");
	printf("     -SINUS signal is on the second DAC channel.\n");
	printf("      (Press any key to terminate the program)\n");

	// ���� ����������� ������ ������� DAC_SAMPLE().
	// ��� ���� �� ������ ������ ��� ����� ��������� �����,
	// � �� ������ ������ ��� ����� ��������� -�����.
	// �.�. ������� �� ������� ��� ����� ��������� � ����������� ;)
	double CurrentTime = 0.0;
	for(DWORD i = 0 ; ; i++)
	{
		DacSample0 = (SHORT)(2000.0*sin(2.0*M_PI*2.5*CurrentTime));
		DacSample1 = (SHORT)(-DacSample0);
		CurrentTime += 1.0/125.0;

		// ���� ���� ������ �����-���� �������, �� ���� ������ �� �����
		if(kbhit()) break;
		// ������� ��������� ������ �� ������ ����� ���
		else if(!pModule->DAC_SAMPLE(&DacSample0, WORD(0x0))) { printf(" DAC_SAMPLE(0) --> Bad\n"); break; }
		// ������� ��������� ������ �� ������ ����� ���
		else if(!pModule->DAC_SAMPLE(&DacSample1, WORD(0x1))) { printf(" DAC_SAMPLE(1) --> Bad\n"); break; }

		// ������� ������ �������� � �������
		if(!(i%800))
		{
			static flag;

			flag ^= -0x1;
			if(flag) printf("                                             \r");
			else printf(" ............................................\r");
		}
	}

	printf("                                             \n");

	// �������� ������� �������� �� ������� ����� ������� ���
	DacSample0 = (SHORT)(0x0);
	if(!pModule->DAC_SAMPLE(&DacSample0, WORD(0x0))) printf(" DAC_SAMPLE(0) --> Bad\n");
	else if(!pModule->DAC_SAMPLE(&DacSample0, WORD(0x1))) printf(" DAC_SAMPLE(1) --> Bad\n");

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

