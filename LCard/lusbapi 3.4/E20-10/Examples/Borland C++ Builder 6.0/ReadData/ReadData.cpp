//******************************************************************************
// Модуль E20-10.
// Консольная программа с организацией потокового ввода данных с АЦП
// с одновременной записью получаемых данных на диск в реальном масштабе времени.
// Ввод осуществляется с четырёх каналов АЦП на частоте 5000 кГц.
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

// идентификатор потока сбора данных
HANDLE hReadThread;
DWORD ReadTid;

// версия библиотеки
DWORD DllVersion;
// указатель на интерфейс модуля
ILE2010 *pModule;
// дескриптор устройства
HANDLE ModuleHandle;
// название модуля
char ModuleName[7];
// скорость работы шины USB
BYTE UsbSpeed;
// структура с полной информацией о модуле
MODULE_DESCRIPTION_E2010 ModuleDescription;
// структура параметров работы АЦП модуля
ADC_PARS_E2010 ap;

// размер запроса на сбор данных ReadData()
DWORD DataStep;
// будем собирать NDataBlock блоков по DataStep отсчётов в каждом
const WORD NDataBlock = 80;
// частота работы АЦП в кГц
const double AdcRate = 5000.0;
// буфер данных
SHORT *AdcBuffer;
// структура состояния процесса сбора данных
DATA_STATE_E2010 DataState;

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
	WORD i, j;
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

	printf(" *******************************\n");
	printf(" Module E20-10                 \n");
	printf(" Console example for ADC Stream \n");
	printf(" *******************************\n\n");

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
	pModule = static_cast<ILE2010 *>(CreateLInstance("e2010"));
	if(!pModule) AbortProgram(" Module Interface --> Bad\n");
	else printf(" Module Interface --> OK\n");

	// попробуем обнаружить модуль E20-10 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// что-нибудь обнаружили?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) AbortProgram(" Can't find any module E20-10 in first 127 virtual slots!\n");
	else printf(" OpenLDevice(%u) --> OK\n", i);

	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) AbortProgram(" GetModuleHandle() --> Bad\n");
	else printf(" GetModuleHandle() --> OK\n");

	// прочитаем название модуля в обнаруженном виртуальном слоте
	if(!pModule->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
	else printf(" GetModuleName() --> OK\n");
	// проверим, что это 'E20-10'
	if(strcmp(ModuleName, "E20-10")) AbortProgram(" The module is not 'E20-10'\n");
	else printf(" The module is 'E20-10'\n");

	// попробуем получить скорость работы шины USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
	else printf(" GetUsbSpeed() --> OK\n");
	// теперь отобразим скорость работы шины USB
	printf("   USB is in %s\n", UsbSpeed ? "High-Speed Mode (480 Mbit/s)" : "Full-Speed Mode (12 Mbit/s)");

	// Образ ПЛИС возьмём из соответствующего ресурса штатной DLL библиотеки
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
	if(ModuleDescription.Module.Revision == REVISIONS_E2010[REVISION_A_E2010])
		ap.IsAdcCorrectionEnabled = FALSE;		// запретим автоматическую корректировку данных на уровне модуля (для Rev.A)
	else
		ap.IsAdcCorrectionEnabled = TRUE;		// разрешим автоматическую корректировку данных на уровне модуля (для Rev.B и выше)
	ap.SynchroPars.StartSource = INT_ADC_START_E2010;			// внутренний старт сбора с АЦП
//	ap.SynchroPars.StartSource = EXT_ADC_START_ON_RISING_EDGE_E2010;		// внешний старт сбора с АЦП
	ap.SynchroPars.SynhroSource = INT_ADC_CLOCK_E2010;			// внутренние тактовые импульсы АЦП
	ap.SynchroPars.StartDelay = 0x0;									// задержка начала сбора данных в кадрах отсчётов (для Rev.B и выше)
	ap.SynchroPars.StopAfterNKadrs = 0x0;							// останов сбора данных через заданное кол-во кадров отсчётов (для Rev.B и выше)
	ap.SynchroPars.SynchroAdMode = NO_ANALOG_SYNCHRO_E2010;	// тип аналоговой синхронизации (для Rev.B и выше)
//	ap.SynchroPars.SynchroAdMode = ANALOG_SYNCHRO_ON_HIGH_LEVEL_E2010;
	ap.SynchroPars.SynchroAdChannel = 0x0;							// канал аналоговой синхронизации (для Rev.B и выше)
	ap.SynchroPars.SynchroAdPorog = 0;								// порог аналоговой синхронизации в кодах АЦП (для Rev.B и выше)
	ap.SynchroPars.IsBlockDataMarkerEnabled = 0x0;				// маркирование начала блока данных (удобно, например, при аналоговой синхронизации ввода по уровню) (для Rev.B и выше)
	ap.ChannelsQuantity = CHANNELS_QUANTITY; 		// кол-во активных каналов
	// формируем управляющую таблицу
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i);
	// частоту сбора будем устанавливать в зависимости от скорости USB
	ap.AdcRate = AdcRate; 													// частота работы АЦП в кГц
	if(UsbSpeed == USB11_LUSBAPI)
	{
		ap.InterKadrDelay = 0.01;		// межкадровая задержка в мс
		DataStep = 256*1024;				// размер запроса
	}
	else
	{
		ap.InterKadrDelay = 0.0;		// межкадровая задержка в мс
		DataStep = 1024*1024;			// размер запроса
	}
	// сконфигурим входные каналы
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		ap.InputRange[i] = ADC_INPUT_RANGE_3000mV_E2010;  	// входной диапазоне 3В
		ap.InputSwitch[i] = ADC_INPUT_SIGNAL_E2010;			// источник входа - сигнал
	}
	// передаём в структуру параметров работы АЦП корректировочные коэффициенты АЦП
	for(i = 0x0; i < ADC_INPUT_RANGES_QUANTITY_E2010; i++)
		for(j = 0x0; j  < ADC_CHANNELS_QUANTITY_E2010; j++)
		{
			// корректировка смещения
			ap.AdcOffsetCoefs[i][j] = ModuleDescription.Adc.OffsetCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
			// корректировка масштаба
			ap.AdcScaleCoefs[i][j] = ModuleDescription.Adc.ScaleCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
		}
	// передадим требуемые параметры работы АЦП в модуль
	if(!pModule->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");
	else printf(" SET_ADC_PARS() --> OK\n");

	// выделим память под буфер
	AdcBuffer = new SHORT[2*DataStep];
	if(!AdcBuffer) AbortProgram(" Can not allocate memory\n");

	// откроем файл для записи получаемых с модуля данных
	hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_WRITE_THROUGH, NULL);
	if(hFile == INVALID_HANDLE_VALUE) AbortProgram("\n Can't create file 'Test.dat'!\n");

	// Создаём и запускаем поток сбора данных
	printf(" \n");
	hReadThread = CreateThread(0, 0x2000, ServiceReadThread, 0, 0, &ReadTid);
	if(!hReadThread) AbortProgram(" ServiceReadThread() --> Bad\n");
	else printf(" ServiceReadThread() --> OK\n");

	// отобразим параметры сбора данных модуля на экране монитора
	printf(" \n");
	printf(" Module E20-10 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.FwVersion.Version, ModuleDescription.Mcu.Version.FwVersion.Date);
	printf("     PLD    Version     is %s (%s)\n", ModuleDescription.Pld.Version.Version, ModuleDescription.Pld.Version.Date);
	printf("   Adc parameters:\n");
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.2f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.5f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.2f kHz\n", ap.KadrRate);

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
	WORD i;
	WORD RequestNumber;
	DWORD FileBytesWritten;
	// массив OVERLAPPED структур из двух элементов
	OVERLAPPED ReadOv[2];
	// массив структур с параметрами запроса на ввод/вывод данных
	IO_REQUEST_LUSBAPI IoReq[2];

	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(!pModule->STOP_ADC()) { ReadThreadErrorNumber = 0x1; IsReadThreadComplete = true; return 0x0; }

	// формируем необходимые для сбора данных структуры
	for(i = 0x0; i < 0x2; i++)
	{
		// инициализация структуры типа OVERLAPPED
		ZeroMemory(&ReadOv[i], sizeof(OVERLAPPED));
		// создаём событие для асинхронного запроса
		ReadOv[i].hEvent = CreateEvent(NULL, FALSE , FALSE, NULL);
		// формируем структуру IoReq
		IoReq[i].Buffer = AdcBuffer + i*DataStep;
		IoReq[i].NumberOfWordsToPass = DataStep;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &ReadOv[i];
		IoReq[i].TimeOut = DataStep/ap.KadrRate + 1000;
	}

	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadOv[0].hEvent); CloseHandle(ReadOv[1].hEvent); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// запустим АЦП
	if(pModule->START_ADC())
	{
		// цикл сбора данных
		for(i = 0x1; i < NDataBlock; i++)
		{
			// сделаем запрос на очередную порцию данных
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }
			if(ReadThreadErrorNumber) break;

			// ждём завершения операции сбора предыдущей порции данных
			if(WaitForSingleObject(ReadOv[RequestNumber^0x1].hEvent, IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; break; }
			if(ReadThreadErrorNumber) break;

			// попробуем получить текущее состояние процесса сбора данных
			if(!pModule->GET_DATA_STATE(&DataState)) { ReadThreadErrorNumber = 0x7; break; }
			// теперь можно проверить этот признак переполнения внутреннего буфера модуля
			else if(DataState.BufferOverrun == (0x1 << BUFFER_OVERRUN_E2010)) { ReadThreadErrorNumber = 0x8; break; }

			// всё в порядке. запишем полученную порцию данных в файл
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

			// для примера вносим задержку - для нарушения целостности получаемых данных
//			if(i == 33) Sleep(1100);
		}

		// последняя порция данных
		if(!ReadThreadErrorNumber)
		{
			RequestNumber ^= 0x1;
			// ждём окончания операции сбора последней порции данных
			if(WaitForSingleObject(ReadOv[RequestNumber^0x1].hEvent, IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) ReadThreadErrorNumber = 0x3;
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

	// для примера вносим задержку - для нарушения целостности получаемых данных
//	Sleep(1100);

	// остановим работу АЦП
	// ВСЕХ собраных данных, то функцию STOP_ADC() следует выполнять не позднее,
	// чем через 800 мс после окончания ввода последней порции данных.
	// Для заданной частоты сбора данных в 5 МГц эта величина определяет время
	// переполнения внутренненого FIFO буфера модуля, который имеет размер 8 Мб. 
	if(!pModule->STOP_ADC()) ReadThreadErrorNumber = 0x1;
	// если нужно - анализируем окончательный признак переполнения внутреннего буфера модуля
	if(DataState.BufferOverrun != (0x1 << BUFFER_OVERRUN_E2010))
	{
		// попробуем получить окончательный состояние процесса сбора данных
		if(!pModule->GET_DATA_STATE(&DataState)) ReadThreadErrorNumber = 0x7;
		// теперь можно проверить этот признак переполнения внутреннего буфера модуля
	   else if(DataState.BufferOverrun == (0x1 << BUFFER_OVERRUN_E2010)) ReadThreadErrorNumber = 0x8;
	}
	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(ModuleHandle)) ReadThreadErrorNumber = 0x9;
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadOv[i].hEvent);
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
			printf(" ADC Thread: Writing data file error!\n");
			break;

		case 0x5:
			// если программа была злобно прервана, предъявим ноту протеста
			printf(" ADC Thread: The program was terminated!\n");
			break;

		case 0x6:
			printf(" ADC Thread: START_ADC() --> Bad\n");
			break;

		case 0x7:
			printf(" ADC Thread: CHECK_DATA_INTERGRITY() --> Bad\n");
			break;

		case 0x8:
			printf(" ADC Thread: Data integrity --> Bad\n");
			break;

		case 0x9:
			printf(" ADC Thread: Can't cancel pending input and output (I/O) operations!\n");
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

