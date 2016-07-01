//******************************************************************************
// Модуль E14-440.
// Консольная программа с организацией синхронного варианта запроса на сбор данных
// с последующей записью получаемых данных на диск.
// Ввод осуществляется с первых четырёх каналов АЦП на частоте 400 кГц.
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);

// идентификатор файла
HANDLE hFile;
DWORD FileBytesWritten;

// версия библиотеки
DWORD DllVersion;
// указатель на интерфейс модуля
ILE440 *pModule;
// дескриптор устройства
HANDLE ModuleHandle;
// название модуля
char ModuleName[7];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E440 ModuleDescription;
// структура параметров работы АЦП модуля
ADC_PARS_E440 ap;
// структура с параметрами запроса на ввод/вывод данных
IO_REQUEST_LUSBAPI IoReq;

// кол-во получаемых отсчетов (кратное 32) для Ф. ReadData()
DWORD DataStep = 256*1024;
// буфер данных
SHORT *ReadBuffer;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
//	WORD DacSample;

	// пока ничего не выделено под буфер данных
	ReadBuffer = NULL;
	// пока откытого файла нет :(
	hFile = INVALID_HANDLE_VALUE;

	// зачистим экран монитора
	clrscr();

	printf(" ***************************************\n");
	printf(" Module E14-440                 			\n");
	printf(" Console example for ADC Synchro Stream \n");
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
	pModule = static_cast<ILE440 *>(CreateLInstance("e440"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// попробуем обнаружить модуль E14-440 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// что-нибудь обнаружили?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E14-440 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

	// прочитаем название модуля в обнаруженном виртуальном слоте
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// проверим, что это 'E14-440'
	if(strcmp(ModuleName, "E440")) AbortProgram(" The module is not 'E14-440'\n");
	else printf(" The module is 'E14-440'\n");

	// попробуем получить скорость работы шины USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// теперь отобразим скорость работы шины USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// код LBIOS'а возьмём из соответствующего ресурса штатной DLL библиотеки
	if(!pModule->LOAD_MODULE()) AbortProgram(" LOAD_MODULE() --> Bad\n");
	else printf(" LOAD_MODULE() --> OK\n");

	// проверим загрузку модуля
 	if(!pModule->TEST_MODULE()) AbortProgram(" TEST_MODULE() --> Bad\n");
	else printf(" TEST_MODULE() --> OK\n");

	// получим информацию из ППЗУ модуля
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
	else printf(" GET_MODULE_DESCRIPTION() --> OK\n");

	// получим текущие параметры работы АЦП
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");

	// установим желаемые параметры работы АЦП
	ap.IsCorrectionEnabled = true; 			// разрешим корректировку данных на уровне драйвера DSP
	ap.InputMode = NO_SYNC_E440;				// обычный сбор данных безо всякой синхронизации ввода
	ap.ChannelsQuantity = 0x4;					// четыре активных канала
	// формируем управляющую таблицу
	for(i = 0x0; i < ap.ChannelsQuantity; i++)
		ap.ControlTable[i] = (WORD)(i | (ADC_INPUT_RANGE_2500mV_E440 << 0x6));
	ap.AdcRate = 400.0;							// частота работы АЦП в кГц
	ap.InterKadrDelay = 0.0;					// межкадровая задержка в мс
	ap.AdcFifoBaseAddress = 0x0;			  	// базовый адрес FIFO буфера АЦП в DSP модуля
	ap.AdcFifoLength = MAX_ADC_FIFO_SIZE_E440;	// длина FIFO буфера АЦП в DSP модуля
	// будем использовать фирменные калибровочные коэффициенты, которые храняться в ППЗУ модуля
	for(i = 0x0; i < ADC_CALIBR_COEFS_QUANTITY_E440; i++)
	{
		ap.AdcOffsetCoefs[i] =  ModuleDescription.Adc.OffsetCalibration[i];
		ap.AdcScaleCoefs[i] =  ModuleDescription.Adc.ScaleCalibration[i];
	}

	// передадим требуемые параметры работы АЦП в модуль
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// выделим память под буфер
	ReadBuffer = new SHORT[DataStep];
	if(!ReadBuffer) AbortProgram(" Can not allocate memory\n");

	// откроем файл для записи получаемых с модуля данных
	hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_WRITE_THROUGH, NULL);
	if(hFile == INVALID_HANDLE_VALUE) AbortProgram("\n Can't create file 'Test.dat'!\n");

	// отобразим параметры сбора данных модуля на экране монитора
	printf(" \n");
	printf(" Module E14-440 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("     LBIOS   Version    is %s (%s)\n", ModuleDescription.Dsp.Version.Version, ModuleDescription.Dsp.Version.Date);
	printf("     %s Mode\n", UsbSpeed ? "HIGH-SPEED (480 Mbit/s)" : "FULL-SPEED (12 Mbit/s)");
	printf("   Adc parameters:\n");
	printf("     Data Correction is %s\n", ap.IsCorrectionEnabled ? "enabled" : "disabled");
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.3f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.4f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.3f kHz\n\n", ap.KadrRate);

	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(!pModule->STOP_ADC()) AbortProgram(" STOP_ADC() --> Bad\n");
	else printf(" STOP_ADC() --> OK\n");

	// формируем структуру IoReq
	IoReq.Buffer = ReadBuffer;					// буфер данных
	IoReq.NumberOfWordsToPass = DataStep;	// кол-во собираемых данных
	IoReq.NumberOfWordsPassed = 0x0;
	IoReq.Overlapped = NULL;					// синхронный вариант запроса
	IoReq.TimeOut = DataStep/ap.AdcRate + 1000;	// таймаут синхронного сбора данных 		

	// запустим АЦП
	if(!pModule->START_ADC()) AbortProgram(" START_ADC() --> Bad\n");
	else printf(" START_ADC() --> OK\n");

	// быстренько делаем запрос на сбор данных
	if(!pModule->ReadData(&IoReq)) AbortProgram(" ReadData() --> Bad\n");
	else
	{
		printf(" ReadData() --> OK\n");

		// теперь запишем в файл полученные данные
		DWORD FileBytesWritten = 0x0;
		if(!WriteFile(	hFile,								// handle to file to write to
	    					IoReq.Buffer,			 			// pointer to data to write to file
							2*IoReq.NumberOfWordsToPass,	// number of bytes to write
    						&FileBytesWritten,				// pointer to number of bytes written
					   	NULL			  						// pointer to structure needed for overlapped I/O
					   ))  AbortProgram(" WriteFile(Test.dat) --> Bad\n");
		else printf(" WriteFile(Test.dat) --> Ok\n");
	}      

	// остановим работу АЦП
	if(!pModule->STOP_ADC()) AbortProgram(" STOP_ADC() --> Bad\n");
	else printf(" STOP_ADC() --> OK\n");

	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(ModuleHandle)) AbortProgram(" CancelIo() --> Bad\n");
	else printf(" CancelIo() --> OK\n");

	// Всё хорошо!!!
	AbortProgram("\n The program was completed successfully!!!\n", false);
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

	// освободим память буфера
	if(ReadBuffer) { delete[] ReadBuffer; ReadBuffer = NULL; }
	// освободим идентификатор файла данных
	if(hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE; }

	// выводим текст сообщения
	if(ErrorString) printf(ErrorString);

	// прочистим очередь клавиатуры
	if(kbhit()) { while(kbhit()) getch(); }

	// если нужно - аварийно завершаем программу
	if(AbortionFlag) exit(0x1);
	// или спокойно выходим из функции   
	else return;
}

