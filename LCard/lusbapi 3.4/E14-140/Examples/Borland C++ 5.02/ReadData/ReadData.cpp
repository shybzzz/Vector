//******************************************************************************
// Модуль E14-140.
// Консольная программа с организацией потокового ввода данных с АЦП
// с одновременной записью получаемых данных на диск в реальном масштабе времени.
// Ввод осуществляется с первый четырех каналов АЦП на частоте работе АЦП 100 кГц
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include "Lusbapi.h"

#define CHANNELS_QUANTITY			(0x4)

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);
// функция потока ввода данных с АЦП
DWORD WINAPI ServiceReadThread(PVOID /*Context*/);
// функция вывода сообщений с ошибками
void ShowThreadErrorMessage(void);

// идентификатор файла
HANDLE hFile;
DWORD FileBytesWritten;

// идентификатор потока сбора данных
HANDLE hReadThread;
DWORD ReadTid;

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
// структура параметров работы АЦП модуля
ADC_PARS_E140 ap;

// кол-во получаемых отсчетов (кратное 32) для Ф. ReadData()
DWORD DataStep = 64*1024;
// будем собирать NDataBlock блоков по DataStep отсчётов в каждом
const WORD NDataBlock = 80;
// буфер данных
SHORT *AdcBuffer;

// флажок завершения работы потока сбора данных
bool IsReadThreadComplete;
// номер ошибки при выполнении сбора данных
WORD ReadThreadErrorNumber;

// экранный счетчик-индикатор
DWORD Counter = 0x0, OldCounter = 0xFFFFFFFF;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
//	WORD DacSample;

	// сбросим флажок завершения потока ввода данных
	IsReadThreadComplete = false;
	// пока ничего не выделено под буфер данных
	AdcBuffer = NULL;
	// пока не создан поток ввода данных
	hReadThread = NULL;
	// пока откытого файла нет :(
	hFile = INVALID_HANDLE_VALUE;
	// сбросим флаг ошибок потока ввода данных
	ReadThreadErrorNumber = 0x0;

	// зачистим экран монитора
	clrscr();

	printf(" ***************************************\n");
	printf(" Module E14-140                         \n");
	printf(" Console example for ADC Stream Reading \n");
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

	// проверим прошивку MCU модуля
   if((ModuleDescription.Module.Revision == REVISIONS_E140[REVISION_B_E140]) &&
   	(strtod((char *)ModuleDescription.Mcu.Version.Version, NULL) < 3.05)) AbortProgram(" For module E14-140(Rev.'B') firmware version must be 3.05 or above --> !!! ERROR !!!\n");

	// получим текущие параметры работы АЦП
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");
	// установим желаемые параметры АЦП
	ap.ClkSource = INT_ADC_CLOCK_E140;							// внутренний запуск АЦП
	ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E140; 	// без трансляции тактовых импульсо АЦП
	ap.InputMode = NO_SYNC_E140;		// без синхронизации ввода данных
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 		// кол-во активных канала
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i | (ADC_INPUT_RANGE_2500mV_E140 << 0x6));
	ap.AdcRate = 100.0;					// частота работы АЦП в кГц
	ap.InterKadrDelay = 0.0;			// межкадровая задержка в мс
	// передадим требуемые параметры работы АЦП в модуль
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// выделим память под буфер
	AdcBuffer = new SHORT[2*DataStep];
	if(!AdcBuffer) AbortProgram(" Can not allocate memory\n");

	// откроем файл для записи получаемых с модуля данных
	printf(" \n");
	hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
	if(hFile == INVALID_HANDLE_VALUE) AbortProgram("\n Can't create file 'Test.dat'!\n");

	// Создаём и запускаем поток сбора данных
	hReadThread = CreateThread(0, 0x2000, ServiceReadThread, 0, 0, &ReadTid);
	if(!hReadThread) AbortProgram(" ServiceReadThread() --> Bad\n");
	else printf(" ServiceReadThread() --> OK\n");

	// отобразим параметры сбора данных модуля на экране монитора
	printf(" \n");
	printf(" Module E14-140 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("   Adc parameters:\n");
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.3f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.4f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.3f kHz\n", ap.KadrRate);

	// цикл записи получаемых данных и ожидания окончания работы приложения
//	DacSample = 0x1000;
	printf("\n Press any key if you want to terminate this program...\n\n");
	while(!IsReadThreadComplete)
	{
		if(OldCounter != Counter) { printf(" Counter %3u from %3u\r", Counter, NDataBlock); OldCounter = Counter; }
		else Sleep(20);
//		if(!pModule->ENABLE_TTL_OUT(TRUE)) AbortProgram(" Ошибка разрешения выходных цифровых линий");
//		if(!pModule->TTL_OUT(0xFFFF)) AbortProgram(" Ошибка установки выходных цифровых линий");
//		if(!pModule->TTL_OUT(0x0000)) AbortProgram(" Ошибка установки выходных цифровых линий");
//		if(!pModule->DAC_SAMPLE((SHORT *)&DacSample, 0x0)) AbortProgram(" Ошибка установки канала ЦАП");
	}

	// ждём окончания работы потока ввода данных
	WaitForSingleObject(hReadThread, INFINITE);

	// проверим была ли ошибка выполнения потока сбора данных
	printf("\n\n");
	if(ReadThreadErrorNumber) { AbortProgram(NULL, false); ShowThreadErrorMessage(); }
	else AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------
// Поток, в котором осуществляется сбор данных
//------------------------------------------------------------------------
DWORD WINAPI ServiceReadThread(PVOID /*Context*/)
{
	WORD RequestNumber;
	DWORD i;
	// идентификатор массива их двух событий
	HANDLE ReadEvent[2];
	// массив OVERLAPPED структур из двух элементов
	OVERLAPPED ReadOv[2];
//	DWORD BytesTransferred[2];
	IO_REQUEST_LUSBAPI IoReq[2];

	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(!pModule->STOP_ADC()) { ReadThreadErrorNumber = 0x1; IsReadThreadComplete = true; return 0x0; }

	// формируем необходимые для сбора данных структуры
	for(i = 0x0; i < 0x2; i++)
	{
		// создаём событие
		ReadEvent[i] = CreateEvent(NULL, FALSE , FALSE, NULL);
		// инициализация структуры типа OVERLAPPED
		ZeroMemory(&ReadOv[i], sizeof(OVERLAPPED)); ReadOv[i].hEvent = ReadEvent[i];
		// формируем структуру IoReq
		IoReq[i].Buffer = AdcBuffer + i*DataStep;
		IoReq[i].NumberOfWordsToPass = DataStep;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &ReadOv[i];
		IoReq[i].TimeOut = DataStep/ap.AdcRate + 1000;
	}

	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// запустим АЦП
	if(pModule->START_ADC())
	{
		// цикл сбора данных
		for(i = 0x1; i < NDataBlock; i++)
		{
			// сделаем запрос на очередную порции данных
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }

			// ждём завершения операции сбора предыдущей порции данных
			if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; break; }

			// запишем полученную порцию данных в файл
			if(!WriteFile(	hFile,													// handle to file to write to
		    					IoReq[RequestNumber^0x1].Buffer,					// pointer to data to write to file
								2*DataStep,	 											// number of bytes to write
	    						&FileBytesWritten,									// pointer to number of bytes written
						   	NULL			  											// pointer to structure needed for overlapped I/O
							   )) { ReadThreadErrorNumber = 0x4; break; }

			if(ReadThreadErrorNumber) break;
			else if(kbhit()) { ReadThreadErrorNumber = 0x5; break; }
			else Sleep(20);
			Counter++;
		}

		// последняя порция данных
		if(!ReadThreadErrorNumber)
		{
			RequestNumber ^= 0x1;
			// ждём окончания операции сбора последней порции данных
			if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) ReadThreadErrorNumber = 0x3;
			// запишем последнюю порцию данных в файл
			if(!WriteFile(	hFile,													// handle to file to write to
		    					IoReq[RequestNumber^0x1].Buffer,					// pointer to data to write to file
								2*DataStep,	 											// number of bytes to write
	    						&FileBytesWritten,									// pointer to number of bytes written
						   	NULL			  											// pointer to structure needed for overlapped I/O
							   )) ReadThreadErrorNumber = 0x4;
			Counter++;
		}
	}
	else { ReadThreadErrorNumber = 0x6; }

	// остановим работу АЦП
	if(!pModule->STOP_ADC()) ReadThreadErrorNumber = 0x1;
	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(pModule->GetModuleHandle())) { ReadThreadErrorNumber = 0x7; }
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadEvent[i]);
	// небольшая задержка
	Sleep(100);
	// установим флажок завершения работы потока сбора данных
	IsReadThreadComplete = true;
	// теперь можно спокойно выходить из потока
	return 0x0;
}

//------------------------------------------------------------------------
// Отобразим сообщение с ошибкой
//------------------------------------------------------------------------
void ShowThreadErrorMessage(void)
{
	switch(ReadThreadErrorNumber)
	{
		case 0x1:
			printf(" ADC Thread: STOP_ADC() --> Bad\n");
			break;

		case 0x2:
			printf(" ADC Thread: ReadData() --> Bad\n");
			break;

		case 0x3:
			printf(" ADC Thread: Timeout is occured!\n");
			break;

		case 0x4:
			printf(" ADC Thread: Writing file error!\n");
			break;

		case 0x5:
			// если программа была злобно прервана, предъявим ноту протеста
			printf(" ADC Thread: The program was terminated!\n");
			break;

		case 0x6:
			printf(" ADC Thread: START_ADC() --> Bad\n");
			break;

		case 0x7:
			printf(" ADC Thread: Can't cancel ending input and output (I/O) operations!\n");
			break;

		default:
			printf(" Unknown error!\n");
			break;
	}

	return;
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
	if(AdcBuffer) { delete[] AdcBuffer; AdcBuffer = NULL; }
	// освободим идентификатор потока сбора данных
	if(hReadThread) { CloseHandle(hReadThread); hReadThread = NULL; }
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

