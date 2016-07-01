//******************************************************************************
// Модуль E-154.
// Консольная программа с примером организации однократного ввода отсчёта с АЦП
//	с помощью интерфейсной функции ADC_SAMPLE()
// Ввод осуществляется с первых двух каналов АЦП
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// версия библиотеки
DWORD DllVersion;
// указатель на интерфейс модуля
ILE154 *pModule;
// дескриптор устройства
HANDLE ModuleHandle;
// название модуля
char ModuleName[7];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E154 ModuleDescription;
// структура параметров работы АЦП модуля
ADC_PARS_E154 ap;

// отсчёты АЦП
SHORT AdcSample1, AdcSample2;
// индекс входного диапазона напряжения
//const WORD InputRangeIndex = ADC_INPUT_RANGE_10000mV_E154;
const WORD InputRangeIndex = ADC_INPUT_RANGE_5000mV_E154;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
   double AdcVolts[2];
   DWORD x, y;

	// зачистим экран монитора
	clrscr();

	printf(" *********************************\n");
	printf(" Module E-154                     \n");
	printf(" Console example AdcSample stream \n");
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
	pModule = static_cast<ILE154 *>(CreateLInstance("e154"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// попробуем обнаружить модуль E-154 в первых WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// что-нибудь обнаружили?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E-154 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);


	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

	// прочитаем название модуля в обнаруженном виртуальном слоте
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// проверим, что это 'E14-154'
	if(strcmp(ModuleName, "E154")) AbortProgram(" The module is not 'E-154'\n");
	else printf(" The module is 'E-154'\n");

	// попробуем получить скорость работы шины USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// теперь отобразим скорость работы шины USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// получим информацию из ППЗУ модуля
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// отобразим параметры модуля на экране монитора
	printf(" \n\n");
	printf(" Module E-154 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("   Adc parameters:\n");
	printf("     Input Range  = %6.2f Volt\n", ADC_INPUT_RANGES_E154[InputRangeIndex]);


	printf("\n Press any key if you want to terminate this program...\n");
	// цикл перманентного выполнения функции ADC_SAMPLE и
	// отображения полученных данных на экране диплея
	printf("\n\n");
	printf(" ADC Channel:                 1        2                     1        2\n");
   x=wherex();
   y=wherey();

	while(!kbhit())
	{
		if(!pModule->ADC_SAMPLE(&AdcSample1, (WORD)(0x00  | (InputRangeIndex << 6)))) { printf("\n\n  ADC_SAMPLE(, 0) --> Bad\n");	break; }
    		else if(!pModule->ADC_SAMPLE(&AdcSample2, (WORD)(0x01  | (InputRangeIndex << 6)))) { printf("\n\n  ADC_SAMPLE(, 1) --> Bad\n");	break; }

      // преобразуем коды АЦП в вольты используя калибровочные коэффциенты
      if(!pModule->ProcessOnePoint(AdcSample1, &AdcVolts[0], (WORD)(0x00  | (InputRangeIndex << 6)), TRUE, TRUE))
       { printf("\n\n  PreocessOnePoint() --> Bad\n");	break; }
      if(!pModule->ProcessOnePoint(AdcSample2, &AdcVolts[1], (WORD)(0x01  | (InputRangeIndex << 6)), TRUE, TRUE))
      	{ printf("\n\n  ProcessOtPoint() --> Bad\n");	break; }

  	   gotoxy(x,y);
		printf(" AdcSample Data (ADC code): %5d   %5d;  (ADC volts): %7.3f  %7.3f\r", AdcSample1, AdcSample2,  AdcVolts[0], AdcVolts[1]);
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

