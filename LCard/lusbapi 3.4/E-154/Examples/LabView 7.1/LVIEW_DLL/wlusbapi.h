#ifndef __WLUSBAPI__
#define __WLUSBAPI__
#include <windows.h>
#include <wtypes.h>

// -----------------------------------------------------------------------------
// wlusbapi - "�������" ��������  dll, ��� ������ � ������� ��
//            ��������������� ��������� (MSVB, NI LabView,....)
// -----------------------------------------------------------------------------
// ������ ����������
#define  WE140LC_VERMAJOR                       2       // ������ ���� �����
#define  WE140LC_VERMINOR                       1               // ������ ���� �����
#define  WE140LC_CURRENT_VERSION    ((WE140LC_VERMAJOR<<16)|WE140LC_VERMINOR)


// -----------------------------------------------------------------------------
// ---------------------------------- E140 -------------------------------------
// -----------------------------------------------------------------------------
#pragma pack(4)
/*
// ���������, �������� ����� ������ ��� ��� ������ E-140
typedef struct {                        //
 WORD ClkSource;                        // �������� �������� ��������� ��� ������� ���
 WORD EnableClkOutput;                  // ���������� ���������� �������� ��������� ������� ���
 WORD InputMode;                        // ����� ����� ����� � ���
 WORD SynchroAdType;                    // ��� ���������� �������������
 WORD SynchroAdMode;                    // ����� ���������� ������������
 WORD SynchroAdChannel;                 // ����� ��� ��� ���������� �������������
 SHORT SynchroAdPorog;                  // ����� ������������ ��� ��� ���������� �������������
 WORD ChannelsQuantity;                 // ����� �������� �������
 WORD ControlTable[128];                // ����������� ������� � ��������� ��������
 DOUBLE AdcRate;                        // ������� ������ ��� � ���
 DOUBLE InterKadrDelay;                 // ����������� �������� � ��
 DOUBLE ChannelRate;                    // ������� ������ ������
} ADC_PARS_WE154 ;                      //
// ���������, ���������� ���������� � �����, ������� �������� � ����. ����
typedef struct                          //
{                                       //
 BYTE Name[11];                         // �������� �����
 BYTE SerialNumber[9];                  // �������� �����
 BYTE Revision;                         // ������� �����
 BYTE CpuType[11];                      // ��� �������������� DSP
 LONG QuartzFrequency;                  // ������� ������ � ��
 BOOL IsDacPresented;                   // ������ ������� ���
 DOUBLE CalibrKoefAdc[8];               // ���������������� ����. ��� ���
 DOUBLE CalibrKoefDac[4];               // ���������������� ����. ��� ���
} MODULE_DESCR_WE154;                   //
*/
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
// ���������� ������� ������ ����������
__declspec(dllexport) DWORD WINAPI WE154GetDllVersion(VOID);
// �������� ���������� ������������ ������
__declspec(dllexport) LPVOID WINAPI WE154CreateInstance(VOID);
//  ������������ ���������� ����������
__declspec(dllexport) HANDLE WINAPI WE154GetModuleHandle(LPVOID lptr);
//  ������� ����������� ���� ��� ������� � USB ������
__declspec(dllexport) BOOL WINAPI WE154OpenLDevice(LPVOID lptr, WORD VirtualSlot);
//  ��������� ������� ����������� ����
__declspec(dllexport) BOOL WINAPI WE154CloseLDevice(LPVOID lptr);
//  ��������� ��������� �� ����������
__declspec(dllexport) BOOL WINAPI WE154ReleaseLDevice(LPVOID lptr);
//  ������ �������� ������
__declspec(dllexport) BOOL WINAPI WE154GetModuleName(LPVOID lptr, BYTE *ModuleName);
//  ������� ������ ������ � ��������� ������� ���������� ������������ �������
__declspec(dllexport)  INT WINAPI WE154GetLastErrorString(LPVOID lptr, LPTSTR lpBuffer, DWORD nSize);
//  ���������� ������ �����
__declspec(dllexport) BOOL WINAPI WE154GET_LBIOS_VERSION(LPVOID lptr, DWORD *LbiosVersion);
// ��������� ������� ���������� ������ ���
__declspec(dllexport) BOOL WINAPI WE154GET_CUR_ADC_PARS(LPVOID lptr, struct ADC_PARS_E154 *am);
//  ���������� ��������� ���������� ������ ���
__declspec(dllexport) BOOL WINAPI WE154FILL_ADC_PARS(LPVOID lptr, struct ADC_PARS_E154 *am);
//  ������ ������ ���
__declspec(dllexport) BOOL WINAPI WE154START_ADC(LPVOID lptr);
//  ������� ���
__declspec(dllexport) BOOL WINAPI WE154STOP_ADC(LPVOID lptr);
//  ���� ����� �������� � ��� ������
__declspec(dllexport) BOOL WINAPI WE154ADC_KADR(LPVOID lptr, SHORT *Data);
//  ����������� ���� � ��������� ����������� ������ ��� ������
//__declspec(dllexport) BOOL WINAPI WE154ADC_SAMPLE(LPVOID lptr, SHORT *Data, WORD Channel)
//__declspec(dllexport) double WINAPI LV_SAMPLE_E154(DWORD Channel, DWORD *error);
__declspec(dllexport) double WINAPI LV_SAMPLE_E154(DWORD Channel, DWORD UseCalibr, DWORD *error);
//  ��������� ������ ������ � ��� ������
//__declspec(dllexport) BOOL WINAPI WE154ReadData(LPVOID lptr, SHORT *lpBuffer, LPDWORD nNumberOfWordsToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
__declspec(dllexport) BOOL WINAPI WE154ReadData(LPVOID lptr, IO_REQUEST_LUSBAPI *ReadRequest);
//  ����������� ����� �� �������� ����� ���
//__declspec(dllexport) BOOL WINAPI WE154DAC_SAMPLE(LPVOID lptr, WORD *Data, WORD Channel);
__declspec(dllexport) BOOL WINAPI LV_OUTDA_E154(double value);
//  ������� ���������� �������� ����� �������� ��������� �������
__declspec(dllexport) BOOL WINAPI WE154ENABLE_TTL_OUT(LPVOID lptr, BOOL flag);
//   ������� ������ ������� ����� �������� ��������� �������
__declspec(dllexport) BOOL WINAPI WE154TTL_IN(LPVOID lptr, WORD *TtlIn);
//  ������� ������ �� �������� ����� �������� ��������� �������
__declspec(dllexport) BOOL WINAPI WE154TTL_OUT(LPVOID lptr, WORD TtlOut);
//  ����������/���������� ������ ������ � ���� ������
__declspec(dllexport) BOOL WINAPI WE154ENABLE_FLASH_WRITE(LPVOID lptr, BOOL EnableFlashWrite);
//  ������� ��������� ���������� � ������ �� ����
__declspec(dllexport) BOOL WINAPI WE154GET_MODULE_DESCR(LPVOID lptr, struct MODULE_DESCRIPTION_E154 *md);
//  ������� ��������� ���������� � ������ � ����
__declspec(dllexport) BOOL WINAPI WE154SAVE_MODULE_DESCR(LPVOID lptr, struct MODULE_DESCRIPTION_E154 *md);
//  ��������� ����������� ������� ����� ������ ��� �������� ���� usb � ����� suspend
__declspec(dllexport) BOOL WINAPI WE154SetSuspendModeFlag(LPVOID lptr, BOOL SuspendModeFlag);
__declspec(dllexport) DWORD WINAPI LV_OpenE154(VOID);
__declspec(dllexport) DWORD WINAPI LV_CreateChannelE154(DWORD channel, DWORD gain);
__declspec(dllexport) DWORD WINAPI LV_CloseE154(VOID);
__declspec(dllexport) DWORD WINAPI LV_GETTTL_E154(DWORD *error);
__declspec(dllexport) BOOL WINAPI LV_SETTTL_E154(DWORD TtlOut);
__declspec(dllexport) BOOL WINAPI LV_SET_ADC_SYCHRO_READ_PARAMETERS(DWORD Nch, DWORD *Channels,
	float Rate, float AdcDelay, DWORD InputMode, DWORD SynchroAdType, DWORD SynchroAdMode, DWORD SynchroAdChannel,
   float SynchroAdPorog, float *SetRate, float *SetAdcDelay);
__declspec(dllexport) BOOL WINAPI LV_GET_ADC_SYNCHRO_ARRAY(DWORD NPoints, SHORT *Data, DWORD TimeOut);
__declspec(dllexport) BOOL WINAPI LV_START_REAL_TIME_SAMPLING(DWORD BufferSize, DWORD PacketSize);
__declspec(dllexport) BOOL WINAPI LV_GET_REAL_TIME_INDEX(DWORD *index, DWORD *overflow_flag, DWORD *user_index);
__declspec(dllexport) BOOL WINAPI LV_GET_REAL_TIME_DATA(SHORT *Data, DWORD NPoints);
__declspec(dllexport) BOOL WINAPI LV_STOP_REAL_TIME_SAMPLING(void);
__declspec(dllexport) BOOL WINAPI WE154D_FLASH_ARRAY(LPVOID lptr, unsigned char *ptr);
__declspec(dllexport) BOOL WINAPI  WEREAD_FLASH_ARRAY(LPVOID lptr, unsigned char *ptr);
__declspec(dllexport) BOOL WINAPI WEProcessArray(SHORT *src, double *dest, DWORD size, DWORD calibr, DWORD volt);
__declspec(dllexport) BOOL WINAPI WEProcessOnePoint(SHORT src, double *dest, DWORD channel, DWORD calibr, DWORD volt);
__declspec(dllexport) BOOL WINAPI WEFIFO_STATUS(DWORD *FifoOverflowFlag, double *FifoMaxPercentLoad, DWORD *FifoSize, DWORD *MaxFifoBytesUsed);
__declspec(dllexport) BOOL WINAPI WEDAC_SAMPLE_VOLT(double DacData, DWORD calibr);
__declspec(dllexport) BOOL WINAPI WE_GET_ARRAY(BYTE *Buffer, WORD Size, WORD Address);
__declspec(dllexport) BOOL WINAPI WE_PUT_ARRAY(BYTE *Buffer, WORD Size, WORD Address);

#ifdef __cplusplus
 }
#endif

#endif
