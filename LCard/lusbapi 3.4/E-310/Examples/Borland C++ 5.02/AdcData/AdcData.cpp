//******************************************************************************
// Модуль E-310.
// Консольная программа с примером организации однократного ввода отсчёта с АЦП
//	с помощью интерфейсной функции GET_ADC_DATA()
// Ввод осуществляется с четырех каналов АЦП
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
// дескриптор устройства
HANDLE ModuleHandle;
// название модуля
char ModuleName[16];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E310 ModuleDescription;
// структура параметров работы АЦП модуля
ADC_PARS_E310 AdcPars;
// структура отсчётов АЦП
ADC_DATA_E310 AdcData;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
   DWORD x, y;

	// зачистим экран монитора
	clrscr();

	printf(" *********************************\n");
	printf(" Module E-310.							\n");
	printf(" ADC console example.					\n");
	printf(" *********************************\n\n");

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
	pModule = static_cast<ILE310 *>(CreateLInstance("e310"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// попробуем обнаружить модуль E-310 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// что-нибудь обнаружили?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E-310 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

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
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// получим текущие параметры работы АЦП
	if(!pModule->GET_ADC_PARS(&AdcPars)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");

	// установим желаемые параметры работы АЦП
	AdcPars.AdcStartSource = INTERNAL_ADC_START_E310;		// внутренний запуск АЦП
	AdcPars.ChannelsMask =	(0x1 << ADC_CHANNEL_1_E310) | (0x1 << ADC_CHANNEL_2_E310) |
									(0x1 << ADC_CHANNEL_3_E310) | (0x1 << ADC_CHANNEL_4_E310);

	// передадим требуемые параметры работы АЦП в модуль
	if(!pModule->SET_ADC_PARS(&AdcPars)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// отобразим параметры модуля на экране монитора
	printf(" \n\n");
	printf(" Module E-310 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("   Adc parameters:\n");
	printf("     ADC start source is %s\n", AdcPars.AdcStartSource ? "external" : "internal");
	printf("     Input Range = %6.2f Volt\n", AdcPars.InputRange);

	printf("\n Press any key if you want to terminate this program...\n");
	// цикл перманентного выполнения функции GET_ADC_DATA и
	// отображения полученных данных на экране диплея
//	printf("\n\n");
	printf(" ADC Channel:                1         2         3         4\n");
	x=wherex();
	y=wherey();
	while(!kbhit())
	{
		// попробуем получить данные с АЦП
		if(!pModule->GET_ADC_DATA(&AdcData)) { printf("\n\n  GET_ADC_DATA() --> Bad\n");	break; }

		// отобразим на экране коды АЦП
		gotoxy(x, y);
		printf(" Adc Data (ADC code):");
		for(i = 0x0; i < ADC_CHANNEL_QUANTITY_E310; i++)
			printf("%10d", AdcData.DataInCode[i]);

		// отобразим на экране Вольты
		gotoxy(x, y + 0x1);
		printf(" Adc Data (V):       ");
		for(i = 0x0; i < ADC_CHANNEL_QUANTITY_E310; i++)
			printf("%10.1f", AdcData.DataInV[i]);

		Sleep(50);
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

