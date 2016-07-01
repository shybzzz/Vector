//******************************************************************************
// Модуль E14-140.
// Консольная программа с примером организации однократного ввода кадра отсчётов
//	с АЦП с помощью интерфейсной функции ADC_KADR()
// Ввод осуществляется с первый четырех каналов АЦП
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

#define CHANNELS_QUANTITY			(0x4)
//#define CHANNELS_QUANTITY			(0x20)

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
// структура параметров работы АЦП модуля
ADC_PARS_E140 ap;

// буфер данных
SHORT AdcBuffer[CHANNELS_QUANTITY];
// индекс входного диапазона напряжения
const WORD InputRangeIndex = ADC_INPUT_RANGE_2500mV_E140;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// зачистим экран монитора
	clrscr();

	printf(" ***********************************\n");
	printf(" Module E14-140                     \n");
	printf(" Console example for AdcKadr Stream \n");
	printf(" ***********************************\n\n");

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

	// проверим прошивку MCU модуля
   if((ModuleDescription.Module.Revision == REVISIONS_E140[REVISION_B_E140]) &&
   	(strtod((char *)ModuleDescription.Mcu.Version.Version, NULL) < 3.05)) AbortProgram(" For module E14-140(Rev.'B') firmware version must be 3.05 or above --> !!! ERROR !!!\n");

	// получим текущие параметры работы АЦП
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");
	// установим желаемые параметры работы АЦП
	ap.ClkSource = INT_ADC_CLOCK_E140;				// внутренний запуск АЦП
	ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E140; 	// без трансляции тактовых импульсо АЦП
	ap.InputMode = NO_SYNC_E140;						// без синхронизации ввода данных
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 		// четыре активных канала
	// формируем управляющую таблицу
	for(i = 0x0; i < ap.ChannelsQuantity; i++)
		ap.ControlTable[i] = (WORD)(i | (InputRangeIndex << 0x6));
	ap.AdcRate = 100.0;									// частота работы АЦП в кГц
	ap.InterKadrDelay = 0.0;							// межкадровая задержка в мс
	// передадим требуемые параметры работы АЦП в модуль
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// отобразим параметры сбора данных модуля на экране монитора
	printf(" \n");
	printf(" Module E14-140 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("   Adc parameters:\n");
	printf("     Input Range  = %6.2f Volt\n", ADC_INPUT_RANGES_E140[InputRangeIndex]);
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.3f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.4f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.3f kHz\n", ap.KadrRate);

	printf("\n Press any key if you want to terminate this program...\n");
	// цикл перманентного выполнения функции ADC_KADR и
	// отображения полученных данных на экране диплея
	printf("\n\n");
	printf(" ADC Channel:                1         2         3         4\n");
	while(!kbhit())
	{
		if(!pModule->ADC_KADR(AdcBuffer)) AbortProgram("\n\n  ADC_KADR() --> Bad\n");
		printf(" Kadr Data (ADC code):     %5d     %5d     %5d     %5d\r", AdcBuffer[0], AdcBuffer[1], AdcBuffer[2], AdcBuffer[3]);
		Sleep(20);
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

