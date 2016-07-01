//******************************************************************************
// Модуль E-310.
// Консольная программа с примером организации работы с частотомером
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
// структура параметров работы частотометра
FM_PARS_E310 FmPars;
// структура отсчёта частотометра
FM_SAMPLE_E310 FmSample;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// зачистим экран монитора
	clrscr();

	printf(" *********************************\n");
	printf(" Module E-310.							\n");
	printf(" FM console example.					\n");
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

	// -=== параметры частотомеры ===-
	// получаем текущие параметры частотомера
	if(!pModule->GET_FM_PARS(&FmPars)) AbortProgram(" GET_FM_PARS() --> Bad\n");
	else printf(" GET_FM_PARS() --> OK\n");

	FmPars.Mode				= PERIOD_MODE_FM_E310;		// метод: измерение периода входного сигнала
	FmPars.InputDivider	= INPUT_DIVIDER_1_FM_E310;	// делитель частоты 1:1 входного сигнала
//	FmPars.InputDivider	= INPUT_DIVIDER_8_FM_E310;	// делитель частоты 1:8 входного сигнала
	FmPars.BaseClockRateDivIndex = BASE_CLOCK_DIV_02_INDEX_FM_E310;	// делитель базовой частоты
//	FmPars.BaseClockRateDivIndex = BASE_CLOCK_DIV_08_INDEX_FM_E310;	// делитель базовой частоты
	FmPars.Offset			= 0.0;//2.5;					// смещение порогра частотомера в В

	// устанавливаем параметры частотомера
	if(!pModule->SET_FM_PARS(&FmPars)) AbortProgram(" SET_FM_PARS() --> Bad\n");
	else printf(" SET_FM_PARS() --> OK\n");
	// -=============================-

	// запускаем частотомер
	if(!pModule->START_FM()) AbortProgram(" START_FM() --> Bad\n");
	else printf(" START_FM() --> OK\n");

	// отобразим параметры модуля на экране монитора
	printf(" \n\n");
	printf(" Module E-310 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("   FM parameters:\n");
	printf("     Input Divider is %s\n",		FmPars.InputDivider ? "1:1" : "1:8");
	printf("     Clock Rate  = %10.4f kHz\n",	FmPars.ClockRate/1000.0);

	printf("\n Press any key if you want to terminate this program...\n");
	// цикл перманентного выполнения функции GET_ADC_DATA и
	// отображения полученных данных на экране диплея
	printf("\n\n");
	while(!kbhit())
	{
		// пробуем получить отсчёт частотомера
		if(!pModule->FM_SAMPLE(&FmSample)) { printf("\n\n  FM_SAMPLE() --> Bad\n");	break; }
		// отобразим на экране отсчёт частотомера
		if(FmSample.IsActual)
			printf(" FM Sample: %10.3f kHz\r", FmSample.Frequency);
		else
			printf(" FM Sample: --------------\r");
		// задержечка
		Sleep(200);
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
		// остановим частотомер
		if(!pModule->STOP_FM()) printf(" STOP_FM() --> Bad\n");
		else printf(" STOP_FM() --> OK\n");
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
