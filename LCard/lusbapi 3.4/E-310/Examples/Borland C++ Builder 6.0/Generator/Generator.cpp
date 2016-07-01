//******************************************************************************
// ћодуль E-310.
//  онсольна€ программа с примером организации работы с генератором
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// верси€ библиотеки
DWORD DllVersion;
// указатель на интерфейс модул€
ILE310 *pModule;
// дескриптор устройства
HANDLE ModuleHandle;
// название модул€
char ModuleName[16];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E310 ModuleDescription;
// структура параметрами работы генератора
GENERATOR_PARS_E310 GeneratorPars;

// экранный счетчик-индикатор
DWORD Counter;

//------------------------------------------------------------------------
// основна€ программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// зачистим экран монитора
	clrscr();

	printf(" *********************************\n");
	printf(" Module E-310.							\n");
	printf(" Generator console example.			\n");
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

	// прочитаем название модул€ в обнаруженном виртуальном слоте
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

	// получим информацию из ѕѕ«” модул€
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// получим текущие параметры работы генератора
	if(!pModule->GET_GENERATOR_PARS(&GeneratorPars)) AbortProgram(" GET_GENERATOR_PARS() --> Bad\n");
	else printf(" GET_GENERATOR_PARS() --> OK\n");

	// установим желаемые параметры работы генератора
	GeneratorPars.StartFrequency = 10.0;
	GeneratorPars.FrequencyIncrements = 1.0;
	GeneratorPars.NumberOfIncrements = 250;
	GeneratorPars.IncrementIntervalPars.BaseIntervalType = CLOCK_PERIOD_INCREMENT_INTERVAL_E310;
	GeneratorPars.IncrementIntervalPars.MultiplierIndex = INCREMENT_INTERVAL_MULTIPLIERS_500_E310;
	GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber = 250;
	GeneratorPars.MasterClock = 50000.0;
	GeneratorPars.MasterClockSource = INTERNAL_MASTER_CLOCK_E310;
	GeneratorPars.CyclicAutoScanType = CYCLIC_TRIANGLE_AUTO_SCAN_E310;//NO_CYCLIC_AUTOSCAN_E310;
	GeneratorPars.IncrementType = AUTO_INCREMENT_E310;
	GeneratorPars.CtrlLineType = INTERNAL_CTRL_LINE_E310;
	GeneratorPars.InterrupLineType = INTERNAL_INTERRUPT_LINE_E310;
	GeneratorPars.SquareWaveOutputEna = false;
	GeneratorPars.SynchroOutEna = true;//false;
	GeneratorPars.SynchroOutType = SYNCOUT_AT_END_OF_SCAN_E310;
	GeneratorPars.AnalogOutputsPars.SignalType = SINUSOIDAL_ANALOG_OUTPUT_E310;
	GeneratorPars.AnalogOutputsPars.GainIndex = ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310;
	GeneratorPars.AnalogOutputsPars.Output10OhmOffset = 0.0;
	GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource = INTERNAL_OUTPUT_10_OHM_OFFSET_E310;

	// передадим требуемые параметры работы генератора в модуль
	if(!pModule->SET_GENERATOR_PARS(&GeneratorPars)) AbortProgram(" SET_GENERATOR_PARS() --> Bad\n");
	else printf(" SET_GENERATOR_PARS() --> OK\n");

	// запускаем генератор
	if(!pModule->START_GENERATOR()) AbortProgram(" START_GENERATOR() --> Bad\n");
	else printf(" START_GENERATOR() --> OK\n");

	// отобразим параметры модул€ на экране монитора
	printf(" \n\n");
	printf(" Module E-310 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("   Generator parameters:\n");
//	printf("     ADC start source is %s\n", ap.AdcStartSource ? "external" : "internal");
//	printf("     Input Range  = %6.2f Volt\n", ap.InputRange);


	// !!!  ќсновной цикл программы ожидани€ нажати€ клавиши ESC !!!
	printf("\n Press any key if you want to terminate this program...\n\n");
	printf("\n Time: %8lu s\r", Counter = 0x0);
	while(!kbhit())
	{
		Sleep(1000);
		printf(" Time: %8lu s\r", ++Counter);
	}

	// освободим интерфейс модул€
	printf("\n\n");
	AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------
// аварийное завершение программы
//------------------------------------------------------------------------
void AbortProgram(char *ErrorString, bool AbortionFlag)
{
	// подчищаем интерфейс модул€
	if(pModule)
	{
		// остановим генератор
		if(!pModule->STOP_GENERATOR()) printf(" STOP_GENERATOR() --> Bad\n");
		else printf(" STOP_GENERATOR() --> OK\n");
		// освободим интерфейс модул€
		if(!pModule->ReleaseLInstance()) printf(" ReleaseLInstance() --> Bad\n");
		else printf(" ReleaseLInstance() --> OK\n");
		// обнулим указатель на интерфейс модул€
		pModule = NULL;
	}

	// выводим текст сообщени€
	if(ErrorString) printf(ErrorString);

	// прочистим очередь клавиатуры
	if(kbhit()) { while(kbhit()) getch(); }

	// если нужно - аварийно завершаем программу
	if(AbortionFlag) exit(0x1);
	// или спокойно выходим из функции
	else return;
}
