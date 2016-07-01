//******************************************************************************
// Модуль E14-440.
// Консольная программа с организацией одновременного потокового ввода и вывода данных.
// Ввод осуществляется с первых четырёх каналов АЦП. Частота ввода 100(Rev.A-C) или 250(Rev.D и выше) кГц.
// Выводится синусоидальный сигнал на первый канал ЦАП. Частота вывода 100(Rev.A-C) или 125(Rev.D и выше) кГц.
//******************************************************************************
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <FCNTL.h>
#include <io.h>
#include "Lusbapi.h"

// кол-во вводимых блоков данных
#define		ADC_DATA_BLOCK_QUANTITY			(20)

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);
// функция вывода сообщений с ошибками
void ShowErrorMessage(void);
// округление
WORD Round(double Data);

// определяем константу pi
const double M_PI = 3.14159265358979323846;

// ***** ПОТОК ВВОДА ДАННЫХ *****
// функция потока ввода данных с АЦП
DWORD 	WINAPI ServiceReadThread(PVOID /*Context*/);
// ожидание завершения асинхронного запроса на ввод данных
BOOL WaitingForReadRequestCompleted(OVERLAPPED *ReadOv);
// идентификатор потока ввода
HANDLE 	hReadThread;
DWORD 	ReadTid;
// ******************************

// ***** ПОТОК ВЫВОДА ДАННЫХ *****
// функция потока вывода данных на ЦАП
DWORD 	WINAPI ServiceWriteThread(PVOID /*Context*/);
// ожидание завершения асинхронного запроса на вывод данных
BOOL WaitingForWriteRequestCompleted(OVERLAPPED *ReadOv);
// идентификатор потока вывода
HANDLE 	hWriteThread;
DWORD 	WriteTid;
// ******************************

// идентификатор файла для получаемых данных
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
// структура параметров работы ЦАП
DAC_PARS_E440 dp;
// частоты работы АЦП и ЦАП
double AdcRate, DacRate;

//max возможное кол-во передаваемых отсчетов (кратное 32) для ф. ReadData и WriteData()
DWORD DataStep = 256*1024;
// общее кол-во данных для  ввода с АЦП
DWORD PointsToRead = ADC_DATA_BLOCK_QUANTITY * DataStep;
// указатель на буфер для данных с АЦП
SHORT	*ReadBuffer;
// указатель на буфер данных для ЦАП
WORD 	*WriteBuffer;

// параметры сигнала, выводимого на ЦАП
const WORD DacNumber = 0x0;						// номер канала ЦАП, на который будет выводиться сигнал
const double SignalFrequency = 0.73;//0.073; 	  		// в кГц
const double SignalAmplitude = 2000;			// в кодах ЦАП

// флажки завершения потоков ввода и вывода данных
bool IsReadThreadComplete, IsWriteThreadComplete;
// номер ошибки при выполнении сбора данных
WORD ReadThreadErrorNumber;
// номер ошибки при выполнении потока вывода данных на ЦАП
WORD WriteThreadErrorNumber;

// экранный счетчик-индикатор
DWORD Counter = 0x0, OldCounter = 0xFFFFFFFF;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i;
	SHORT DacSample;

	// сбросим флажок завершения потока ввода данных
	IsReadThreadComplete = false;
	// пока ничего не выделено под буфер данных
	ReadBuffer = NULL;
	// пока не создан поток ввода данных
	hReadThread = NULL;
	// пока откытого файла нет :(
	hFile = INVALID_HANDLE_VALUE;
	// сбросим флаг ошибок потока ввода данных
	ReadThreadErrorNumber = 0x0;

	// сбросим флажок завершения потока вывода данных
	IsWriteThreadComplete = false;
	// пока ничего не выделено под буфер данных
	WriteBuffer = NULL;
	// пока не создан поток ввода данных
	hWriteThread = NULL;
	// сбросим флаг ошибок потока ввода данных
	WriteThreadErrorNumber = 0x0;

	// зачистим экран монитора
	system("cls");

	printf(" ************************************\n");
	printf(" Module E14-440                      \n");
	printf(" Console example for ADC&DAC Streams \n");
	printf(" ************************************\n\n");

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

	// проверим есть ли на модуле ЦАП
	if(ModuleDescription.Dac.Active == DAC_INACCESSIBLED_E440) AbortProgram(" DAC is absent on this module E14-440!\n");
	//  
	if((ModuleDescription.Module.Revision >= 'A') && (ModuleDescription.Module.Revision <= 'C'))
		{ AdcRate = DacRate = 100.0; }
	else { AdcRate = 250.0; DacRate = 125.0; }

	// ***** параметры АЦП *****
	// получим текущие параметры работы АЦП
	if(!pModule->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
	else printf(" GET_ADC_PARS() --> OK\n");

	// установим желаемые параметры работы АЦП
	ap.IsCorrectionEnabled = TRUE;			// разрешим корректировку данных на уровне драйвера DSP
	ap.InputMode = NO_SYNC_E440;				// обычный сбор данных безо всякой синхронизации ввода
	ap.ChannelsQuantity = 0x4;					// четыре активных канала
	// формируем управляющую таблицу
	for(i = 0x0; i < ap.ChannelsQuantity; i++)
//		ap.ControlTable[i] = (WORD)(i | (ADC_INPUT_RANGE_2500mV_E440 << 0x6));
		ap.ControlTable[i] = (WORD)(i | (ADC_INPUT_RANGE_10000mV_E440 << 0x6));
	ap.AdcRate = AdcRate;						// частота работы АЦП в кГц
	ap.InterKadrDelay = 0.0;					// межкадровая задержка в мс
	ap.AdcFifoBaseAddress = 0x0;			  			// базовый адрес FIFO буфера АЦП в DSP модуля
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
	// *************************


	// ***** параметры ЦАП *****
	// получим текущие параметры работы ЦАП
	if(!pModule->GET_DAC_PARS(&dp)) AbortProgram(" GET_CUR_DAC_PARS() --> Bad\n");
	else printf(" GET_CUR_DAC_PARS() --> OK\n");

	// установим желаемые параметры ЦАП
	dp.DacRate = DacRate;								// частота работы ЦАП в кГц
	dp.DacFifoLength = MAX_DAC_FIFO_SIZE_E440;  	// длина FIFO буфера ЦАП в DSP модуля

	if(!pModule->SET_DAC_PARS(&dp)) AbortProgram(" SET_DAC_PARS() --> Bad\n");
	else printf(" SET_DAC_PARS() --> OK\n");
	// *************************

	// попробуем выделить память под буфер АЦП
	ReadBuffer = new SHORT[PointsToRead];
	if(!ReadBuffer) AbortProgram(" Can not allocate memory\n");
	// попробуем выделить память под буфер ЦАП
	WriteBuffer = new WORD[2*DataStep];
	if(!WriteBuffer) AbortProgram(" Cannot allocate WriteBuffer memory\n");

	// откроем файл для записи получаемых с модуля данных
	hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_WRITE_THROUGH, NULL);
	if(hFile == INVALID_HANDLE_VALUE) AbortProgram("\n Can't create file 'Test.dat'!\n");

	// выставим нулевые значения на выходах обоих каналов ЦАП
	DacSample = (SHORT)(0x0);
	if(!pModule->DAC_SAMPLE(&DacSample, WORD(0x0))) AbortProgram(" DAC_SAMPLE(0) --> Bad\n");
	else if(!pModule->DAC_SAMPLE(&DacSample, WORD(0x1))) AbortProgram(" DAC_SAMPLE(1) --> Bad\n");

	// Создаем и запускаем поток сбора данных с АЦП
	printf("\n");
	hReadThread = CreateThread(0, 0x2000, ServiceReadThread, 0, 0, &ReadTid);
	if(!hReadThread) AbortProgram(" ServiceReadThread() --> Bad\n");
	else printf(" ServiceReadThread() --> OK\n");
	// Создаем и запускаем поток выдачи данных на ЦАП
	hWriteThread = CreateThread(0, 0x2000, ServiceWriteThread, 0, 0, &WriteTid);
	if(!hWriteThread) AbortProgram(" ServiceWriteThread() --> Bad\n");
	else printf(" ServiceWriteThread() --> OK\n");

	// отобразим параметры работы модуля на экране монитора
	printf(" \n");
	printf(" Module E14-440 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     AVR Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("     LBIOS   Version    is %s (%s)\n", ModuleDescription.Dsp.Version.Version, ModuleDescription.Dsp.Version.Date);
	printf("  Adc parameters:\n");
	printf("     Data Correction is %s\n", ap.IsCorrectionEnabled ? "enabled" : "disabled");
	printf("     ChannelsQuantity = %2d\n", ap.ChannelsQuantity);
	printf("     AdcRate = %8.3f kHz\n", ap.AdcRate);
	printf("     InterKadrDelay = %2.4f ms\n", ap.InterKadrDelay);
	printf("     KadrRate = %8.3f kHz\n", ap.KadrRate);
	printf("     FIFO Length = 0x%4X(%5u)\n", ap.AdcFifoLength, ap.AdcFifoLength);
	printf("  Dac parameters:\n");
	printf("     DacRate = %8.3f kHz\n", dp.DacRate);
	printf("     FIFO Length = 0x%04X(%5u)\n", dp.DacFifoLength, dp.DacFifoLength);
	printf("  Dac Signal parameters:\n");
	printf("    Frequency = %5.3f kHz\n", SignalFrequency);
	printf("    Amplitude = %d DAC code\n\n", (WORD)SignalAmplitude);

	// ждем завершения работы обоих потоков
	printf("\n Counter %3u from %3u\r", Counter, ADC_DATA_BLOCK_QUANTITY);
	while(!IsReadThreadComplete || !IsWriteThreadComplete)
	{
		if(OldCounter != Counter) { printf(" Counter %3u from %3u\r", Counter, ADC_DATA_BLOCK_QUANTITY); OldCounter = Counter; }
		Sleep(20);
	}

	// ждём окончания работы потока ввода данных
	WaitForSingleObject(hReadThread, INFINITE);
	// ждём окончания работы потока вывода данных
	WaitForSingleObject(hWriteThread, INFINITE);

	// проверим была ли ошибка выполнения потоков ввода/вывода данных
	printf("\n\n");
	if(ReadThreadErrorNumber | WriteThreadErrorNumber) { AbortProgram(NULL, false); ShowErrorMessage(); }
	else AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------
// Поток в котором осуществляется сбор данных с АЦП
//------------------------------------------------------------------------
DWORD WINAPI ServiceReadThread(PVOID /*Context*/)
{
	WORD i;
	WORD RequestNumber;
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
		IoReq[i].Buffer = ReadBuffer + i*DataStep;
		IoReq[i].NumberOfWordsToPass = DataStep;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &ReadOv[i];
		IoReq[i].TimeOut = (DWORD)(DataStep/ap.AdcRate + 1000);
	}

	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadOv[0].hEvent); CloseHandle(ReadOv[1].hEvent); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// запустим АЦП
	if(pModule->START_ADC())
	{
		// цикл сбора данных
		for(i = 0x1; i < ADC_DATA_BLOCK_QUANTITY; i++)
		{
			// сделаем запрос на очередную порцию данных
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }
			if(ReadThreadErrorNumber) break;

			// ждём завершения операции сбора предыдущей порции данных
			if(!WaitingForReadRequestCompleted(IoReq[RequestNumber^0x1].Overlapped)) break;
//			if(WaitForSingleObject(IoReq[RequestNumber^0x1].Overlapped->hEvent, IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; break; }
			if(ReadThreadErrorNumber) break;

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

	// остановим работу АЦП
	if(!pModule->STOP_ADC()) ReadThreadErrorNumber = 0x1;
	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(ModuleHandle)) { ReadThreadErrorNumber = 0x7; }
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadOv[i].hEvent);
	// небольшая задержка
	Sleep(100);
	// установим флажок завершения работы потока сбора данных
	IsReadThreadComplete = true;
	// теперь можно спокойно выходить из потока
	return 0x0;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL WaitingForReadRequestCompleted(OVERLAPPED *ReadOv)
{
	DWORD ReadBytesTransferred;

	while(TRUE)
	{
		if(GetOverlappedResult(ModuleHandle, ReadOv, &ReadBytesTransferred, FALSE)) break;
		else if(GetLastError() !=  ERROR_IO_INCOMPLETE) { ReadThreadErrorNumber = 0x3; return FALSE; }
		else if(kbhit()) { ReadThreadErrorNumber = 0x5; return FALSE; }
		else Sleep(20);
	}
	return TRUE;
}

//------------------------------------------------------------------------
// Поток в котором осуществляется выдача данных на ЦАП
//------------------------------------------------------------------------
DWORD WINAPI ServiceWriteThread(PVOID /*Context*/)
{
	WORD RequestNumber;
	DWORD i, BaseIndex;
	// OVERLAPPED структура
	OVERLAPPED WriteOv[2];
	// массив структур с параметрами запроса на ввод/вывод данных
	IO_REQUEST_LUSBAPI IoReq[2];
	// параметры выводимого сигнала
	double CurrentTime;						// текущее время в млс

	// остановим работу ЦАП и одновременно сбросим USB-канал записи данных
	if(!pModule->STOP_DAC()) { WriteThreadErrorNumber = 0x6; IsWriteThreadComplete = true; return 0; }

	// формируем данные для целого FIFO буфера ЦАП в модуле
	CurrentTime = 0.0;
	for(i = 0; i < (DWORD)dp.DacFifoLength; i++)
	{
		WriteBuffer[i] = Round(SignalAmplitude*sin(2.0*M_PI*SignalFrequency*CurrentTime));
		WriteBuffer[i] &= (WORD)(0x0FFF);
		WriteBuffer[i] |= (WORD)(DacNumber << 12);
		CurrentTime += 1.0/dp.DacRate;
	}
/*
// !!! ОТЛАДКА !!!
for(i = 0; i < (DWORD)15; i++)
{
	WriteBuffer[i] = SignalAmplitude;
	WriteBuffer[i] &= (WORD)(0xFFF);
}
*/
	// заполняем целиком FIFO буфер ЦАП в модуле
	if(!pModule->PUT_DM_ARRAY(dp.DacFifoBaseAddress, dp.DacFifoLength, (SHORT *)WriteBuffer))
   										{ WriteThreadErrorNumber = 0x1; IsWriteThreadComplete = true; return 1; }

	// теперь формируем данные целиком для всего буфера WriteBuffer
	for(i = 0x0; i < 2*DataStep; i++)
	{
		WriteBuffer[i] = Round(SignalAmplitude*sin(2.0*M_PI*SignalFrequency*CurrentTime));
		WriteBuffer[i] &= (WORD)(0x0FFF);
		WriteBuffer[i] |= (WORD)(DacNumber << 12);
		CurrentTime += 1.0/dp.DacRate;
	}
/*
// !!! ОТЛАДКА !!!
for(i = 0; i < (DWORD)15; i++)
{
	WriteBuffer[i] = -SignalAmplitude;
	WriteBuffer[i] &= (WORD)(0xFFF);
}
*/
	// формируем структуры, необходимые для потокового вывода данных
	for(i = 0x0; i < 0x2; i++)
	{
		// инициализация структуры типа OVERLAPPED
		ZeroMemory(&WriteOv[i], sizeof(OVERLAPPED));
		// создаём событие для асинхронного запроса
		WriteOv[i].hEvent = CreateEvent(NULL, FALSE , FALSE, NULL);
		// формируем структуру IoReq
		IoReq[i].Buffer = (SHORT *)(WriteBuffer + i*DataStep);
		IoReq[i].NumberOfWordsToPass = DataStep;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &WriteOv[i];
		IoReq[i].TimeOut = (DWORD)(DataStep/dp.DacRate + 1000);
	}

	// для модулей с ревизией 'D' и выше можно делать предварительный,
	// т.е. до функции START_DAC(), запрос на вывод данных
	if(('D' <= ModuleDescription.Module.Revision) && (ModuleDescription.Module.Revision <= 'Z'))
	{
		RequestNumber = 0x0;
		if(!pModule->WriteData(&IoReq[RequestNumber])) { CloseHandle(WriteOv[0].hEvent); CloseHandle(WriteOv[1].hEvent); WriteThreadErrorNumber = 0x2; IsWriteThreadComplete = true; return 0; }
	}

	// запустим ЦАП
	if(pModule->START_DAC())
	{
		// для старых ревизий модуля (от 'A' до 'C') первый запрос на
		// вывод данных делаем ОБЯЗАТЕЛЬНО после функции START_DAC()
		if(('A' <= ModuleDescription.Module.Revision) && (ModuleDescription.Module.Revision <= 'C'))
		{
			RequestNumber = 0x0;
			if(!pModule->WriteData(&IoReq[RequestNumber])) { CloseHandle(WriteOv[0].hEvent); CloseHandle(WriteOv[1].hEvent); WriteThreadErrorNumber = 0x2; IsWriteThreadComplete = true; return 0; }
		}
		// перманентный цикл вывода данных на ЦАП
		while(!IsReadThreadComplete)
		{
			// сделаем запрос на очередную порцию передаваемых данных
			RequestNumber ^= 0x1;
			if(!pModule->WriteData(&IoReq[RequestNumber])) { WriteThreadErrorNumber = 0x2; break; }
			if(WriteThreadErrorNumber) break;

			// ждем завершения асинхронной операции записи данных в модуль
//			if(!WaitingForWriteRequestCompleted(IoReq[RequestNumber^0x1].Overlapped)) break;
			if(WaitForSingleObject(WriteOv[RequestNumber^0x1].hEvent, IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { WriteThreadErrorNumber = 0x4; break; }
			if(WriteThreadErrorNumber) break;

			// сформируем следующую порцию данных для первой половинки буфера WriteBuffer
			BaseIndex = (RequestNumber^0x1)*DataStep;
			for(i = 0x0; i < DataStep; i++)
			{
				WriteBuffer[i + BaseIndex] = Round(SignalAmplitude*sin(2.0*M_PI*SignalFrequency*CurrentTime));
				WriteBuffer[i + BaseIndex] &= (WORD)(0x0FFF);
				WriteBuffer[i + BaseIndex] |= (WORD)(DacNumber << 12);
				CurrentTime += 1.0/dp.DacRate;
	  		}

			// если поток сбора завершен, то просто выйдем
			if(WriteThreadErrorNumber) break;
			else if(IsReadThreadComplete) break;
			else if(kbhit()) { WriteThreadErrorNumber = 0x3; break; }
			else Sleep(20);
		}
	}
	else { WriteThreadErrorNumber = 0x5; }

	// остановим работу ЦАП
	if(!pModule->STOP_DAC()) WriteThreadErrorNumber = 0x6;

	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(ModuleHandle)) WriteThreadErrorNumber = 0x7;
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x2; i++) CloseHandle(IoReq[i].Overlapped->hEvent);
	// небольшая задержка
	Sleep(100);
	// установим флажок окончания потока выдачи данных
	IsWriteThreadComplete = true;
	// теперь можно спокойно выходить из потока
	return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WORD Round(double Data)
{
	if(Data > 0.0) Data += 0.5;
	else if(Data < 0.0) Data += -0.5;
	return (WORD)Data;
}

//------------------------------------------------------------------------------
// ожидание завершения выполнения очередного запроса на вывод данных
//------------------------------------------------------------------------------
BOOL WaitingForWriteRequestCompleted(OVERLAPPED * const WriteOv)
{
	DWORD BytesTransferred;

	while(true)
	{
			if(GetOverlappedResult(ModuleHandle, WriteOv, &BytesTransferred, FALSE)) break;
			else if(GetLastError() !=  ERROR_IO_INCOMPLETE) { WriteThreadErrorNumber = 0x2; return FALSE; }
			else if(IsReadThreadComplete) return FALSE;
			else Sleep(20);
	}
	return TRUE;
}

//------------------------------------------------------------------------------
// Отобразим сообщение с ошибкой
//------------------------------------------------------------------------------
void ShowErrorMessage(void)
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

	switch(WriteThreadErrorNumber)
	{
		case 0x0:
			break;

		case 0x1:
			printf(" DAC Thread: PUT_DM_ARRAY() --> Bad! :(((\n");
			break;

		case 0x2:
			printf(" DAC Thread: WriteData() --> Bad :(((\n");
			break;

		case 0x3:
			// если программа была злобно прервана, предъявим ноту протеста
			printf(" DAC Thread: The program was terminated! :(((\n");
			break;

		case 0x4:
			printf(" DAC Thread: Timeout is occured :(((\n");
			break;

		case 0x5:
			printf(" DAC Thread: START_DAC() --> Bad :(((\n");
			break;

		case 0x6:
			printf(" DAC Thread: STOP_DAC() --> Bad! :(((\n");
			break;

		case 0x7:
			printf("\n DAC Thread: Can't complete input and output (I/O) operations! :(((");
			break;

		default:
			printf(" DAC Thread: Unknown error! :(((\n");
			break;
	}

	printf("\n");

	return;
}

//------------------------------------------------------------------------
// аварийное завершение программы
//------------------------------------------------------------------------
void AbortProgram(char *ErrorString, bool AbortionFlag)
{
	SHORT DacSample;

	// подчищаем интерфейс модуля
	if(pModule)
	{
		// выставим нулевые значения на выходах обоих каналов ЦАП
		DacSample = (SHORT)(0x0);
		if(!pModule->DAC_SAMPLE(&DacSample, WORD(0x0))) printf(" DAC_SAMPLE(0) --> Bad\n");
		else if(!pModule->DAC_SAMPLE(&DacSample, WORD(0x1))) printf(" DAC_SAMPLE(1) --> Bad\n");

		// освободим интерфейс модуля
		if(!pModule->ReleaseLInstance()) printf(" ReleaseLInstance() --> Bad\n");
		else printf(" ReleaseLInstance() --> OK\n");
		// обнулим указатель на интерфейс модуля
		pModule = NULL;
	}
	// освободим память буфера ввода данных
	if(ReadBuffer) { delete[] ReadBuffer; ReadBuffer = NULL; }
	// освободим идентификатор потока сбора данных
	if(hReadThread) { CloseHandle(hReadThread); hReadThread = NULL; }
	// освободим память буфера вывода данных
	if(WriteBuffer) { delete[] WriteBuffer; WriteBuffer = NULL; }
	// освободим идентификатор потока вывода данных
	if(hWriteThread) { CloseHandle(hWriteThread); hWriteThread = NULL; }
	// освободим идентификатор файла данных
	if(hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE; }

	// выводим текст сообщения
	if(ErrorString) printf(ErrorString);

	// прочистим очередь клавиатуры
	if(kbhit()) { while(kbhit()) getch(); }

	// если нужно - аварийно завершаем программу
	if(AbortionFlag) exit(0x1);
	else return;
}

