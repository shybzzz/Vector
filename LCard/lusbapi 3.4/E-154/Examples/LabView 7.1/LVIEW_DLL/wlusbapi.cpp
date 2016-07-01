#include <windows.h>
#include "lusbapi.h"
#include "wlusbapi.h"
#pragma argsused

static ILE154 *pModule;
// дескриптор устройства
static HANDLE ModuleHandle;
// название модуля
static char ModuleName[7];
// скорость работы шины USB
static BYTE UsbSpeed;
// структура с полной информацией о модуле
static MODULE_DESCRIPTION_E154 ModuleDescription;
// структура параметров работы АЦП модуля
static ADC_PARS_E154 ap;

DWORD WINAPI ServiceRealTImeReadThread(PVOID /*Context*/);

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
    return 1;
}
//------------------------------------------------------------------------
// возвращает текущую версию библиотеки
//------------------------------------------------------------------------
__declspec(dllexport) DWORD WINAPI WE154GetDllVersion(VOID)
{
        return GetDllVersion();
}
//------------------------------------------------------------------------
// возвращает текущую версию библиотеки
//------------------------------------------------------------------------
void BadExit(void);

__declspec(dllexport) DWORD WINAPI LV_OpenE154(VOID)
{
int i;

	BadExit();

	// попробуем получить указатель на интерфейс
	pModule = static_cast<ILE154 *>(CreateLInstance("e154"));
	if(!pModule) { BadExit(); return 0; }

	// попробуем обнаружить модуль E14-154 в первых WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// что-нибудь обнаружили?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) { BadExit(); return 0; }

	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) { BadExit(); return 0; }

	// прочитаем название модуля в обнаруженном виртуальном слоте
	if(!pModule->GetModuleName(ModuleName)) { BadExit(); return 0; }

	// проверим, что это 'E14-154'
	if(strcmp(ModuleName, "E154")) { BadExit(); return 0; }

	// попробуем получить скорость работы шины USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) { BadExit(); return 0; }

	// получим информацию из ППЗУ модуля
	if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { BadExit(); return 0; }

   return 1;
}

__declspec(dllexport) DWORD WINAPI LV_CloseE154(VOID)
{
BadExit();
return 0;
}

void BadExit(void)
{
	// подчищаем интерфейс модуля
	if(pModule)
	{
		// освободим интерфейс модуля
		pModule->ReleaseLInstance();

		// обнулим указатель на интерфейс модуля
		pModule = NULL;
	}
}
// -----------------------------------------------------------------------------
// ---------------------------------- E154 -------------------------------------
// -----------------------------------------------------------------------------
//------------------------------------------------------------------------
// создание экземпляра необходимого класса
//------------------------------------------------------------------------
__declspec(dllexport) LPVOID WINAPI WE154CreateInstance(VOID)
{
    return CreateLInstance("E154");
}
//------------------------------------------------------------------------
//  возвращается дескриптор устройства
//------------------------------------------------------------------------
__declspec(dllexport) HANDLE WINAPI WE154GetModuleHandle(LPVOID lptr)
{
    return ((ILE154*)lptr)->GetModuleHandle();
}
//------------------------------------------------------------------------
//  Откроем виртуальный слот для доступа к USB модулю
//------------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154OpenLDevice(LPVOID lptr, WORD VirtualSlot)
{
    return (BOOL)((ILE154*)lptr)->OpenLDevice(VirtualSlot);
}
//------------------------------------------------------------------------
//  Освободим текущий виртуальный слот
//------------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154CloseLDevice(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->CloseLDevice();
}
// -----------------------------------------------------------------------
//  Освободим указатель на устройство
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ReleaseLDevice(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->ReleaseLInstance();
}
// -----------------------------------------------------------------------
//  Чтение название модуля
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154GetModuleName(LPVOID lptr, BYTE *ModuleName)
{
    return (BOOL)((ILE154*)lptr)->GetModuleName(ModuleName);
}
// -----------------------------------------------------------------------
//  функция выдачи строки с последней ошибкой выполнения интерфейсной функции
// -----------------------------------------------------------------------
/*__declspec(dllexport) INT WINAPI WE154GetLastErrorString(LPVOID lptr, LPTSTR lpBuffer, DWORD nSize)
{
    return ((ILE154*)lptr)->GetLastErrorString(lpBuffer, nSize);
} */
// -----------------------------------------------------------------------
//  возвращает версию биоса
// -----------------------------------------------------------------------
/*__declspec(dllexport) BOOL WINAPI WE154GET_LBIOS_VERSION(LPVOID lptr, DWORD *LbiosVersion)
{
    return (BOOL)((ILE154*)lptr)->GET_LBIOS_VERSION(LbiosVersion);
} */
// -----------------------------------------------------------------------
// получение текущих параметров работы АЦП
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154GET_CUR_ADC_PARS(LPVOID lptr, ADC_PARS_E154 *wam)
{
 if(((ILE154*)lptr)->GET_ADC_PARS(wam)) {
    return TRUE;
  } else return FALSE;
}
// -----------------------------------------------------------------------
//  заполнение требуемых параметров работы АЦП
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154FILL_ADC_PARS(LPVOID lptr, ADC_PARS_E154 *wam)
{
 if(((ILE154*)lptr)->SET_ADC_PARS(wam)) {
    return TRUE;
  } else return FALSE;
}
// -----------------------------------------------------------------------
//  запуск работы АЦП
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154START_ADC(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->START_ADC();
}
// -----------------------------------------------------------------------
//  Останов АЦП
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154STOP_ADC(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->STOP_ADC();
}
// -----------------------------------------------------------------------
//  ввод кадра отсчетов с АЦП модуля
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ADC_KADR(LPVOID lptr, SHORT *Data)
{
    return (BOOL)((ILE154*)lptr)->ADC_KADR(Data);
}
// -----------------------------------------------------------------------
//  однократный ввод с заданного логического канала АЦП модуля
// -----------------------------------------------------------------------
static int test;
__declspec(dllexport) double WINAPI LV_SAMPLE_E154(DWORD Channel, DWORD UseCalibr, DWORD *error)
{
double f=0.0;
SHORT adc;
BOOL result;

result=pModule->ADC_SAMPLE(&adc, Channel);
*error=(result == TRUE) ? 1 : 0;
if(result == FALSE) return f;

// f=(result == TRUE) ? adc : 0;
// f = f*ADC_INPUT_RANGES_E154[(Channel >> 6) & 3]/2048.;
result=pModule->ProcessOnePoint(adc, &f, Channel, UseCalibr, TRUE);
*error=(result == TRUE) ? 1 : 0;

return f;
}
// -----------------------------------------------------------------------
//  потоковое чтение данных с АЦП модуля
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ReadData(LPVOID lptr, IO_REQUEST_LUSBAPI *ReadRequest)
{

    return (BOOL)((ILE154*)lptr)->ReadData(ReadRequest);
}
// -----------------------------------------------------------------------
//  однократный вывод на заданный канал ЦАП
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI LV_OUTDA_E154(double value)
{
short val;

val=value*128./5.12;
    return (BOOL)pModule->DAC_SAMPLE(&val, 0);
}
// -----------------------------------------------------------------------
//  функция разрешения выходных линий внешнего цифрового разъёма
// -----------------------------------------------------------------------
static int TtlOutEnabled;
__declspec(dllexport) BOOL WINAPI WE154ENABLE_TTL_OUT(LPVOID lptr, BOOL flag)
{
    return (BOOL)((ILE154*)lptr)->ENABLE_TTL_OUT(flag ? true : false);
}
// -----------------------------------------------------------------------
//   функция чтения входных линии внешнего цифрового разъёма
// -----------------------------------------------------------------------
__declspec(dllexport) DWORD WINAPI LV_GETTTL_E154(DWORD *error)
{
BOOL result;
unsigned short ttl;

result=pModule->TTL_IN(&ttl);
if(result == FALSE)
	{
   *error=0;
   return 0;
   }

*error=1;
return (DWORD) ttl;
}
// -----------------------------------------------------------------------
//  функция вывода на выходные линии внешнего цифрового разъёма
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI LV_SETTTL_E154(DWORD TtlOut)
{
BOOL result;

if(!TtlOutEnabled)
	{
   if(!pModule->ENABLE_TTL_OUT(true)) { return 0; }
   TtlOutEnabled=1;
   }

return (BOOL) pModule->TTL_OUT(TtlOut);
}
// -----------------------------------------------------------------------
//  разрешение/запрещение режима записи в ППЗУ модуля
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ENABLE_FLASH_WRITE(LPVOID lptr, BOOL EnableFlashWrite)
{
    return (BOOL)((ILE154*)lptr)->ENABLE_FLASH_WRITE(EnableFlashWrite ? true : false);
}
// -----------------------------------------------------------------------
//  чтенние слова из ППЗУ
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154D_FLASH_ARRAY(LPVOID lptr, unsigned char *ptr)
{
    return (BOOL)((ILE154*)lptr)->WRITE_FLASH_ARRAY((unsigned char *)ptr);
}
// -----------------------------------------------------------------------
//  запись слова в ППЗУ
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI  WEREAD_FLASH_ARRAY(LPVOID lptr, unsigned char *ptr)
{
    return (BOOL)((ILE154*)lptr)->READ_FLASH_ARRAY((unsigned char *) ptr);
}
// -----------------------------------------------------------------------
//  получим служебную информацию о модуле из ППЗУ
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154GET_MODULE_DESCR(LPVOID lptr, MODULE_DESCRIPTION_E154 *wmd)
{
 if(((ILE154*)lptr)->GET_MODULE_DESCRIPTION(wmd)) {
    return TRUE;
 } else return FALSE;
}
// -----------------------------------------------------------------------
//  запишем служебную информацию о модуле в ППЗУ
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154SAVE_MODULE_DESCR(LPVOID lptr, MODULE_DESCRIPTION_E154 *wmd)
{
 return (BOOL)((ILE154*)lptr)->SAVE_MODULE_DESCRIPTION(wmd);
}
// -----------------------------------------------------------------------
//             состояние аналогового питания узлов модуля
//               при переходе шины usb в режим suspend
// -----------------------------------------------------------------------
/*__declspec(dllexport) BOOL WINAPI WE154SetSuspendModeFlag(LPVOID lptr, BOOL SuspendModeFlag)
{
    return (BOOL)((ILE154*)lptr)->SetSuspendModeFlag(SuspendModeFlag ? true : false);
}
  */

//------------------------------------------------------------------------
// возвращает текущую версию библиотеки
//------------------------------------------------------------------------
__declspec(dllexport) DWORD WINAPI LV_CreateChannelE154(DWORD channel, DWORD gain)
{
return (channel & 7) | ((gain & 3) << 6);
}


// -----------------------------------------------------------------------
//  установка параметров синхронного ввода с АВЦП
// -----------------------------------------------------------------------
// Nch - число вводимых каналов (от 1 до 128)
// Channels - массив с номерами каналов
// Rate частота опроса каналов АЦП в Гц
// AdcDelay - время преобразования АЦП (межканальная задержка), в мс
// InputMode, SynchroAdType, SynchroAdMode, SynchroAdChanne параметры синхронизации (согласно описанию)
// SynchroAdPorog порог сихронизации в вольтах
// SetRate реально установленнрая частота опроса каналов АЦП в Гц
// SetAdcDelay реально установленная межканальная задержка (мс)
__declspec(dllexport) BOOL WINAPI LV_SET_ADC_SYCHRO_READ_PARAMETERS(DWORD Nch, DWORD *Channels,
	float Rate, float AdcDelay, DWORD InputMode, DWORD SynchroAdType, DWORD SynchroAdMode, DWORD SynchroAdChannel,
   float SynchroAdPorog, float *SetRate, float *SetAdcDelay)
{
BOOL result;
double f;
int i;
double adc_ms, rate_ms, inter_kadr_ms;

if(!Nch) return false;

// получим текущие параметры работы АЦП
if(!pModule->GET_ADC_PARS(&ap)) return false;

// установим желаемые параметры АЦП
ap.ClkSource = INT_ADC_CLOCK_E154;							// внутренний запуск АЦП
ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// без трансляции тактовых импульсо АЦП
ap.InputMode = InputMode;
ap.SynchroAdType=SynchroAdType;
ap.SynchroAdMode=SynchroAdMode;
ap.SynchroAdChannel=SynchroAdChannel;
ap.SynchroAdPorog=SynchroAdPorog*2048./ADC_INPUT_RANGES_E154[(SynchroAdChannel >> 6) & 3];

ap.ChannelsQuantity = Nch; 		// кол-во активных канала
for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)Channels[i];

if(AdcDelay < 0.007) ap.AdcRate=120.;
  else ap.AdcRate = 1./AdcDelay;					// частота работы АЦП в кГц

// переведем все в мс
if(Rate > 120000) Rate=120000;
adc_ms=1./ap.AdcRate;
rate_ms=1000./Rate;
inter_kadr_ms=rate_ms-(Nch-1)*adc_ms;
if(inter_kadr_ms < 0) inter_kadr_ms=0;
ap.InterKadrDelay = inter_kadr_ms;			// межкадровая задержка в мс

// передадим требуемые параметры работы АЦП в модуль
if(!pModule->SET_ADC_PARS(&ap)) return false;

if(!pModule->GET_ADC_PARS(&ap)) return false;

*SetAdcDelay=1./ap.AdcRate;
*SetRate=ap.KadrRate*1000.;

return true;
}

DWORD WINAPI ServiceReadThread(PVOID /*Context*/);
// функция вывода сообщений с ошибками
void ShowThreadErrorMessage(void);

// идентификатор потока сбора данных
HANDLE hReadThread;
DWORD ReadTid;


// кол-во получаемых отсчетов (кратное 32) для Ф. ReadData()
DWORD DataStep = 32*1024;	// будем получать данные по 64 кБ

// буфер данных
SHORT *AdcBuffer, *GlobalPtr;

// флажок завершения работы потока сбора данных
bool IsReadThreadComplete;
// номер ошибки при выполнении сбора данных
WORD ReadThreadErrorNumber;

DWORD DataReadN, DataReadAll;;

// экранный счетчик-индикатор
DWORD Counter = 0x0, OldCounter = 0xFFFFFFFF;

// NPoints - число отсчетов
// Data - целочисленный массив с кодами АЦП
// TimeOut	таймаут мс, по завершению которого функция закончит выполнения даже если данные не были введены (при этом усановится признак ошибки)
// result 1 все закончилось хорошо, 0 плохо
__declspec(dllexport) BOOL WINAPI LV_GET_ADC_SYNCHRO_ARRAY(DWORD NPoints, SHORT *Data, DWORD TimeOut)
{
double f;
int i;
int n_wait, n;
   // если данных надо не много то обойдемся одним запросом
	WORD RequestNumber;
	// идентификатор массива их двух событий
	HANDLE ReadEvent[2];
	// массив OVERLAPPED структур из двух элементов
	OVERLAPPED ReadOv[2];
	IO_REQUEST_LUSBAPI IoReq[2];

if(!ap.ChannelsQuantity ) return false;
if(!NPoints) return false;

if(!TimeOut) n_wait=250;	// 5 sec
  else n_wait=TimeOut/20;

// сбросим флажок завершения потока ввода данных
IsReadThreadComplete = false;

// пока ничего не выделено под буфер данных
AdcBuffer = NULL;

// пока не создан поток ввода данных
hReadThread = NULL;

// сбросим флаг ошибок потока ввода данных
ReadThreadErrorNumber = 0x0;

DataReadN=0;
DataReadAll=NPoints*ap.ChannelsQuantity;
GlobalPtr=Data;

if(DataReadAll < 128*1024)
	{
   // получим число точек кратное 64 байтам (32 слова)
   n=DataReadAll/32;
   if(DataReadAll % 32) n++;
   n *= 32;

	AdcBuffer = new SHORT[n];
	if(!AdcBuffer) return false;

	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(!pModule->STOP_ADC()) { free(AdcBuffer); return false; }

	// формируем необходимые для сбора данных структуры
	// создаём событие
	ReadEvent[0] = CreateEvent(NULL, FALSE , FALSE, NULL);
	// инициализация структуры типа OVERLAPPED
	ZeroMemory(&ReadOv[0], sizeof(OVERLAPPED)); ReadOv[0].hEvent = ReadEvent[0];
	// формируем структуру IoReq
	IoReq[0].Buffer = AdcBuffer;
	IoReq[0].NumberOfWordsToPass = n;
	IoReq[0].NumberOfWordsPassed = 0x0;
	IoReq[0].Overlapped = &ReadOv[0];
	IoReq[0].TimeOut = n/ap.AdcRate + 10000;

	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]);  return 0x0; }

	// запустим АЦП
	if(pModule->START_ADC())
	{
	// ждём завершения операции сбора предыдущей порции данных
	if(WaitForSingleObject(ReadEvent[0], IoReq[0].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; }
     else
       memcpy(Data, AdcBuffer, DataReadAll*2);
	}
	else { ReadThreadErrorNumber = 0x6; }

	// остановим работу АЦП
	if(!pModule->STOP_ADC()) ReadThreadErrorNumber = 0x1;
	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(pModule->GetModuleHandle())) { ReadThreadErrorNumber = 0x7; }
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadEvent[i]);
	// небольшая задержка
  //	Sleep(100);
	// установим флажок завершения работы потока сбора данных
	// теперь можно спокойно выходить из потока
   free(AdcBuffer);

   return (ReadThreadErrorNumber) ? false :  true;
   }

// выделим память под буфер
AdcBuffer = new SHORT[2*DataStep];
if(!AdcBuffer) return false;

// Создаём и запускаем поток сбора данных
hReadThread = CreateThread(0, 0x2000, ServiceReadThread, 0, 0, &ReadTid);
if(!hReadThread) { free(AdcBuffer); return false; }


// цикл записи получаемых данных и ожидания окончания работы приложения
for(i=0; i < n_wait && !IsReadThreadComplete; i++)
	{
		//if(OldCounter != Counter) { printf(" Counter %3u from %3u\r", Counter, NDataBlock); OldCounter = Counter; }
		//else
      Sleep(20);
	}

// ждём окончания работы потока ввода данных
WaitForSingleObject(hReadThread, INFINITE);

if(ReadThreadErrorNumber) { free(AdcBuffer); return false; }

free(AdcBuffer);

return true;
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
		IoReq[i].TimeOut = DataStep/ap.AdcRate + 10000;
	}

	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// запустим АЦП
	if(pModule->START_ADC())
	{
		// цикл сбора данных
		for(;;)
		{
			// сделаем запрос на очередную порции данных
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }

			// ждём завершения операции сбора предыдущей порции данных
			if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; break; }

         if((DataReadAll-DataReadN) < DataStep)
         	{
            // последняя порция данных
            memcpy(&GlobalPtr[DataReadN], IoReq[RequestNumber^0x1].Buffer, 2*(DataReadAll-DataReadN));
            break;
            }
           else
           	{
            memcpy(&GlobalPtr[DataReadAll], IoReq[RequestNumber^0x1].Buffer, 2*DataStep);
            DataReadN += DataStep;
            }
			// запишем полученную порцию данных в файл
			if(ReadThreadErrorNumber) break;
//			else if(kbhit()) { ReadThreadErrorNumber = 0x5; break; }
			else Sleep(20);
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


int GlobalPacketSize;
SHORT *RealTimePtr;
int RealTimeReadIndex;
DWORD RealBufferSize;
static int StopEvent;
static int OverflowFlag;
static int UserIndex;

// BufferSize - размер буфера в дву-байтных словах, в который будут помещаться данные по кольцу (может быть хоть 1 Гб, лишь бы ОЗУ хватило)
// PacketSize - размер одного запроса к плате, фактически в главный буфер данные будут приходить порциями по PacketSize слов
// Данные записываются в FIFO размером RealBufferSize пачками по  GlobalPacketSize
// текущее положение указателя записи - RealTimeReadIndex
// текущее положения указателя считывания  UserIndex
__declspec(dllexport) BOOL WINAPI LV_START_REAL_TIME_SAMPLING(DWORD BufferSize, DWORD PacketSize)
{
double f;
int i;
int n_wait, n;

// если данных надо не много то обойдемся одним запросом
WORD RequestNumber;

// идентификатор массива их двух событий
HANDLE ReadEvent[2];

// массив OVERLAPPED структур из двух элементов
OVERLAPPED ReadOv[2];
IO_REQUEST_LUSBAPI IoReq[2];

if(!ap.ChannelsQuantity ) return false;
if(BufferSize < 1024  || (BufferSize & 1)) return false;
if(PacketSize > BufferSize/2) PacketSize=BufferSize/2;
if(PacketSize < 32) PacketSize=32;
if(PacketSize % 32) { PacketSize=(PacketSize/32); PacketSize=(PacketSize+1)*32; }

// сбросим флажок завершения потока ввода данных
IsReadThreadComplete = false;

// пока ничего не выделено под буфер данных
RealTimeReadIndex=0;
StopEvent=0;
GlobalPacketSize=PacketSize;
RealBufferSize=BufferSize;
AdcBuffer = NULL;
UserIndex=0;

// пока не создан поток ввода данных
hReadThread = NULL;

// сбросим флаг ошибок потока ввода данных
ReadThreadErrorNumber = 0x0;

// выделим память под буфер
AdcBuffer = new SHORT[2*PacketSize];
if(!AdcBuffer) return false;

RealTimePtr = new SHORT[RealBufferSize];
if(!RealTimePtr) { free(AdcBuffer); return false; }

// Создаём и запускаем поток сбора данных
hReadThread = CreateThread(0, 0x2000, ServiceRealTImeReadThread, 0, 0, &ReadTid);
//SetThreadPriority(hReadThread,  THREAD_PRIORITY_TIME_CRITICAL);
if(!hReadThread) { free(AdcBuffer); return false; }

if(ReadThreadErrorNumber) return false;

return true;
}

__declspec(dllexport) BOOL WINAPI LV_STOP_REAL_TIME_SAMPLING(void)
{
StopEvent=1;

for(;!IsReadThreadComplete;)
	{
      Sleep(20);
	}

// ждём окончания работы потока ввода данных
WaitForSingleObject(hReadThread, INFINITE);

free(AdcBuffer); free(RealTimePtr);
AdcBuffer=NULL;
RealTimePtr=NULL;

if(ReadThreadErrorNumber) return false;

return true;
}


// index возвращает текущее положение заполненности буфера
// overflow_flag ненулевое значение означает, что произошло переполнение буфера
__declspec(dllexport) BOOL WINAPI LV_GET_REAL_TIME_INDEX(DWORD *index, DWORD *overflow_flag, DWORD *user_index)
{

*overflow_flag=OverflowFlag;
*index=RealTimeReadIndex;
*user_index=UserIndex;

if(ReadThreadErrorNumber) return false;

return true;
}

static int cnt=0;

// Data массив в который будут переложены данные
// NPoints сколько отсчетов перекладывать
__declspec(dllexport) BOOL WINAPI LV_GET_REAL_TIME_DATA(SHORT *Data, DWORD NPoints)
{
DWORD n;
DWORD i;


// Данные записываются в FIFO размером RealBufferSize пачками по  GlobalPacketSize
// текущее положение указателя записи - RealTimeReadIndex
// текущее положения указателя считывания  UserIndex
if((UserIndex+NPoints) <= RealBufferSize)
         	{
            memcpy(Data, &RealTimePtr[UserIndex], 2*NPoints);
            UserIndex += NPoints;
            if(UserIndex >= RealBufferSize) UserIndex=0;
//            for(i=0, ++cnt; i < NPoints; i++) Data[i]=cnt;
            }
           else
           	{
            // 1. запишем кусок до конца буфера
            n=RealBufferSize-UserIndex;
            memcpy(Data, &RealTimePtr[UserIndex], 2*n);

            UserIndex=0;

            // дозапишем хвост
            memcpy(&Data[n], &RealTimePtr[UserIndex], 2*(NPoints-n));
            UserIndex += (NPoints-n);
            }

            //cnt++;

return true;
}



//------------------------------------------------------------------------
// Поток, в котором осуществляется сбор данных
//------------------------------------------------------------------------
DWORD WINAPI ServiceRealTImeReadThread(PVOID /*Context*/)
{
	WORD RequestNumber;
	DWORD i;
	// идентификатор массива их двух событий
	HANDLE ReadEvent[2];
	// массив OVERLAPPED структур из двух элементов
	OVERLAPPED ReadOv[2];
	IO_REQUEST_LUSBAPI IoReq[2];
   DWORD n;DWORD OldPointer;

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
		IoReq[i].Buffer = AdcBuffer + i*GlobalPacketSize;
		IoReq[i].NumberOfWordsToPass = GlobalPacketSize;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &ReadOv[i];
		IoReq[i].TimeOut = 1000;
	}

	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// запустим АЦП
	if(pModule->START_ADC())
	{
		// цикл сбора данных
		for(;;)
		{
			// сделаем запрос на очередную порции данных
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }

			// ждём завершения операции сбора предыдущей порции данных
         for(;;)
         	{
            if(StopEvent) break;
				if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) continue;
            	else break;
            }
         if(StopEvent) break;

			// Данные записываются в FIFO размером RealBufferSize пачками по  GlobalPacketSize
			// текущее положение указателя записи - RealTimeReadIndex
			// текущее положения указателя считывания  UserIndex
         OldPointer=RealTimeReadIndex;
         if((RealTimeReadIndex+GlobalPacketSize) <= RealBufferSize)
         	{
            memcpy(&RealTimePtr[RealTimeReadIndex], IoReq[RequestNumber^0x1].Buffer, 2*GlobalPacketSize);
            RealTimeReadIndex += GlobalPacketSize;

	         // проверим на переполнение
   	      if(UserIndex > OldPointer && UserIndex <= RealTimeReadIndex)   OverflowFlag=1;

            if(RealTimeReadIndex == RealBufferSize) RealTimeReadIndex=0;
            }
           else
           	{
            // 1. запишем кусок до конца буфера
            n=RealBufferSize-RealTimeReadIndex;
            memcpy(&RealTimePtr[RealTimeReadIndex], IoReq[RequestNumber^0x1].Buffer, 2*n);

	         // проверим на переполнение
            RealTimeReadIndex += n;
   	      if(UserIndex > OldPointer && UserIndex <= RealTimeReadIndex)   OverflowFlag=1;

            RealTimeReadIndex=0;

            // дозапишем хвост
            memcpy(&RealTimePtr[RealTimeReadIndex], &IoReq[RequestNumber^0x1].Buffer[n], 2*(GlobalPacketSize-n));
            RealTimeReadIndex += (GlobalPacketSize-n);
            }


			if(ReadThreadErrorNumber) break;
				else Sleep(20);
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
	Sleep(20);
	// установим флажок завершения работы потока сбора данных
	IsReadThreadComplete = true;
	// теперь можно спокойно выходить из потока
	return 0x0;
}


__declspec(dllexport) BOOL WINAPI WEProcessArray(SHORT *src, double *dest, DWORD size, DWORD calibr, DWORD volt)
{
    return (BOOL)pModule->ProcessArray(src, dest, size, calibr, volt);
}

__declspec(dllexport) BOOL WINAPI WEProcessOnePoint(SHORT src, double *dest, DWORD channel, DWORD calibr, DWORD volt)
{
    return (BOOL)pModule->ProcessOnePoint(src, dest, channel, calibr, volt);
}

__declspec(dllexport) BOOL WINAPI WEFIFO_STATUS(DWORD *FifoOverflowFlag, double *FifoMaxPercentLoad, DWORD *FifoSize, DWORD *MaxFifoBytesUsed)
{
    return (BOOL)pModule->FIFO_STATUS(FifoOverflowFlag, FifoMaxPercentLoad, FifoSize, MaxFifoBytesUsed);
}


__declspec(dllexport) BOOL WINAPI WEDAC_SAMPLE_VOLT(double DacData, DWORD calibr)
{
    return (BOOL)pModule->DAC_SAMPLE_VOLT(DacData, calibr);
}


__declspec(dllexport) BOOL WINAPI WE_GET_ARRAY(BYTE *Buffer, WORD Size, WORD Address)
{
    return (BOOL)pModule->GetArray (Buffer, Size, Address);
}

__declspec(dllexport) BOOL WINAPI WE_PUT_ARRAY(BYTE *Buffer, WORD Size, WORD Address)
{
    return (BOOL)pModule->PutArray (Buffer, Size, Address);
}


