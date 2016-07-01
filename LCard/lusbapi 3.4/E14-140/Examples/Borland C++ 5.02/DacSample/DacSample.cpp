//******************************************************************************
// Модуль E14-140.
// Консольная программа с организацией однократного ввода данных на оба канала ЦАП
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// версия библиотеки
DWORD DllVersion;
// указатель на интерфейс модуля
ILE140 *pModule;
// дескриптор устройства
HANDLE ModuleHandle;
// название модуля
char ModuleName[7];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E140 ModuleDescription;

// отсчёты для ЦАП
SHORT DacSample0, DacSample1;
// состояния входных цифровых линий
//WORD TtlIn;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// зачистим экран монитора
	clrscr();

	printf(" *************************************\n");
	printf(" Module E14-140                       \n");
	printf(" Console example for DAC_SAMPLE Stream\n");
	printf(" *************************************\n\n");

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

	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

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
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// проверим есть ли на модуле ЦАП
	if(ModuleDescription.Dac.Active == DAC_INACCESSIBLED_E140) AbortProgram(" DAC is absent on this module E14-140!\n");

	// теперь отобразим параметры ЦАП на дисплее
	printf(" \n");
	printf(" Module E14-140 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("\n  Now SINUS signal is on the first DAC channel and \n");
	printf("     -SINUS signal is on the second DAC channel.\n");
	printf("      (Press any key to terminate the program)\n");

	// Цикл перманетной работы функции DAC_SAMPLE().
	// При этом на первом канале ЦАП будет выводится синус,
	// а на втором канале ЦАП будет выводится -синус.
	// Т.е. сигналы на каналах ЦАП будут выводится в противофазе ;)
	double CurrentTime = 0.0;
	for(DWORD i = 0 ; ; i++)
	{
		DacSample0 = (SHORT)(2000.0*sin(2.0*M_PI*2.5*CurrentTime));
		DacSample1 = (SHORT)(-DacSample0);
		CurrentTime += 1.0/125.0;

		DacSample0 = (SHORT)(2000);
		DacSample1 = (SHORT)(-DacSample0);

		// если была нажата какая-либо клавиша, то тупо выйдем из цикла
		if(kbhit()) break;
		// выведем очередной отсчёт на первый канал ЦАП
		else if(!pModule->DAC_SAMPLE(&DacSample0, WORD(0x0))) { printf(" DAC_SAMPLE(0) --> Bad\n"); break; }
		// читаем состояния входных цифровых линий
//		else if(!pModule->TTL_IN(&TtlIn)) { printf("\n\n  TTL_IN() --> Bad\n");	break; }
		// выведем очередной отсчёт на второй канал ЦАП
		else if(!pModule->DAC_SAMPLE(&DacSample1, WORD(0x1))) { printf(" DAC_SAMPLE(1) --> Bad\n"); break; }

		// изредка моргнём строчкой с точками
		if(!(i%30))
		{
			static flag;

			flag ^= -0x1;
			if(flag) printf("                                             \r");
			else printf(" ............................................\r");
		}
	}

	printf("                                             \n");

	// выставим нулевые значения на выходах обоих каналов ЦАП
	DacSample0 = (SHORT)(0x0);
	if(!pModule->DAC_SAMPLE(&DacSample0, WORD(0x0))) printf(" DAC_SAMPLE(0) --> Bad\n");
	else if(!pModule->DAC_SAMPLE(&DacSample0, WORD(0x1))) printf(" DAC_SAMPLE(1) --> Bad\n");

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

