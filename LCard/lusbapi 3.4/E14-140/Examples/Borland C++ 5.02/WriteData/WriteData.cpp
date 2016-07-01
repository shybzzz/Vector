//******************************************************************************
// Модуль E14-140.
// Консольная программа с организацией потокового вывода данных на ЦАП.
// Выводится синусоидальный сигнал на первый канал ЦАП. Чатота вывода 125 кГц.
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);
// функция потока вывода данных на ЦАП
DWORD WINAPI ServiceWriteThread(PVOID /*Context*/);
// ожидание завершения асинхронного запроса на вывод данных
BOOL WaitingForRequestCompleted(OVERLAPPED *ReadOv);
// функция вывода сообщений с ошибками
void ShowErrorMessage(void);
// округление
WORD Round(double Data);

// идентификатор потока
HANDLE 	hWriteThread;
DWORD 	WriteTid;

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
// структура параметров работы ЦАП
DAC_PARS_E140 dp;

// кол-во передаваемых отсчетов (кратное 32) для Ф. WriteData()
DWORD DataStep = 128*1024;
// буфер данных
SHORT *WriteBuffer;
// номер канала ЦАП, на который будет выводиться сигнал
WORD DacNumber = 0x0;
// параметры выводимого сигнала
double SignalFrequency = 1.2;	  		// частота сигнала в кГц
WORD SignalAmplitude = 32000;      	// амплитуда сигнала в кодах ЦАП

// флажок завершения работы потока вывода данных на ЦАП
bool IsWriteThreadComplete;
// номер ошибки при выполнении потока вывода данных на ЦАП
WORD WriteThreadErrorNumber;

// экранный счетчик-индикатор
DWORD Counter;

//------------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------------
void main(void)
{
	WORD i;

	// пока ничего не выделено под буфер данных
	WriteBuffer = NULL;
	// пока не создан поток ввода данных
	hWriteThread = NULL;
	// сбросим флаг ошибок потока ввода данных
	WriteThreadErrorNumber = 0x0;

	// зачистим экран монитора
	clrscr();

	printf(" **************************************\n");
	printf(" Module E14-140                        \n");
	printf(" Console example for E14-140 DAC stream\n");
	printf(" **************************************\n\n");


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

	// попробуем обнаружить модуль E14-140 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
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

	// проверим ревизию модуля 
   if(ModuleDescription.Module.Revision == 'A') AbortProgram(" Module E14-140 Rev.'A' dosn't support DAC stream!\n");
	// проверим есть ли на модуле ЦАП
	else if(ModuleDescription.Dac.Active == DAC_INACCESSIBLED_E140) AbortProgram(" DAC is absent on this module E14-140!\n");

	// получим текущие параметры работы ЦАП
	if(!pModule->GET_DAC_PARS(&dp)) AbortProgram(" GET_CUR_DAC_PARS() --> Bad\n");
	else printf(" GET_CUR_DAC_PARS() --> OK\n");
	// установим желаемые параметры ЦАП
	dp.DacRate = 200.0;									// частота работы ЦАП в кГц
	if(!pModule->SET_DAC_PARS(&dp)) AbortProgram(" SET_DAC_PARS() --> Bad\n");
	else printf(" SET_DAC_PARS() --> OK\n");

	// попробуем выделить память под буфер ЦАП
	WriteBuffer = new SHORT[2*DataStep];
	if(!WriteBuffer) AbortProgram(" Cannot allocate WriteBuffer memory\n");

	// Создаем и запускаем поток выдачи данных на ЦАП
	printf(" \n");
	hWriteThread = CreateThread(0, 0x2000, ServiceWriteThread, 0, 0, &WriteTid);
	if(!hWriteThread) AbortProgram(" ServiceWriteThread() --> Bad\n");
	else printf(" ServiceWriteThread() --> OK\n");

	// отобразим параметры ЦАП на дисплее
	printf(" \n");
	printf(" Module E14-140 (S/N %s) is ready ... \n", ModuleDescription.Module.SerialNumber);
	printf("   Module Info:\n");
	printf("     Module  Revision   is '%c'\n", ModuleDescription.Module.Revision);
	printf("     MCU Driver Version is %s (%s)\n", ModuleDescription.Mcu.Version.Version, ModuleDescription.Mcu.Version.Date);
	printf("  Dac parameters:\n");
	printf("    DacRate = %7.2f kHz\n", dp.DacRate);
	printf("  Signal parameters:\n");
	printf("    Frequency = %5.3f kHz\n", SignalFrequency);
	printf("    Amplitude = %d DAC code\n", SignalAmplitude);
	printf("\n Now SINUS and -SINUS signals are on the DAC channels ... \n");
	printf("  (Press any key to terminate the program)\n");

	// цикл ожидания окончания работы приложения по нажатию любой клавиши
	printf("\n Time: %8lu s\r", Counter);
	while(!IsWriteThreadComplete)
	{
		Sleep(1000);
		printf(" Time: %8lu s\r", ++Counter);
	}

	// ждём окончания работы потока ввода данных
	WaitForSingleObject(hWriteThread, INFINITE);

	// проверим была ли ошибка выполнения потока сбора данных
	printf("\n\n");
	if(WriteThreadErrorNumber) { AbortProgram(NULL, false); ShowErrorMessage(); }
	else AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------------
// Поток в котором осуществляется выдача данных на ЦАП
//------------------------------------------------------------------------------
DWORD WINAPI ServiceWriteThread(PVOID /*Context*/)
{
	WORD RequestNumber;
	DWORD i, BaseIndex;
	// параметры выводимого сигнала
	double CurrentTime;						// текущее время в млс
	// OVERLAPPED структура
	OVERLAPPED WriteOv[2];
	// массив структур с параметрами запроса на ввод/вывод данных
	IO_REQUEST_LUSBAPI IoReq[2];

	// остановим работу ЦАП и одновременно сбросим USB-канал записи данных
	if(!pModule->STOP_DAC()) { WriteThreadErrorNumber = 0x6; IsWriteThreadComplete = true; return 0; }

	// формируем данные целиком для всего буфера WriteBuffer
	CurrentTime = 0.0;
	for(i = 0x0; i < 2*DataStep; i += 2)
	{
		WriteBuffer[i + 0x0] = Round(SignalAmplitude*sin(2.0*M_PI*SignalFrequency*CurrentTime));
		WriteBuffer[i + 0x1] = SHORT(-WriteBuffer[i + 0x0]);
		CurrentTime += 1.0/dp.DacRate;
	}

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
		IoReq[i].TimeOut = DataStep/dp.DacRate + 1000;
	}

	// теперь можно сделать предварительный,
	// т.е. до функции START_DAC(), запрос на вывод данных
	RequestNumber = 0x0;
	if(!pModule->WriteData(&IoReq[RequestNumber])) { CloseHandle(WriteOv[0].hEvent); CloseHandle(WriteOv[1].hEvent); WriteThreadErrorNumber = 0x2; IsWriteThreadComplete = true; return 0; }

	// запустим ЦАП
	if(pModule->START_DAC())
	{
		// перманентный цикл вывода данных на ЦАП
		while(true)
		{
			// сделаем запрос на очередную порцию передаваемых данных
			RequestNumber ^= 0x1;
			if(!pModule->WriteData(&IoReq[RequestNumber])) { WriteThreadErrorNumber = 0x2; break; }
			if(WriteThreadErrorNumber) break;

			// ждем завершения асинхронной операции записи данных в модуль
			if(!WaitingForRequestCompleted(IoReq[RequestNumber^0x1].Overlapped)) break;
//			if(WaitForSingleObject(WriteOv[RequestNumber^0x1].hEvent, IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { WriteThreadErrorNumber = 0x4; break; }
			if(WriteThreadErrorNumber) break;

			// сформируем следующую порцию данных для первой половинки буфера WriteBuffer
			BaseIndex = (RequestNumber^0x1)*DataStep;
			for(i = 0x0; i < DataStep; i += 0x2)
			{
				WriteBuffer[i + BaseIndex + 0x0] = Round(SignalAmplitude*sin(2.*M_PI*SignalFrequency*CurrentTime));
				WriteBuffer[i + BaseIndex + 0x1] = SHORT(-WriteBuffer[i + BaseIndex + 0x0]);
				CurrentTime += 1.0/dp.DacRate;
	  		}

			// если поток сбора завершен, то просто выйдем
			if(WriteThreadErrorNumber) break;
			if(kbhit()) { WriteThreadErrorNumber = 0x3; break; }
			else Sleep(20);
		}
	}
	else { WriteThreadErrorNumber = 0x5; }

	// остановим работу ЦАП
	if(!pModule->STOP_DAC()) WriteThreadErrorNumber = 0x6;
	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(ModuleHandle)) WriteThreadErrorNumber = 0x7;
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x2; i++) CloseHandle(WriteOv[i].hEvent);
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

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL WaitingForRequestCompleted(OVERLAPPED *ReadOv)
{
	DWORD ReadBytesTransferred;

	while(TRUE)
	{
		if(GetOverlappedResult(ModuleHandle, ReadOv, &ReadBytesTransferred, FALSE)) break;
		else if(GetLastError() !=  ERROR_IO_INCOMPLETE) { WriteThreadErrorNumber = 0x4; return FALSE; }
		else if(kbhit()) { WriteThreadErrorNumber = 0x3; return FALSE; }
		else Sleep(20);
	}
	return TRUE;
}

//------------------------------------------------------------------------------
// Отобразим сообщение с ошибкой
//------------------------------------------------------------------------------
void ShowErrorMessage(void)
{
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

//------------------------------------------------------------------------------
// аварийное завершение программы
//------------------------------------------------------------------------------
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

	// освободим память буфера
	if(WriteBuffer) { delete[] WriteBuffer; WriteBuffer = NULL; }
	// освободим идентификатор потока сбора данных
	if(hWriteThread) { CloseHandle(hWriteThread); hWriteThread = NULL; }

	// выводим текст сообщения
	if(ErrorString) printf(ErrorString);

	// прочистим очередь клавиатуры
	if(kbhit()) { while(kbhit()) getch(); }

	// если нужно - аварийно завершаем программу
	if(AbortionFlag) exit(0x1);
	// или спокойно выходим из функции
	else return;
}

