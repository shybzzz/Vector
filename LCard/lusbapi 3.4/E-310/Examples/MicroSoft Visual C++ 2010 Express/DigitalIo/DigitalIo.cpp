//******************************************************************************
// ������ E-310.
// ���������� ��������� � �������� ����������� ������ � ��������� �������
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
// �������� ������
char ModuleName[16];
// �������� ������ ���� USB
BYTE UsbSpeed;
// ��������� � ������ ����������� � ������
MODULE_DESCRIPTION_E310 ModuleDescription;

// ����� �������� �����
WORD TtlLinesPattern;
// ��������� �������� �����
WORD TtlOut, TtlIn;

//------------------------------------------------------------------------
// �������� ���������
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// �������� ����� ��������
	system("cls");

	printf(" ***************************************\n");
	printf(" Module E-310									\n");
	printf(" Digital IO console example					\n");
	printf(" ***************************************\n\n");

	// �������� ������ ������������ ���������� Lusbapi.dll
	if((DllVersion = GetDllVersion()) != CURRENT_VERSION_LUSBAPI)
	{
		char String[128];
		sprintf_s(String, " Lusbapi.dll Version Error!!!\n   Current: %1u.%1u. Required: %1u.%1u",
											DllVersion >> 0x10, DllVersion & 0xFFFF,
											CURRENT_VERSION_LUSBAPI >> 0x10, CURRENT_VERSION_LUSBAPI & 0xFFFF);

		AbortProgram(String);
	}
	else printf(" Lusbapi.dll Version --> OK\n");

	// ��������� �������� ��������� �� ���������
	pModule = static_cast<ILE310 *>(CreateLInstance("e310"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// ��������� ���������� ������ E-310 � ������ WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// ���-������ ����������?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E-310 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

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
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { printf(" GET_MODULE_DESCRIPTION() --> Bad\n"); exit(1); }
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// ��������� ����� �������� �����:
	// ������ 4 ����� - ��������
	// ������ 4 ����� - �������
	TtlLinesPattern = 0x000F;
	// ���������� �������� �����
	if(!pModule->CONFIG_TTL_LINES(TtlLinesPattern)) { printf(" CONFIG_TTL_LINES() --> Bad\n"); exit(1); }
	else printf(" CONFIG_TTL_LINES() --> OK\n");

	// ��������� ��������� ����� ������ ������ �� ������ ��������
	printf(" \n");
	printf(" Module E-310 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("   Digital lines parameters:\n");
	printf("     Outputs: 4\n");
	printf("     Inputs : 4\n");

	// ���� ������������� ���������� ������� TTL_OUT() � TTL_IN()
	printf("\n Press any key if you want to terminate this program...\n");
	// ����������������� ��������� �������� �������� �����
	TtlOut = 0x0;
	printf("\n\n");
	printf("                            In       Out\n");
	while(!_kbhit())
	{
		// ������� ������� ��������� �������� �������� �����
		if(!pModule->TTL_OUT(&TtlOut)) { printf("\n\n  TTL_OUT() --> Bad\n");	break; }
		// ������ ������ ��������� ������� �������� �����
		else if(!pModule->TTL_IN(&TtlIn)) { printf("\n\n  TTL_IN() --> Bad\n");	break; }
		// �������������� ��������� �������� �������� �����
		TtlOut++; TtlOut &= TtlLinesPattern;
		// ����������� �� �������
		printf(" Digital Lines States:     0x%02X     0x%02X\r", TtlIn, TtlOut);
		// ����������
		Sleep(300);
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
	if(_kbhit()) { while(_kbhit()) _getch(); }

	// ���� ����� - �������� ��������� ���������
	if(AbortionFlag) exit(0x1);
	// ��� �������� ������� �� �������
	else return;
}

