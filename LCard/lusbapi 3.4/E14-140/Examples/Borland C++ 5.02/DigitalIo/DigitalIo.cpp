//******************************************************************************
// Модуль E14-140.
// Консольная программа с примером организации работы с цифровыми линиями
//	при помощи интерфейсных функций TTL_IN() и TTL_OUT()
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);
// функция потока ввода данных с АЦП
DWORD WINAPI ServiceReadThread(PVOID /*Context*/);
// функция вывода сообщений с ошибками
void ShowThreadErrorMessage(void);

// версия библиотеки
DWORD DllVersion;
// указатель на интерфейс модуля
ILE140 *pModule;
// название модуля
char ModuleName[7];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E140 ModuleDescription;

// состояния цифровых линий
WORD TtlIn, TtlOut;

// экранный счетчик-индикатор
DWORD Counter = 0x0, OldCounter = 0xFFFFFFFF;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// зачистим экран монитора
	clrscr();

	printf(" ***************************************\n");
	printf(" Module E14-140                         \n");
	printf(" Console example for Digital IO			\n");
	printf(" ***************************************\n\n");

	// проверим версию используемой библиотеки Lusbapi.dll
	if((DllVersion = GetDllVersion()) != CURRENT_VERSION_LUSBAPI)
	{
		char String[128];
		sprintf(String, " Lusbapi.dll Version Error!!!\n   Current: %1u.%1u. Required: %1u.%1u",
											DllVersion >> 0x10, DllVersion & 0xFFFF,
											CURRENT_VERSION_LUSBAPI >> 0x10, CURRENT_VERSION_LUSBAPI & 0xFFFF);

		AbortProgram(String);
	}
	else printf(" Lusbapi.dll Version --> OK\n");

	// попробуем получить указатель на интерфейс
	pModule = static_cast<ILE140 *>(CreateLInstance("e140"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// попробуем обнаружить модуль E14-140 в первых WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// что-нибудь обнаружили?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E14-140 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// прочитаем название модуля в обнаруженном виртуальном слоте
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// проверим, что это 'E14-140'
	if(strcmp(ModuleName, "E140")) AbortProgram(" The module is not 'E14-140'\n");
	else printf(" The module is 'E14-140'\n");

	// попробуем получить скорость работы шины USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// теперь отобразим скорость работы шины USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// получим информацию из ППЗУ модуля
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { printf(" GET_MODULE_DESCRIPTION() --> Bad\n"); exit(1); }
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// отобразим параметры сбора данных модуля на экране монитора
	printf(" \n");
	printf(" Module E14-140 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);

	// цикл перманентного выполнения функций TTL_IN() и TTL_OUT()
	// с отображением полученных данных на экране диплея
	printf("\n Press any key if you want to terminate this program...\n");
	// проинициализируем состояния выходных цифровых линий
	TtlOut = 0x0;
	printf("\n\n");
	printf("                             In         Out\n");
	while(!kbhit())
	{
		// выводим текущие состояния выходных цифровых линий
		if(!pModule->TTL_OUT(TtlOut)) { printf("\n\n  TTL_OUT() --> Bad\n");	break; }
		// теперь читаем состояния входных цифровых линий
		else if(!pModule->TTL_IN(&TtlIn)) { printf("\n\n  TTL_IN() --> Bad\n");	break; }
		// инкрементируем состояния выходных цифровых линий
		TtlOut++;

		printf(" Digital Lines States:     0x%04X     0x%04X\r", TtlIn, TtlOut);
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
	if(kbhit()) { while(kbhit()) getch(); }

	// если нужно - аварийно завершаем программу
	if(AbortionFlag) exit(0x1);
	// или спокойно выходим из функции
	else return;
}


