//******************************************************************************
// Модуль E14-140.
// Консольная программа организует потоковый ввод данных одновременно
// с нескольких модулей.
//******************************************************************************
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "Lusbapi.h"

// аварийный выход из программы
void AbortProgram(char *ErrorString, bool AbortionFlag = true);
// функция потока ввода данных
DWORD WINAPI ServiceReadThread(PVOID ThreadIndex);
// ожидание завершения асинхронного запроса на ввод данных
BOOL WaitingForIoRequestCompleted(OVERLAPPED *ReadOv, DWORD ThreadTid);

// кол-во опрашиваемых виртуальных слотов
const WORD MaxVirtualSoltsQuantity	= 0x7;
// кол-во опрашиваемых каналов для каждого модуля
const WORD ADC_CHANNELS_QUANTITY 		= 0x4;
// частота работы АЦП при сборе данных
const double AdcRate = 100.0;

// идентификаторы потока
HANDLE 	ReadThreadHandle[MaxVirtualSoltsQuantity];
DWORD		ReadThreadIndex[MaxVirtualSoltsQuantity];
DWORD		ReadThreadTid[MaxVirtualSoltsQuantity];

// массив указателей на интерфейс модулей типа E14-140
ILE140 *pModules[MaxVirtualSoltsQuantity];
// версия библиотеки
DWORD DllVersion;
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
// кол-во реально обнаруженных модулей типа E14-140
WORD ModulesQuantity;

// размер запроса сбора данных
DWORD DataStep = 64*1024;
// массив буферов для получаемых данных
SHORT *Buffer[MaxVirtualSoltsQuantity];
// флажок-признак аварийного завершения соответствующего потока
bool IsReadThreadTerminated[MaxVirtualSoltsQuantity];
// номер ошибки каждого из потоков
WORD ReadThreadErrorNumber[MaxVirtualSoltsQuantity];
// номер ошибки выполнения основной программы
WORD MainErrorNumber;

// экранные координаты вывода значения счётчиков
WORD XCoordCounter, YCoordCounter;
// счетчик потоков
WORD Counter[MaxVirtualSoltsQuantity];

// критическая секция
CRITICAL_SECTION cs;

//------------------------------------------------------------------------
// основная программа
//------------------------------------------------------------------------
void main(void)
{
	WORD i, j;

	// проинициализируем указатели и переменные
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		// обнулим указатель на интерфейс
		pModules[i] = NULL;
		// сбросим флажок прерывания потока сбора данных
		IsReadThreadTerminated[i] = false;
		// пока ничего не выделено под буфер данных
		Buffer[i] = NULL;
		// пока не создан поток ввода данных
		ReadThreadHandle[i] = NULL;
		// сбросим флаг ошибок потока ввода данных
		ReadThreadErrorNumber[i] = 0x0;
	}

	// зачищаем экран монитора
	clrscr();

	printf(" ***********************************\n");
	printf(" Module E14-140                     \n");
	printf(" Console example for Multi Modules  \n");
	printf(" ***********************************\n\n");

	// инициализация критической секции
	InitializeCriticalSection(&cs);

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

	printf("\n E14-140 modules List:\n");
	// обнуляем счётчик обнаруженных модулей
	ModulesQuantity = 0x0;
	// Сканируем первые MaxVirtualSoltsQuantity виртуальных слотов в поисках модулей типа E14-140
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		// получим указатель на интерфейс
		pModules[i] = static_cast<ILE140 *>(CreateLInstance("e140"));
		if(!pModules[i]) AbortProgram(" Module Interface --> Bad\n");
		// попробуем обнаружить модуль типа E14-140 в соответствующем виртуальном слоте
		else if(!pModules[i]->OpenLDevice(i))  { pModules[i]->ReleaseLInstance(); pModules[i] = NULL; continue; }
		// прочитаем название модуля в нулевом виртуальном слоте
		else if(!pModules[i]->GetModuleName(ModuleName)) AbortProgram(" GetModuleName() --> Bad\n");
		// убедимся, что это E14-140
		else if(strcmp(ModuleName, "E140")) AbortProgram(" The detected module is not E14-140. The appication is terminated.\n");
		// попробуем узнать скорость USB для текущего модуля
		else if(!pModules[i]->GetUsbSpeed(&UsbSpeed)) AbortProgram(" GetUsbSpeed() --> Bad\n");
		// получим информацию из ППЗУ модуля
		else if(!pModules[i]->GET_MODULE_DESCRIPTION(&ModuleDescription)) AbortProgram(" GET_MODULE_DESCRIPTION() --> Bad\n");
		// проверим прошивку MCU модуля
	   else if((ModuleDescription.Module.Revision == REVISIONS_E140[REVISION_B_E140]) &&
	   	     (strtod((char *)ModuleDescription.Mcu.Version.Version, NULL) < 3.05)) AbortProgram(" For module E14-140(Rev.'B') firmware version must be 3.05 or above --> !!! ERROR !!!\n");

		// получим текущие параметры работы АЦП
		if(!pModules[i]->GET_ADC_PARS(&ap)) AbortProgram(" GET_ADC_PARS() --> Bad\n");
		// установим желаемые параметры АЦП
		ap.ClkSource = INT_ADC_CLOCK_E140;							// внутренний запуск АЦП
		ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E140; 	// без трансляции тактовых импульсо АЦП
		ap.InputMode = NO_SYNC_E140;		// без синхронизации ввода данных
		ap.ChannelsQuantity = ADC_CHANNELS_QUANTITY; 			// кол-во активных каналов
		for(j = 0x0; j < ADC_CHANNELS_QUANTITY; j++) ap.ControlTable[j] = (WORD)(j | (ADC_INPUT_RANGE_2500mV_E140 << 0x6));
		ap.AdcRate = AdcRate;								// частота работы АЦП в кГц
		ap.InterKadrDelay = 0.0;							// межкадровая задержка в мс
		// передадим требуемые параметры работы АЦП в модуль
		if(!pModules[i]->SET_ADC_PARS(&ap)) AbortProgram(" SET_ADC_PARS() --> Bad\n");

		// выделим память под двойной буфер
		Buffer[i] = new SHORT[2*DataStep];
		if(!Buffer[i]) AbortProgram(" Cannot allocate buffer memory.\n");

		// инкрементируем счётчик обнаруженных модулей
		ModulesQuantity++;
		printf("   %2u. Virtual Slot %2u. Module (S/N %s, %s) is ready ... \n", ModulesQuantity, i, ModuleDescription.Module.SerialNumber, UsbSpeed ? "HIGH-SPEED (480 Mbit/s)" : "FULL-SPEED (12 Mbit/s)");
	}

	// проверим: мы нашли хоть какой-нибудь модуль?
	if(!ModulesQuantity) { printf("   Empty...\n");  AbortProgram("\n Can't detect any E14-140 module  :(((\n"); }

	// текущие координаты курсора
	XCoordCounter = (WORD)wherex();
	YCoordCounter = (WORD)wherey(); YCoordCounter += (WORD)0x3;

	// попробуем запустить поток сбора данных для каждого из обнаруженных модуля
	MainErrorNumber = 0x0;
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		// создаем и запускаем потоки сбора данных
		if(!pModules[i]) continue;
		ReadThreadIndex[i] = i;
		ReadThreadHandle[i] = CreateThread(0, 0x2000, ServiceReadThread, &ReadThreadIndex[i], 0, &ReadThreadTid[i]);
		if(!ReadThreadHandle[i]) AbortProgram("\n CreateThread() --> Bad\n");
	}

	// ждем завершения работы потоков по факту нажатия любой клавиши клавиатуры
	printf("\n  Press any key to terminate the program ...\n\n");
	while(true)
	{
		if(kbhit()) { MainErrorNumber = 0x1; break; }
		Sleep(20);
	}

	// если программа была злобно прервана, предъявим ноту протеста
	if(MainErrorNumber == 0x1) AbortProgram(" The program was terminated successfully!\n", false);
	else AbortProgram(" The program was completed successfully!!!\n", false);
}

//------------------------------------------------------------------------
// Поток сбора данных с модуля E14-140
//------------------------------------------------------------------------
DWORD WINAPI ServiceReadThread(PVOID ThreadIndex)
{
	WORD i;
	// номер запроса
	WORD RequestNumber;
	// номер текущего потока
	DWORD ti = *(DWORD *)ThreadIndex;
	// массив OVERLAPPED структур
	OVERLAPPED ReadOv[2];
	// массив структур с параметрами запроса на ввод/вывод данных
	IO_REQUEST_LUSBAPI IoReq[2];

	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(pModules[ti]->STOP_ADC())
	{
		// формируем необходимые для сбора данных структуры
		for(i = 0x0; i < 0x2; i++)
		{
			// инициализация структуры типа OVERLAPPED
			ZeroMemory(&ReadOv[i], sizeof(OVERLAPPED));
			// создаём событие для асинхронного запроса
			ReadOv[i].hEvent = CreateEvent(NULL, FALSE , FALSE, NULL);
			// формируем структуру IoReq
			IoReq[i].Buffer = Buffer[ti] + i*DataStep;
			IoReq[i].NumberOfWordsToPass = DataStep;
			IoReq[i].NumberOfWordsPassed = 0x0;
			IoReq[i].Overlapped = &ReadOv[i];
			IoReq[i].TimeOut = DataStep/ap.AdcRate + 1000;
		}

		// координаты курсора текущего потока
		EnterCriticalSection(&cs);
			gotoxy(XCoordCounter, YCoordCounter + 1*ti);
			printf("  Counter[%2u]: %8u", ti+0x1, Counter[ti] = 0x0);
		LeaveCriticalSection(&cs);

		// делаем предварительный запрос на ввод данных
		RequestNumber = 0x0;
		if(!pModules[ti]->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber[ti] = 0x2; goto ReadThreadFinish; }

		// теперь запускаем ввод данных
		if(pModules[ti]->START_ADC())
		{
			// цикл сбора данных
			while(!IsReadThreadTerminated[ti])
			{
				// сделаем запрос на очередную порции данных
				RequestNumber ^= 0x1;
				if(!pModules[ti]->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber[ti] = 0x2; break; }
				if(IsReadThreadTerminated[ti]) break;

				// ждём завершения операции сбора предыдущей порции данных
				if(!WaitingForIoRequestCompleted(IoReq[RequestNumber^0x1].Overlapped, ti)) break;
//				if(WaitForSingleObject(ReadOv[RequestNumber^0x1].hEvent, IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { /*ReadThreadErrorNumber = 0x3;*/ break; }
				if(IsReadThreadTerminated[ti]) break;

				// инкремент счётчика текущего потока
				Counter[ti]++;
				// отобразим состояние счётчика данного потока при отсутствии ошибок
				if(!IsReadThreadTerminated[ti] && !ReadThreadErrorNumber[ti])
				{
					EnterCriticalSection(&cs);
						gotoxy(XCoordCounter, YCoordCounter + 1*ti);
						printf("  Counter[%2u]: %8u", ti+0x1, Counter[ti]);
					LeaveCriticalSection(&cs);
				}

				// была ли какая-нибудь ошибка в работе данного потока
				if(IsReadThreadTerminated[ti]) break;
				else if(ReadThreadErrorNumber[ti]) break;
				else Sleep(20);
			}
		}
		else ReadThreadErrorNumber[ti] = 0x4;
	}
	else ReadThreadErrorNumber[ti] = 0x6;

ReadThreadFinish:
	// остановим ввод данных
	if(!pModules[ti]->STOP_ADC()) ReadThreadErrorNumber[ti] = 0x6;
	// прервём, если нужно, асинхронный запрос
	if(!CancelIo(pModules[ti]->GetModuleHandle())) ReadThreadErrorNumber[ti] = 0x7;
	// освободим все идентификаторы событий
	for(WORD i = 0x0; i < 0x2; i++) CloseHandle(IoReq[i].Overlapped->hEvent);
	// отобразим состояние счётчика данного потока при наличии ошибок
	if(ReadThreadErrorNumber[ti])
	{
		EnterCriticalSection(&cs);
			gotoxy(XCoordCounter, YCoordCounter + 1*ti);
			printf("  Counter[%2u]: Thread Error!", ti+0x1, Counter[ti]);
		LeaveCriticalSection(&cs);
	}
	// теперь спокойно можно выйти из потока
	return 0x0;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL WaitingForIoRequestCompleted(OVERLAPPED *ReadOv, DWORD ThreadTid)
{
	// ждём завершения очередного запроса
	while(!IsReadThreadTerminated[ThreadTid])
	{
		if(HasOverlappedIoCompleted(ReadOv)) break;
		else if(IsReadThreadTerminated[ThreadTid]) break;
		else Sleep(20);
	}
	return TRUE;
}

//------------------------------------------------------------------------
// вывод сообщения об ошибке
//------------------------------------------------------------------------
void AbortProgram(char *ErrorString, bool AbortionFlag)
{
	WORD i;

	// подчистим за собой
	for(i = 0x0; i < MaxVirtualSoltsQuantity; i++)
	{
		if(ReadThreadHandle[i] != NULL)
		{
			// прервем данный поток
			IsReadThreadTerminated[i] = true;
			// ждём окончания работы данного потока
			WaitForSingleObject(ReadThreadHandle[i], INFINITE);
			// освободим идентификатор данного потока
			CloseHandle(ReadThreadHandle[i]);
		}
		// освободим память буфера
		if(Buffer[i]) { delete[] Buffer[i]; Buffer[i] = NULL; }
		// освободим модуль
		if(pModules[i]) { pModules[i]->ReleaseLInstance(); pModules[i] = NULL; }
	}

	// освободим критической секции
	DeleteCriticalSection(&cs);

	// выставим положение курсора
	if(ModulesQuantity) gotoxy(XCoordCounter, YCoordCounter + ModulesQuantity + 0x2);
	else printf("\n\n");

	// выводим текст сообщения
	if(ErrorString) printf(ErrorString);

	// если нужно - аварийно завершаем программу
	if(AbortionFlag) exit(0x1);
	else return;
}

