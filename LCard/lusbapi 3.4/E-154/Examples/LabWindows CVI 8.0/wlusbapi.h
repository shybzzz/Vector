#ifndef __WLUSBAPI__
#define __WLUSBAPI__
#include <windows.h>
#include <wtypes.h>

// -----------------------------------------------------------------------------
// wlusbapi - "обертка" основной  dll, для работы с языками не
//            поддерживающими указатели (MSVB, NI LabView,....)
// -----------------------------------------------------------------------------
// версия библиотеки
#define  WE140LC_VERMAJOR                       2       // только одна цифра
#define  WE140LC_VERMINOR                       1               // только одна цифра
#define  WE140LC_CURRENT_VERSION    ((WE140LC_VERMAJOR<<16)|WE140LC_VERMINOR)


// -----------------------------------------------------------------------------
// ---------------------------------- E140 -------------------------------------
// -----------------------------------------------------------------------------
#pragma pack(4)
/*
// структура, задающая режим работы АЦП для модуля E-140
typedef struct {                        //
 WORD ClkSource;                        // источник тактовых импульсов для запуска АПП
 WORD EnableClkOutput;                  // разрешение тарнсляции тактовых импульсов запуска АЦП
 WORD InputMode;                        // режим ввода даных с АЦП
 WORD SynchroAdType;                    // тип аналоговой синхронизации
 WORD SynchroAdMode;                    // режим аналоговой сихронизации
 WORD SynchroAdChannel;                 // канал АЦП при аналоговой синхронизации
 SHORT SynchroAdPorog;                  // порог срабатывания АЦП при аналоговой синхронизации
 WORD ChannelsQuantity;                 // число активных каналов
 WORD ControlTable[128];                // управляющая таблица с активными каналами
 DOUBLE AdcRate;                        // частота работы АЦП в кГц
 DOUBLE InterKadrDelay;                 // межкадровая задержка в мс
 DOUBLE ChannelRate;                    // частота одного канала
} ADC_PARS_WE154 ;                      //
// структура, содержащая информацию о плате, которая хранится в поль. ППЗУ
typedef struct                          //
{                                       //
 BYTE Name[11];                         // название платы
 BYTE SerialNumber[9];                  // серийный номер
 BYTE Revision;                         // ревизия платы
 BYTE CpuType[11];                      // тип установленного DSP
 LONG QuartzFrequency;                  // частота кварца в Гц
 BOOL IsDacPresented;                   // флажок наличия ЦАП
 DOUBLE CalibrKoefAdc[8];               // корректировочные коэф. для АЦП
 DOUBLE CalibrKoefDac[4];               // корректировочные коэф. для ЦАП
} MODULE_DESCR_WE154;                   //
*/
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
// возвращает текущую версию библиотеки
__declspec(dllexport) DWORD WINAPI WE154GetDllVersion(VOID);
// создание экземпляра необходимого класса
__declspec(dllexport) LPVOID WINAPI WE154CreateInstance(VOID);
//  возвращается дескриптор устройства
__declspec(dllexport) HANDLE WINAPI WE154GetModuleHandle(LPVOID lptr);
//  Откроем виртуальный слот для доступа к USB модулю
__declspec(dllexport) BOOL WINAPI WE154OpenLDevice(LPVOID lptr, WORD VirtualSlot);
//  Освободим текущий виртуальный слот
__declspec(dllexport) BOOL WINAPI WE154CloseLDevice(LPVOID lptr);
//  Освободим указатель на устройство
__declspec(dllexport) BOOL WINAPI WE154ReleaseLDevice(LPVOID lptr);
//  Чтение название модуля
__declspec(dllexport) BOOL WINAPI WE154GetModuleName(LPVOID lptr, BYTE *ModuleName);
//  функция выдачи строки с последней ошибкой выполнения интерфейсной функции
__declspec(dllexport)  INT WINAPI WE154GetLastErrorString(LPVOID lptr, LPTSTR lpBuffer, DWORD nSize);
//  возвращает версию биоса
__declspec(dllexport) BOOL WINAPI WE154GET_LBIOS_VERSION(LPVOID lptr, DWORD *LbiosVersion);
// получение текущих параметров работы АЦП
__declspec(dllexport) BOOL WINAPI WE154GET_CUR_ADC_PARS(LPVOID lptr, struct ADC_PARS_E154 *am);
//  заполнение требуемых параметров работы АЦП
__declspec(dllexport) BOOL WINAPI WE154FILL_ADC_PARS(LPVOID lptr, struct ADC_PARS_E154 *am);
//  запуск работы АЦП
__declspec(dllexport) BOOL WINAPI WE154START_ADC(LPVOID lptr);
//  Останов АЦП
__declspec(dllexport) BOOL WINAPI WE154STOP_ADC(LPVOID lptr);
//  ввод кадра отсчетов с АЦП модуля
__declspec(dllexport) BOOL WINAPI WE154ADC_KADR(LPVOID lptr, SHORT *Data);
//  однократный ввод с заданного логического канала АЦП модуля
__declspec(dllexport) BOOL WINAPI WE154ADC_SAMPLE(LPVOID lptr, SHORT *Data, WORD Channel);
//  потоковое чтение данных с АЦП модуля
//__declspec(dllexport) BOOL WINAPI WE154ReadData(LPVOID lptr, SHORT *lpBuffer, LPDWORD nNumberOfWordsToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
__declspec(dllexport) BOOL WINAPI WE154ReadData(LPVOID lptr, struct IO_REQUEST_LUSBAPI *ReadRequest);
//  однократный вывод на заданный канал ЦАП
__declspec(dllexport) BOOL WINAPI WE154DAC_SAMPLE(LPVOID lptr, WORD *Data, WORD Channel);
//  функция разрешения выходных линий внешнего цифрового разъёма
__declspec(dllexport) BOOL WINAPI WE154ENABLE_TTL_OUT(LPVOID lptr, BOOL flag);
//   функция чтения входных линии внешнего цифрового разъёма
__declspec(dllexport) BOOL WINAPI WE154TTL_IN(LPVOID lptr, WORD *TtlIn);
//  функция вывода на выходные линии внешнего цифрового разъёма
__declspec(dllexport) BOOL WINAPI WE154TTL_OUT(LPVOID lptr, WORD TtlOut);
//  разрешение/запрещение режима записи в ППЗУ модуля
__declspec(dllexport) BOOL WINAPI WE154ENABLE_FLASH_WRITE(LPVOID lptr, BOOL EnableFlashWrite);
//  чтенние слова из ППЗУ
__declspec(dllexport) BOOL WINAPI WE154READ_FLASH_WORD(LPVOID lptr, WORD FlashAddress, SHORT *FlashWord);
//  запись слова в ППЗУ
__declspec(dllexport) BOOL WINAPI WE154WRITE_FLASH_WORD(LPVOID lptr, WORD FlashAddress, SHORT FlashWord);
//  получим служебную информацию о модуле из ППЗУ
__declspec(dllexport) BOOL WINAPI WE154GET_MODULE_DESCR(LPVOID lptr, struct MODULE_DESCRIPTION_E154 *md);
//  запишем служебную информацию о модуле в ППЗУ
__declspec(dllexport) BOOL WINAPI WE154SAVE_MODULE_DESCR(LPVOID lptr, struct MODULE_DESCRIPTION_E154 *md);
//  состояние аналогового питания узлов модуля при переходе шины usb в режим suspend
__declspec(dllexport) BOOL WINAPI WE154SetSuspendModeFlag(LPVOID lptr, BOOL SuspendModeFlag);
#ifdef __cplusplus
 }
#endif

#endif
