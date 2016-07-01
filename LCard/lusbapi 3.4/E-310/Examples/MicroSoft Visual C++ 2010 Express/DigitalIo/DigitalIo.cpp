//******************************************************************************
// Модуль E-310.
// Консольная программа с примером организации работы с цифровыми линиями
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// версия библиотеки
DWORD DllVersion;
// указатель на интерфейс модуля
ILE310 *pModule;
// название модуля
char ModuleName[16];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E310 ModuleDescription;

// маска цифровых линий
WORD TtlLinesPattern;
// состояния цифровых линий
WORD TtlOut, TtlIn;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// зачистим экран монитора
	system("cls");

	printf(" ***************************************\n");
	printf(" Module E-310									\n");
	printf(" Digital IO console example					\n");
	printf(" ***************************************\n\n");

	// проверим версию используемой библиотеки Lusbapi.dll
	if((DllVersion = GetDllVersion()) != CURRENT_VERSION_LUSBAPI)
	{
		char String[128];
		sprintf_s(String, " Lusbapi.dll Version Error!!!\n   Current: %1u.%1u. Required: %1u.%1u",
											DllVersion >> 0x10, DllVersion & 0xFFFF,
											CURRENT_VERSION_LUSBAPI >> 0x10, CURRENT_VERSION_LUSBAPI & 0xFFFF);

		AbortProgram(String);
	}
	else printf(" Lusbapi.dll Version --> OK\n");

	// попробуем получить указатель на интерфейс
	pModule = static_cast<ILE310 *>(CreateLInstance("e310"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// попробуем обнаружить модуль E-310 в первых WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// что-нибудь обнаружили?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E-310 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// прочитаем название модуля в обнаруженном виртуальном слоте
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// проверим, что это 'E-310'
	if(strcmp(ModuleName, "E-310")) AbortProgram(" The module is not 'E-310'\n");
	else printf(" The module is 'E-310'\n");

	// попробуем получить скорость работы шины USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// теперь отобразим скорость работы шины USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// получим информацию из ППЗУ модуля
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { printf(" GET_MODULE_DESCRIPTION() --> Bad\n"); exit(1); }
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// формируем маску цифровых линий:
	// первые 4 линии - выходные
	// вторые 4 линии - входные
	TtlLinesPattern = 0x000F;
	// конфигурим цифровые линии
	if(!pModule->CONFIG_TTL_LINES(TtlLinesPattern)) { printf(" CONFIG_TTL_LINES() --> Bad\n"); exit(1); }
	else printf(" CONFIG_TTL_LINES() --> OK\n");

	// отобразим параметры сбора данных модуля на экране монитора
	printf(" \n");
	printf(" Module E-310 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("   Digital lines parameters:\n");
	printf("     Outputs: 4\n");
	printf("     Inputs : 4\n");

	// цикл перманентного выполнения функций TTL_OUT() и TTL_IN()
	printf("\n Press any key if you want to terminate this program...\n");
	// проинициализируем состояния выходных цифровых линий
	TtlOut = 0x0;
	printf("\n\n");
	printf("                            In       Out\n");
	while(!_kbhit())
	{
		// выводим текущие состояния выходных цифровых линий
		if(!pModule->TTL_OUT(&TtlOut)) { printf("\n\n  TTL_OUT() --> Bad\n");	break; }
		// теперь читаем состояния входных цифровых линий
		else if(!pModule->TTL_IN(&TtlIn)) { printf("\n\n  TTL_IN() --> Bad\n");	break; }
		// инкрементируем состояния выходных цифровых линий
		TtlOut++; TtlOut &= TtlLinesPattern;
		// отображение на дисплее
		printf(" Digital Lines States:     0x%02X     0x%02X\r", TtlIn, TtlOut);
		// задержечка
		Sleep(300);
	}

	// освободим интерфейс модуля
	printf("\n\n");
	AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------
// аварийное завершение программы
//------------------------------------------------------------------------
void AbortProgram(char *ErrorString, bool AbortionFlag)
{
	// подчищаем интерфейс модуля
	if(pModule)
	{
		// освободим интерфейс модуля
		if(!pModule->ReleaseLInstance()) printf(" ReleaseLInstance() --> Bad\n");
		else printf(" ReleaseLInstance() --> OK\n");
		// обнулим указатель на интерфейс модуля
		pModule = NULL;
	}

	// выводим текст сообщения
	if(ErrorString) printf(ErrorString);

	// прочистим очередь клавиатуры
	if(_kbhit()) { while(_kbhit()) _getch(); }

	// если нужно - аварийно завершаем программу
	if(AbortionFlag) exit(0x1);
	// или спокойно выходим из функции
	else return;
}

