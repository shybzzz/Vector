#include <windows.h>
#include "toolbox.h"
#include <analysis.h>
#include <utility.h>
#include <rs232.h>
#include <ansi_c.h>
#include <cvirte.h>		/* Needed if linking in external compiler; harmless otherwise */
#include <userint.h>
#include <winioctl.h>
#include "test.h"
#include "bulkioct.h"

//#include "lusbapi.h"
	#pragma pack(1)
	// структура с информацией об модуле E-154
#include "LusbapiTypes.h"   

	struct MODULE_DESCRIPTION_E154
	{
		struct MODULE_INFO_LUSBAPI     Module;		// общая информация о модуле
		struct INTERFACE_INFO_LUSBAPI  Interface;	// информация об используемом интерфейсе
		struct MCU_INFO_LUSBAPI		Mcu;	// информация о микроконтроллере
		struct ADC_INFO_LUSBAPI        Adc;			// информация о АЦП
		struct DAC_INFO_LUSBAPI        Dac;			// информация о ЦАП
		struct DIGITAL_IO_INFO_LUSBAPI DigitalIo;	// информация о цифровом вводе-выводе
	};
	// структура, задающая режим работы АЦП для модуля E14-154
	struct ADC_PARS_E154
	{
		WORD ClkSource;							// источник тактовых импульсов для запуска АПП
		WORD EnableClkOutput;					// разрешение трансляции тактовых импульсов запуска АЦП
		WORD InputMode;							// режим ввода даных с АЦП
		WORD SynchroAdType;						// тип аналоговой синхронизации
		WORD SynchroAdMode; 						// режим аналоговой сихронизации
		WORD SynchroAdChannel;  				// канал АЦП при аналоговой синхронизации
		SHORT SynchroAdPorog; 					// порог срабатывания АЦП при аналоговой синхронизации
		WORD ChannelsQuantity;					// число активных каналов
		WORD ControlTable[128];					// управляющая таблица с активными каналами
		double AdcRate;	  			  			// частота работы АЦП в кГц
		double InterKadrDelay;		  			// межкадровая задержка в мс
		double KadrRate;							// частота кадра в кГц
	};
	#pragma pack()
	
	

#include "wlusbapi.h"

#define N_POINTS 5000
#define  LUSB_RESULT_OK  (10000)

#define RECT_WINDOW	0
#define HANN_WINDOW	1
#define	HAMMING_WINDOW	2
#define BLACKMAN_WINDOW	3
#define BARLETTE_WINDOW 4
#define BLACKMAN_HARRIS_WINDOW 5

#define M_PI 3.14
	
int InitLtr(void) ;
int CloseLtr(void) ;
	
void SynchroPanel(void);	

double RatekHz=120, InterKadrDelay, RealRate=120;

FillBpfWindow(double *ptr, int n, int type);
double zero_search(double *arr);
double SNR_parm, EOB_parm;
int FingSNR(double *ptr, int N, float max);
long calc_one_turn(int *Ptr, int N, double *f1, double *a1);
long calc_one_turn(int *Ptr, int N, double *f1, double *a1);
double CalculateFreq(int nch, int rate);
int DrawHyst(int handle, int ctrl, double *data, int n_points, double freq);
double Ac, Dc, Mx, Dx;
double harmonic_amplitudes[100];
double harmonic_frequencies[100];
double Thdnoise, Thd;
int DrawGraphScreen(void);
void LoadParm(void);
int DrawSpektr(int handle, int ctrl, double *data, int n_points, double freq);
int DataOn;
int	DataNch;
int	DataNPoints;
int	DataRate;
int DrawChanOn[8];
int DrawGain[8];
void ShowAcDc(double *data, int n);

int E154Soft(int *channel_on, int *gain, int rate, int n, signed short int *data);
void DrawCtrl(void);
int RunSoft(void);
int E154Adchan(int channel, int gain, signed short *ad_value);
double GainArray[4]={5.0, 1.6, 0.5, 0.16};	// 2048
int RunAdchan(void);
int GetArray(unsigned char * const Buffer, unsigned short int Size, unsigned short int Address);

int LoopsN;
int ScreenWidth, ScreenHeight;
int AdcDrawMode;
extern int AnalisPcMode;
unsigned short int CRC_Calc16(unsigned char *b, unsigned char N) ;
void ConfigAxes(void);

int LoopOn, StopPressed, LoopN;
int y_scale_mode=1, x_scale_mode=1;
static int panelHandle;
int GraphPnl;
int LastErrorNumber;
char Com, Data1, Data2, Buf[20];
unsigned char cstr[1024], Rx[10240], Tx[1024];
double CurrentI;
int FlashMode;
double MaxArray[512], MinArray[512], Max, Min;
int MaxIndex, MinIndex;
int PlotHnd;
int BlocksN;
unsigned short int data[32000];
unsigned char drive_name[20], dir_name[1024], file_name[256];

signed short *Data;
double *DataDouble;
double *Conv_Spectrum;
double *Auto_Spectrum, *ArrayXRe, *ArrayXIm, *Window;


int AutoScaleX[3]={1,1,1};
int AutoScaleY[3]={1,1,1};
double MinX[3]={0, 0, 0};
double MaxX[3]={1000, 1000, 1000};
double MinY[3]={-2048, -100, -100};
double MaxY[3]={2048, 0, 0};
int FonColor[3]={VAL_BLACK,VAL_BLACK,VAL_BLACK};
int GraphColor[3]={VAL_YELLOW,VAL_YELLOW,VAL_YELLOW};
int GridColor[3]={VAL_LT_GRAY,VAL_LT_GRAY,VAL_LT_GRAY};


HANDLE hDevice;
unsigned char drive_name[20], dir_name[1024], file_name[256];
unsigned char cfg_file[1024];

int ChannelOnEvent[8]={PANEL_CHAN_ON_1,PANEL_CHAN_ON_2,PANEL_CHAN_ON_3,PANEL_CHAN_ON_4,PANEL_CHAN_ON_5,PANEL_CHAN_ON_6,PANEL_CHAN_ON_7,
					   PANEL_CHAN_ON_8};
int ChannelOn[8]={1,1,1,1, 1,1,1,1};
int ChannelGainEvent[8]={PANEL_ADC_GAIN_1,PANEL_ADC_GAIN_2,PANEL_ADC_GAIN_3,PANEL_ADC_GAIN_4,PANEL_ADC_GAIN_5,PANEL_ADC_GAIN_6,
						PANEL_ADC_GAIN_7,PANEL_ADC_GAIN_8};
int ChannelArrayGain[8];			

int TtlInEvent[8]={PANEL_TTL_1,PANEL_TTL_2,PANEL_TTL_3,PANEL_TTL_4,PANEL_TTL_5,PANEL_TTL_6,PANEL_TTL_7,PANEL_TTL_8};
int TtlOutEvent[8]={PANEL_TTL_16,PANEL_TTL_15,PANEL_TTL_14,PANEL_TTL_13,PANEL_TTL_12,PANEL_TTL_11,PANEL_TTL_10,PANEL_TTL_9};

int ChanDrawEvent[8]={PANEL_CHANDRAW_1,PANEL_CHANDRAW_2,PANEL_CHANDRAW_3,PANEL_CHANDRAW_4,
					  PANEL_CHANDRAW_5,PANEL_CHANDRAW_6,PANEL_CHANDRAW_7,PANEL_CHANDRAW_8};

LPVOID pModule;
HANDLE ModuleHandle;
char ModuleName[32];

// флажок завершения работы потока сбора данных
BOOL IsReadThreadComplete;
// номер ошибки при выполнении сбора данных
WORD ReadThreadErrorNumber;

// экранный счетчик-индикатор
DWORD Counter = 0x0, OldCounter = 0xFFFFFFFF;
DWORD DataStep = 8*1024;
SHORT AdcBuffer[8*1024*2];
WORD NDataBlock;
SHORT *GlobalPtr;
HANDLE hReadThread;
DWORD ReadTid;
static DWORD WINAPI  ServiceReadThread(LPVOID lpp) ;

int SynchroMode;
int SynchroTtlMask;
int SynchroTtlMode;
int SynchroAdChannel;
int SynchroAdGain;
int SynchroAdMode;
int SynchroAdSubMode1;
int SynchroAdSubMode2;
double SynchroAdPorog;
int StopCommandFlag;

int RunTestMain(void);


int main (int argc, char *argv[])
{

 int i, j, i1, len, Handle, ErrorCode, n=10, n1;
 
 char str[180], *str1, str2[100];
// unsigned char DeviceName[512];
// OVERLAPPED overlapped;
 
if (InitCVIRTE (0, argv, 0) == 0)	/* Needed if linking in external compiler; harmless otherwise */
		return -1;	/* out of memory */
 
 RunTestMain();
 return 0;
  	  /*
 if (InitCVIRTE (0, argv, 0) == 0)
		return -1;
 if ((panelHandle = LoadPanel (0, "test.uir", PANEL)) < 0)
		return -1;

    i=GetFullPathFromProject ("e154_test.exe", cstr);
    if(i)
      {
      MessagePopup("СТАТУС", "Отсутствует файл e154_test.exe.");
      return 0;
      }
	SplitPath (cstr, drive_name, dir_name, file_name);
	i=toupper(drive_name[0])-'A';
	SetDrive (i);
	SetDir (dir_name);

	sprintf(cfg_file, "%s%se154test.cfg", drive_name, dir_name);
	
	pModule=WE154CreateInstance();
	if(!pModule) MessagePopup("ERROR", "No dll found");  
		{
		for(i = 0x0; i < 128; i++) if(WE154OpenLDevice(pModule, i)) break;
		// что-нибудь обнаружили?
		if(i == 128)  MessagePopup("ERROR", "No lcard usb device found");
		  else
		  {
		  ModuleHandle=WE154GetModuleHandle(pModule);
	  	  if(ModuleHandle == INVALID_HANDLE_VALUE) MessagePopup("ERROR", "No lcard usb device found"); 
		    else
				{
				if(!WE154GetModuleName(pModule, ModuleName)) 	MessagePopup("ERROR", "No lcard usb device found"); 
				  else 
				  	{
					if(strcmp(ModuleName, "E154")) MessagePopup("ERROR", "No E-154");
						else   MessagePopup("INFO", "E-154 found OK.");  
				  	}
				}
		  }
	
		}
	
 LoadParm(); 
 DrawCtrl();  
 DisplayPanel (panelHandle);
 
 Data=malloc(2*1000000);
 DataDouble=malloc(8*200000);
 Conv_Spectrum=malloc(8*100000);
 Auto_Spectrum=malloc(8*100000);
 Auto_Spectrum=malloc(8*100000);
 ArrayXRe=malloc(8*100000);
 ArrayXIm=malloc(8*100000);
 Window=malloc(8*100000);
 
 if(Data == NULL || DataDouble == NULL || Conv_Spectrum  == NULL || Window==NULL ||
	 Auto_Spectrum == NULL || ArrayXIm == NULL || ArrayXRe == NULL) {MessagePopup("ERROR", "No memory"); return 0; }
 
 InitLtr();
 RunUserInterface ();	  */
 return 0;
}

unsigned short int CRC_Calc16(unsigned char *b, unsigned char N) 
{
unsigned short int crc;
unsigned char i;
unsigned short int j;       
  
for(i=crc=0; i<N; i++) 
 {                                  
  j=b[i];
  crc = crc ^ (j << 8);
  for(j=0;j<8;j++) 
   {
   if(crc & 0x8000) crc = (crc<<1) ^ 0x1021;
     else crc <<= 1;
   }
 }

return crc;
}

int CVICALLBACK Done (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
// 			if(hDevice!=INVALID_HANDLE_VALUE) CloseHandle(hDevice) ;
		if(pModule)
			{
				// освободим интерфейс модуля
				if(!WE154ReleaseLDevice(pModule));
				pModule = NULL;
			}
			CloseLtr();
		    QuitUserInterface (0);
			break;
		}
	return 0;
}



int TestLink(void)
{
 int i, j, i1, len, Handle, ErrorCode, n=10, n1;
char str[256], *str1, str2[256];

i=GetArray(str, 2, 0) ;
if(str[0] != 3 || str[1] != 4) return 2;
if(i) return 1;

return 0;
}

int CVICALLBACK RECONECT_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
 int i, j, i1, len, Handle, ErrorCode, n=10, n1;
 
 char str[180], *str1, str2[100];
 unsigned char DeviceName[512];
 OVERLAPPED overlapped;

	switch (event)
		{
		case EVENT_COMMIT:
			if(hDevice!=INVALID_HANDLE_VALUE) CloseHandle(hDevice) ; 
  
 
 for(i=0; i < 128; i++)
   {
   sprintf(DeviceName, "\\\\.\\LDevUsb%d", i);
 
   hDevice=CreateFile(DeviceName, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  if(hDevice==INVALID_HANDLE_VALUE) continue;
    else break;
    }

  if(i == 128)  
    {
    MessagePopup("ERROR", "No usb found");
    }
			
			break;
		}
	return 0;
}


//-----------------------------------------------------------------------------
// асинхронная запись данных в плату
//-----------------------------------------------------------------------------
int WriteData(LPBYTE lpBuffer, LPDWORD lpNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWrite, LPOVERLAPPED lpOverlapped)
{
 LastErrorNumber=LUSB_RESULT_OK;
 if(lpNumberOfBytesToWrite!=NULL) {
    if(hDevice!=INVALID_HANDLE_VALUE) {
	    if(!WriteFile(hDevice, lpBuffer, *lpNumberOfBytesToWrite, lpNumberOfBytesWrite, lpOverlapped) &&
           GetLastError()!=ERROR_IO_PENDING) LastErrorNumber=1212;
    } else LastErrorNumber=1004;
 } else LastErrorNumber=1000;
 return LastErrorNumber<LUSB_RESULT_OK ? 0 : 1;
}

//-----------------------------------------------------------------------------
// синхронная запись данных в плату
//-----------------------------------------------------------------------------
int SyncWriteData(LPBYTE lpBuffer, LPDWORD lpNumberOfBytesToWrite, DWORD TimeOut)
{
 OVERLAPPED overlapped;
 memset(&overlapped, 0, sizeof(overlapped));
 overlapped.hEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
 if(overlapped.hEvent!=NULL) {
    DWORD size;
    if(WriteData(lpBuffer, lpNumberOfBytesToWrite, &size, &overlapped)) {
        if(WaitForSingleObject(overlapped.hEvent, TimeOut)==WAIT_OBJECT_0) {
            GetOverlappedResult(hDevice, &overlapped, lpNumberOfBytesToWrite, FALSE);
        } else {
            CancelIo(hDevice);
            LastErrorNumber=1000;
        }
    }
    CloseHandle(overlapped.hEvent);
 } else LastErrorNumber=11111;
 return LastErrorNumber<LUSB_RESULT_OK ? 0 : 1;
}


//-----------------------------------------------------------------------------
// асинхроннное чтение данных из платы
//-----------------------------------------------------------------------------
int ReadData(LPBYTE lpBuffer, LPDWORD lpNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
 LastErrorNumber=LUSB_RESULT_OK;
 if(lpNumberOfBytesToRead!=NULL) {
    if(hDevice!=INVALID_HANDLE_VALUE) {
        if(!ReadFile(hDevice, lpBuffer, *lpNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped) &&
           GetLastError()!=ERROR_IO_PENDING) LastErrorNumber=1211;
    } else LastErrorNumber=1004;
 } else LastErrorNumber=1000;
 return LastErrorNumber<LUSB_RESULT_OK ? 0 : 1;
}

//-----------------------------------------------------------------------------
// синхроннное чтение данных из платы
//-----------------------------------------------------------------------------
int SyncReadData(LPBYTE lpBuffer, LPDWORD lpNumberOfBytesToRead, DWORD TimeOut)
{
 OVERLAPPED overlapped;
 memset(&overlapped, 0, sizeof(overlapped));
 overlapped.hEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
 if(overlapped.hEvent!=NULL) {
    DWORD size;
    if(ReadData(lpBuffer, lpNumberOfBytesToRead, &size, &overlapped)) {
        if(WaitForSingleObject(overlapped.hEvent, TimeOut)==WAIT_OBJECT_0) {
            GetOverlappedResult(hDevice, &overlapped, lpNumberOfBytesToRead, FALSE);
        } else {
        CancelIo(hDevice);
        LastErrorNumber=1000;     
        }
    }
    CloseHandle(overlapped.hEvent);
 } else LastErrorNumber=11111;
 return LastErrorNumber<LUSB_RESULT_OK ? 0 : 1;
}



// returns 1 if bad, 0 if good
int LsitBigCommand(int command, int n_rec, unsigned char *out_array, unsigned char *in_array)
{
  int i, n, n1, j;
  unsigned char data[256];
  
  data[0]=0xA5;
  data[1]=command;
  for(i=0; i < 126; i++) data[2+i]=out_array[i];
 
  n=127;
  i=SyncWriteData(data, &n, INFINITE);
  if(!i) return 1;

  n1=n_rec;
  j=SyncReadData(in_array, &n1, 1000);
  if(!j) return 1;

  if(n1 != n_rec) return 1;
 
return 0;
}


int CVICALLBACK TEST_LINK_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int i;

	switch (event)
		{
		case EVENT_COMMIT:
			SetCtrlVal(panelHandle, PANEL_STATUS, "");
			i=TestLink();
			if(i)
			  {
			  sprintf(cstr, "Error Code=%u", i);
			  SetCtrlVal(panelHandle, PANEL_STATUS, cstr);
			  }
			 else 
			    SetCtrlVal(panelHandle, PANEL_STATUS, "Тест пройден успешно");
			break;
		}
	return 0;
}


unsigned int TimeOut=1000;

int LDeviceIoControl(	DWORD dwIoControlCode,		// control code of operation to perform
														LPVOID lpInBuffer,			// pointer to buffer to supply input data
														DWORD nInBufferSize,			// size of input buffer in bytes
														LPVOID lpOutBuffer,			// pointer to buffer to receive output data
														DWORD nOutBufferSize,		// size of output buffer in bytes
														DWORD TimeOut)					// таймаут в мс
{

	DWORD RealBytesTransferred;
	DWORD BytesReturned;
	OVERLAPPED Ov;
	DWORD result;

	// виртуальный слот доступен?
	if(hDevice == INVALID_HANDLE_VALUE) {  return FALSE; }

	// инициализируем OVERLAPPED структуру
	memset(&Ov, 0x0, sizeof(OVERLAPPED));
	// создаём событие для асинхронного запроса
	Ov.hEvent = CreateEvent(NULL, FALSE , FALSE, NULL);
	if(!Ov.hEvent) { return FALSE; }

	// посылаем требуемый запрос в AVR
	result=DeviceIoControl(	hDevice, dwIoControlCode,
								lpInBuffer, nInBufferSize,
								lpOutBuffer, nOutBufferSize,
								&BytesReturned, &Ov);
	if(!result) { if((result=GetLastError()) != ERROR_IO_PENDING) { LastErrorNumber = 1001; CloseHandle(Ov.hEvent); return 1; } }
	// ждём окончания выполнения запроса
	if(WaitForSingleObject(Ov.hEvent, TimeOut) == WAIT_TIMEOUT) { CancelIo(hDevice); CloseHandle(Ov.hEvent); LastErrorNumber = 1001; return 1; }
	// попробуем получить кол-во реально переданных байт данных
	else if(!GetOverlappedResult(hDevice, &Ov, &RealBytesTransferred, TRUE)) { CancelIo(hDevice); CloseHandle(Ov.hEvent); LastErrorNumber = 1001; return 1; }
	// проверим сколько реально было передано байт данных
	if(nOutBufferSize != RealBytesTransferred) { CancelIo(hDevice); CloseHandle(Ov.hEvent); LastErrorNumber = 1001; return 1; }
	// закроем событие асинхронного запроса
	else if(!CloseHandle(Ov.hEvent)) { LastErrorNumber = 1001; return 1; }
	// все хорошо :)))))
	return 0;
}

int GetArray(unsigned char * const Buffer, unsigned short int Size, unsigned short int Address)
{
	unsigned int i;
	unsigned short int InBuf[4] = 
		{ 
			0x0001, 	// RequestType
			0, 			// bRequest
			3, 			// wValue
			0x0 };		// wIndex
	InBuf[2]=Address;

	// проверим буфер и размер
	if(!Buffer || !Size) { //LAST_ERROR_NUMBER(1); 
	return 1; }
	
	// читаем данные
		if(LDeviceIoControl(DIOC_SEND_COMMAND,&InBuf, sizeof(InBuf), Buffer,  Size, TimeOut))  {  return 1; }
	// все хорошо :)))))
	return 0;
}

int GetAdchan(signed short int *ad_data, unsigned int gain, unsigned int channel)
{
	unsigned int i;
	unsigned short int InBuf[4] = 
		{ 
			0x0001, 	// RequestType
			7, 			// bRequest
			0, 			// wValue
			0x0 };		// wIndex
	unsigned char Buffer[2];
	unsigned short int Size=2;
	
	InBuf[2]=(gain & 0xFF) | ((channel & 0xFF) << 8);

	// читаем данные
		if(LDeviceIoControl(DIOC_SEND_COMMAND,&InBuf, sizeof(InBuf), Buffer,  Size, TimeOut))  {  return 1; }
	// все хорошо :)))))
		*ad_data=Buffer[0] | (Buffer[1] << 8);
	return 0;
}

int E154TtlIn(int *ttl)
{
	unsigned int i;
	unsigned short int InBuf[4] = 
		{ 
			0x0001, 	// RequestType Read From E-154
			12, 			// bRequest
			0, 			// wValue
			0x0 };		// wIndex
	unsigned char Buffer[2];
	unsigned short int Size=1;
	
	// читаем данные
	if(LDeviceIoControl(DIOC_SEND_COMMAND,&InBuf, sizeof(InBuf), Buffer,  Size, TimeOut))  {  return 1; }
	
	// все хорошо :)))))
	*ttl=Buffer[0];
	return 0;
}

int E154TtlOut(int ttl)
{
	unsigned int i;
	unsigned short int InBuf[4] = 
		{ 
			0x0000, 	// RequestType Write To E-154
			12, 			// bRequest
			0, 			// wValue
			0x0 };		// wIndex
	unsigned char Buffer[2];
	unsigned short int Size=1;
	
	InBuf[2]=ttl;
	
	// читаем данные
	if(LDeviceIoControl(DIOC_SEND_COMMAND,&InBuf, sizeof(InBuf), Buffer,  Size, TimeOut))  {  return 1; }
	return 0;
}

int E154SetDac(int dac)
{
	unsigned int i;
	unsigned short int InBuf[4] = 
		{ 
			0x0000, 	// RequestType Write To E-154
			13, 			// bRequest
			0, 			// wValue
			0x0 };		// wIndex
	unsigned char Buffer[2];
	unsigned short int Size=1;
	
	InBuf[2]=dac;
	
	// читаем данные
	if(LDeviceIoControl(DIOC_SEND_COMMAND,&InBuf, sizeof(InBuf), Buffer,  Size, TimeOut))  {  return 1; }
	return 0;
}

int SetStartStop(int flag)
{
	unsigned int i;
	unsigned short int InBuf[4] = 
		{ 
			0x0001, 	// RequestType
			3, 			// bRequest
			0, 			// wValue
			0x0 };		// wIndex
	unsigned char Buffer[2];
	unsigned short int Size=0;
	
	InBuf[2]=flag;

	// читаем данные
	if(LDeviceIoControl(DIOC_SEND_COMMAND,&InBuf, sizeof(InBuf), Buffer,  Size, TimeOut))  {  return 1; }
	if(!flag) 
		{
		Delay(0.1);	
		if(LDeviceIoControl(DIOC_RESET_PIPE3, NULL, 0, NULL, 0, TimeOut)) {  return 1; }
		}

	return 0;
}

int SetSoftParm(int nch, unsigned char *gain, unsigned char *chan, unsigned int rate)
{
	unsigned int i;
	unsigned short int InBuf[4] = 
		{ 
			0x0000, 	// RequestType
			4, 			// bRequest
			0, 			// wValue
			0x0 };		// wIndex
	unsigned char Buffer[32];
	unsigned short int Size=20;
	

	// пишем данные
	Buffer[0]=0xA5;
	Buffer[1]=nch;
	Buffer[2]=rate;
	Buffer[3]=(rate >> 8);
	for(i=0; i < nch; i++) Buffer[4+i]=gain[i];
	for(i=0; i < nch; i++) Buffer[4+i+nch]=chan[i];
	if(LDeviceIoControl(DIOC_SEND_COMMAND,&InBuf, sizeof(InBuf), Buffer,  Size, TimeOut))  {  return 1; }

	return 0;
}


unsigned int ChannelAd;
unsigned int ChannelGain;
unsigned int ChannelLoop;
unsigned int AdcFormat;
unsigned int RunMode;
unsigned int RunType;
unsigned int GraphType;
unsigned int SoftLoop;
unsigned int Rate=1000;
unsigned int NPoints=1000;
unsigned int SoftChannel;

int CVICALLBACK CHAN_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int i;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_CHANNEL, &ChannelAd);
			GetCtrlVal(panelHandle, PANEL_LOOP, &ChannelLoop);
			GetCtrlVal(panelHandle, PANEL_GAIN, &ChannelGain);
			GetCtrlVal(panelHandle, PANEL_LOOP_2, &SoftLoop);
			//GetCtrlVal(panelHandle, PANEL_RATE, &Rate);
			GetCtrlVal(panelHandle, PANEL_POINTS, &NPoints);
			GetCtrlVal(panelHandle, PANEL_DELAY, &InterKadrDelay);
			
			for(i=0; i < 8; i++)
				{
				GetCtrlVal(panelHandle, ChannelOnEvent[i], &ChannelOn[i]);	
				GetCtrlVal(panelHandle, ChannelGainEvent[i], &ChannelArrayGain[i]);	
				}
			
			DrawCtrl();
			break;
		}
	return 0;
}

int CVICALLBACK ADCHAN_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			RunMode^=1;
			RunType=0;
			LoopsN=0;
	//		if(ChannelLoop) SetCtrlAttribute (panelHandle, PANEL_GETADC, ATTR_LABEL_TEXT, (RunMode) ? "Stop" : "Run");
			break;
		}
	return 0;
}

int CVICALLBACK TIMER_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_TIMER_TICK:
			if(RunMode)
				{
				switch(RunType)
					{
					case 0: // однократный ввод с АЦП
						RunAdchan();
						if(!ChannelLoop) RunMode=0;
						  else 
							  {
							  sprintf(cstr, "Loop=%u", LoopsN++);
							  SetCtrlVal(panelHandle, PANEL_STATUS, cstr);  
							  }
						break;
						
					case 1: // синхронный ввод с АЦП
						RunSoft();
						if(!SoftLoop) RunMode=0;
						  else 
							  {
							  sprintf(cstr, "Loop=%u", LoopsN++);
							  SetCtrlVal(panelHandle, PANEL_STATUS, cstr);  
							  }
						break;
					}
				
				}
			break;
		}
	return 0;
}

// mode=0 adc code, mode=1 volts
double ConvertAdValue(signed short int ad_value, int gain, int mode)
{
double f;

if(!mode) return (double) ad_value;
if(mode == 1) return ad_value*GainArray[gain]/2048.;

return 0;
}


int RunAdchan(void)
{
int result;
signed short int ad_value;

//result=E154Adchan(ChannelAd, ChannelGain, &ad_value);
result=!WE154ADC_SAMPLE(pModule, &ad_value, ChannelAd | (ChannelGain << 6)); 

if(!result)
	{
	SetCtrlAttribute (panelHandle, PANEL_ADC, ATTR_PRECISION, (AdcFormat) ? 4 : 0);
	SetCtrlVal(panelHandle, PANEL_ADC, ConvertAdValue(ad_value, ChannelGain, AdcFormat));
	SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_GREEN);
	}
  else
	  {
	  SetCtrlAttribute (panelHandle, PANEL_LED1, ATTR_OFF_COLOR, VAL_RED);
	  SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_RED);
	  }
return 0;
}



int E154Adchan(int channel, int gain, signed short *ad_value)
{
	int result;
	static signed short adc=0;
	
/*	
	adc+=(channel+1)*10;
	if(adc > 2047) adc=-2047;
	*/
    result=GetAdchan(ad_value, gain, channel)	;
	return result;
}
	

void DrawCtrl(void)
{
int i;

SetCtrlVal(panelHandle, PANEL_CHANNEL, ChannelAd);
SetCtrlVal(panelHandle, PANEL_LOOP, ChannelLoop);
SetCtrlVal(panelHandle, PANEL_GAIN, ChannelGain);

switch(GraphType) 
	{
	case 0: SetCtrlVal(panelHandle, PANEL_DRAW_MODE, AdcFormat); break;
	case 1: SetCtrlVal(panelHandle, PANEL_DRAW_MODE, 1); break;
	case 2: SetCtrlVal(panelHandle, PANEL_DRAW_MODE, 0); break;
	}

SetCtrlVal(panelHandle, PANEL_GRAPH_TYPE, GraphType);
SetCtrlVal(panelHandle, PANEL_LOOP_2, SoftLoop);
//SetCtrlVal(panelHandle, PANEL_RATE, Rate);
SetCtrlVal(panelHandle, PANEL_POINTS, NPoints);
//SetCtrlVal(panelHandle, PANEL_SOFT_CHANNEL, SoftChannel);
SetCtrlVal(panelHandle, PANEL_GRAPH_COLOR, GraphColor[GraphType]);
SetCtrlVal(panelHandle, PANEL_GRID_COLOR, GridColor[GraphType]);
SetCtrlVal(panelHandle, PANEL_FON, FonColor[GraphType]);
SetCtrlVal(panelHandle, PANEL_Y_min, MinY[GraphType]);
SetCtrlVal(panelHandle, PANEL_Y_max, MaxY[GraphType]);
SetCtrlVal(panelHandle, PANEL_X_min, MinX[GraphType]);
SetCtrlVal(panelHandle, PANEL_X_max, MaxX[GraphType]);
SetCtrlVal(panelHandle, PANEL_SCALE_Y, AutoScaleY[GraphType]);
SetCtrlVal(panelHandle, PANEL_SCALE_X, AutoScaleX[GraphType]);
SetCtrlVal(panelHandle, PANEL_RATEHZ, RatekHz);
SetCtrlVal(panelHandle, PANEL_DELAY, InterKadrDelay);

			
for(i=0; i < 8; i++)
				{
				SetCtrlVal(panelHandle, ChannelOnEvent[i], ChannelOn[i]);	
				SetCtrlVal(panelHandle, ChannelGainEvent[i], ChannelArrayGain[i]);	
				}

SetCtrlAttribute (panelHandle, PANEL_Y_min, ATTR_DIMMED, AutoScaleY[GraphType]);
SetCtrlAttribute (panelHandle, PANEL_Y_max, ATTR_DIMMED, AutoScaleY[GraphType]);
SetCtrlAttribute (panelHandle, PANEL_X_min, ATTR_DIMMED, AutoScaleX[GraphType]);
SetCtrlAttribute (panelHandle, PANEL_X_max, ATTR_DIMMED, AutoScaleX[GraphType]);
SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_GRID_COLOR, GridColor[GraphType]);
SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_PLOT_BGCOLOR, FonColor[GraphType]);
SetCtrlAttribute (panelHandle, PANEL_DRAW_MODE, ATTR_DIMMED, GraphType);
SetCtrlAttribute (panelHandle, PANEL_THD, ATTR_DIMMED, (GraphType == 1) ? 0 : 1);
SetCtrlAttribute (panelHandle, PANEL_THDN, ATTR_DIMMED, (GraphType == 1) ? 0 : 1);
SetCtrlAttribute (panelHandle, PANEL_FBASE, ATTR_DIMMED, (GraphType == 1) ? 0 : 1);

if(GraphType == 2)
	{
	SetCtrlAttribute (panelHandle, PANEL_SCALE_Y, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_SCALE_X, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_Y_min, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_X_min, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_X_max, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_Y_max, ATTR_DIMMED, 1);
	}
  else
	{
	SetCtrlAttribute (panelHandle, PANEL_SCALE_Y, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelHandle, PANEL_SCALE_X, ATTR_DIMMED, 0);
	}

if(GraphType != 2) 
	SetAxisScalingMode (panelHandle, PANEL_GRAPH, VAL_BOTTOM_XAXIS, AutoScaleX[GraphType], MinX[GraphType], MaxX[GraphType]);
	SetAxisScalingMode (panelHandle, PANEL_GRAPH, VAL_LEFT_YAXIS, AutoScaleY[GraphType], MinY[GraphType], MaxY[GraphType]);

for(i=0; i < 8; i++) SetCtrlVal(panelHandle, ChanDrawEvent[i], (SoftChannel == i) ? 1 : 0);
for(i=0; i < 8; i++) SetCtrlAttribute (panelHandle, ChanDrawEvent[i], ATTR_DIMMED, !DrawChanOn[i]);
}

int CVICALLBACK SOFT_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			RunMode^=1;
			RunType=1;
			LoopsN=0;
			break;
		}
	return 0;
}


int RunSoft(void)
{
int result, i, j, nch;
signed short int ad_value;

result=E154Soft(ChannelOn, ChannelArrayGain, Rate, NPoints, Data);

if(!result)
	{
	SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_GREEN);
	
    for(i=nch=0; i < 8; i++) if(ChannelOn[i]) nch++;
	DataOn=1;
	DataNch=nch;
	DataNPoints=NPoints;
	DataRate=Rate;
	for(i=0; i < 8; i++) {DrawChanOn[i]=ChannelOn[i]; DrawGain[i]=ChannelArrayGain[i]; }
	
	DrawGraphScreen();
	DrawCtrl();
	}
  else
	  {
	  SetCtrlAttribute (panelHandle, PANEL_LED1, ATTR_OFF_COLOR, VAL_RED);
	  SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_RED);
	  }
return 0;
}

	enum {	ADC_INPUT_RANGE_5000mV_E154, ADC_INPUT_RANGE_1600mV_E154, ADC_INPUT_RANGE_500mV_E154, ADC_INPUT_RANGE_160mV_E154, INVALID_ADC_INPUT_RANGE_E154 };
	// доступные индексы источника тактовых импульсов для АЦП
	enum {	INT_ADC_CLOCK_E154, EXT_ADC_CLOCK_E154, INVALID_ADC_CLOCK_E154 };
	// доступные индексы управления трансляцией тактовых импульсов АЦП
	// на линию SYN внешнего цифрового разъёма (только при внутреннем
	// источнике тактовых импульсоц АЦП)
	enum {	ADC_CLOCK_TRANS_DISABLED_E154, ADC_CLOCK_TRANS_ENABLED_E154, INVALID_ADC_CLOCK_TRANS_E154 };
	// возможные типы синхронизации модуля E-154
	enum { 	NO_SYNC_E154, TTL_START_SYNC_E154, TTL_KADR_SYNC_E154, ANALOG_SYNC_E154, INVALID_SYNC_E154 };
	// возможные опции наличия микросхемы ЦАП
	enum {	DAC_INACCESSIBLED_E154, DAC_ACCESSIBLED_E154, INVALID_DAC_OPTION_E154 };
	// доступные индексы ревизий модуля E-154


int E154Soft(int *channel_on, int *gain, int rate, int n, signed short int *data)
{
unsigned char temp[128];
int i, nch, j, n1, n2;
int bad_offset=640;
int result;
unsigned char chan_arr[8], gain_arr[8];
struct ADC_PARS_E154 ap;
double f;
double range[4]={5.0, 1.6, 0.5, 0.16};
unsigned int  DataReadAll, RequestNumber;
	HANDLE ReadEvent[2];
	// массив OVERLAPPED структур из двух элементов
	OVERLAPPED ReadOv[2];
//	DWORD BytesTransferred[2];
	struct IO_REQUEST_LUSBAPI IoReq[2];


for(i=nch=0; i < 8; i++) if(channel_on[i]) nch++;
if(!nch) return 0;

//for(i=0; i < n; i++) for(j=0; j < nch; j++) data[i*nch+j]=1000*sin(3.14*i/200);

for(i=j=0; i < 8; i++)
	{
	if(channel_on[i])
		{
		chan_arr[j]=i | (gain[i] << 6);
		j++;
		}
	}

	if(!WE154GET_CUR_ADC_PARS(pModule, &ap)) return 1;
	
	// разберемся с синхронизацией
	switch(SynchroMode)
		{
			case 0: // no synchro
				ap.ClkSource = INT_ADC_CLOCK_E154;							// внутренний запуск АЦП
				ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// без трансляции тактовых импульсо АЦП
				ap.InputMode = NO_SYNC_E154;		// без синхронизации ввода данных
				break;
				
			case 1: // ttl
				ap.ClkSource = INT_ADC_CLOCK_E154;							// внутренний запуск АЦП
				ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// без трансляции тактовых импульсо АЦП
				ap.InputMode = TTL_START_SYNC_E154;		// без синхронизации ввода данных
				ap.SynchroAdPorog = SynchroTtlMask;		// без синхронизации ввода данных
				ap.SynchroAdChannel = SynchroTtlMode;		// без синхронизации ввода данных
				break;
				
			case 2: // analog
				ap.ClkSource = INT_ADC_CLOCK_E154;							// внутренний запуск АЦП
				ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// без трансляции тактовых импульсо АЦП
				ap.InputMode = ANALOG_SYNC_E154;		// без синхронизации ввода данных
				ap.SynchroAdType = SynchroAdMode;		// без синхронизации ввода данных
				ap.SynchroAdChannel=SynchroAdChannel | (SynchroAdGain << 6);	// канал АЦП при аналоговой синхронизации
				f=SynchroAdPorog*2047./range[SynchroAdGain];
				ap.SynchroAdPorog=f; 					// порог срабатывания АЦП при аналоговой синхронизации

				if(SynchroAdMode) ap.SynchroAdMode=!SynchroAdSubMode2; 				
				  else ap.SynchroAdMode=!SynchroAdSubMode1;

				break;
		}
	
	ap.ChannelsQuantity = nch; 		// кол-во активных канала
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)chan_arr[i];
	ap.AdcRate = RatekHz;					// частота работы АЦП в кГц
	ap.InterKadrDelay = InterKadrDelay;			// межкадровая задержка в мс
	// передадим требуемые параметры работы АЦП в модуль
	if(!WE154FILL_ADC_PARS(pModule, &ap)) return 1;

	if(!WE154GET_CUR_ADC_PARS(pModule, &ap)) return 1;
	
	sprintf(cstr, "Rate=%.2fkHz (Fadc=%.2fkHz, delay=%.3f ms", ap.KadrRate, ap.AdcRate, ap.InterKadrDelay);
	SetCtrlVal(panelHandle, PANEL_STRING, cstr);
	RealRate=ap.KadrRate;
	
	NDataBlock = (n*nch*2/DataStep)+1;
	GlobalPtr=data;
	
DataReadAll=NPoints*ap.ChannelsQuantity;
GlobalPtr=Data;

if(DataReadAll < 128*1024)
	{
   // получим число точек кратное 64 байтам (32 слова)
   n=DataReadAll/32;
   if(DataReadAll % 32) n++;
   n *= 32;

	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(!WE154STOP_ADC(pModule)) { return 1; }

	// формируем необходимые для сбора данных структуры
	// создаём событие
	ReadEvent[0] = CreateEvent(NULL, FALSE , FALSE, NULL);
	// инициализация структуры типа OVERLAPPED
	ZeroMemory(&ReadOv[0], sizeof(OVERLAPPED)); ReadOv[0].hEvent = ReadEvent[0];
	// формируем структуру IoReq
	IoReq[0].Buffer = data;
	IoReq[0].NumberOfWordsToPass = n;
	IoReq[0].NumberOfWordsPassed = 0x0;
	IoReq[0].Overlapped = &ReadOv[0];
	IoReq[0].TimeOut = n/ap.AdcRate + 60000;

	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!WE154ReadData(pModule, &IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]);  return 0x0; }

	// запустим АЦП
	if(WE154START_ADC(pModule))
	{
	// ждём завершения операции сбора предыдущей порции данных
	if(WaitForSingleObject(ReadEvent[0], IoReq[0].TimeOut) == WAIT_TIMEOUT) 
		{ ReadThreadErrorNumber = 0x3; }
	}
	else { ReadThreadErrorNumber = 0x6; }
		  
	// остановим работу АЦП
	if(!WE154STOP_ADC(pModule)) ReadThreadErrorNumber = 0x1;
	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(WE154GetModuleHandle(pModule))) { ReadThreadErrorNumber = 0x7; }
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x1; i++) CloseHandle(ReadEvent[i]);
	// небольшая задержка
  //	Sleep(100);
	// установим флажок завершения работы потока сбора данных
	// теперь можно спокойно выходить из потока

   return (ReadThreadErrorNumber) ? 1 :  0;
   }
	
	
	// Создаём и запускаем поток сбора данных
	hReadThread = CreateThread(0, 0x2000, ServiceReadThread, 0, 0, &ReadTid);
	if(!hReadThread) return 1;
	
	
	while(!IsReadThreadComplete)
	{
	 Sleep(20);
	 ProcessSystemEvents ();
	}

	// ждём окончания работы потока ввода данных
	WaitForSingleObject(hReadThread, INFINITE);

	// проверим была ли ошибка выполнения потока сбора данных
	if(ReadThreadErrorNumber) 
		return 1;
	
	return 0;
	
/*
result=SetSoftParm(nch, gain_arr, chan_arr, rate); 
if(result) return result;

if(LDeviceIoControl(DIOC_RESET_PIPE3, NULL, 0, NULL, 0, TimeOut)) {  return 1; }

 result=SetStartStop(1) ;
 if(result) return result;

n2=n*nch*2;
n2 /= 64;
n2 *= 64;
n2 += 64;
n2 += bad_offset;

n1=n2;
result=SyncReadData((unsigned char *) data, &n1, 100000);
if(!result) return 1;


 result=SetStartStop(0) ;
 if(result) return result;
 
 Delay(0.1);
 
if(n1 != n2)  return 1;

n1=128;
result=SyncReadData((unsigned char *) temp, &n1, 100000);
if(!result) return 1;

if(n1) return 1;
*/  
	return 0;
}




int CVICALLBACK GET_TTL_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int result, i, j, nch, ttl;
signed short int ad_value;
short ttl_short;


	switch (event)
		{
		case EVENT_COMMIT:

//result=E154TtlIn(&ttl);
result=!WE154TTL_IN(pModule, &ttl_short);
ttl=ttl_short;
			
if(!result)
	{
	SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_GREEN);
	SetCtrlVal(panelHandle, PANEL_STATUS, "Цифровые линии считаны успешно");
	
    for(i=0; i < 8; i++) SetCtrlVal(panelHandle, TtlInEvent[i], ttl & (1 << i));
		
	}
  else
	  {
	  SetCtrlAttribute (panelHandle, PANEL_LED1, ATTR_OFF_COLOR, VAL_RED);
	  SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_RED);
	  }
			break;
		}
	return 0;
}

int CVICALLBACK PUT_TTL_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int result, i, j, ttl;

	switch (event)
		{
		case EVENT_COMMIT:

for(i=ttl=0; i < 8; i++) 
	{
	GetCtrlVal(panelHandle, TtlOutEvent[i], &j);
	if(j) ttl |= (1 << i);
	}


//result=E154TtlOut(ttl);
result=!WE154TTL_OUT(pModule, ttl);

if(!result)
	{
	SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_GREEN);
	SetCtrlVal(panelHandle, PANEL_STATUS, "Цифровые линии установлены успешно");
	}
  else
	  {
	  SetCtrlAttribute (panelHandle, PANEL_LED1, ATTR_OFF_COLOR, VAL_RED);
	  SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_RED);
	  }
			break;
		}
	return 0;
}

int CVICALLBACK DAC_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int result, i, j, dac;
double f;
short dac_short;
WORD chan=0;

	switch (event)
		{
		case EVENT_COMMIT:

	GetCtrlVal(panelHandle, PANEL_DAC, &f);
	f += 3.3;
	dac=f*255/6.6;
	dac_short=dac-128;
	result=!WE154DAC_SAMPLE(pModule, &dac_short, 0);  
if(!result)
	{
	SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_GREEN);
	SetCtrlVal(panelHandle, PANEL_STATUS, "ЦАП установлен успешно");
	}
  else
	  {
	  SetCtrlAttribute (panelHandle, PANEL_LED1, ATTR_OFF_COLOR, VAL_RED);
	  SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_RED);
	  }
			break;
		}
	return 0;
}

int CVICALLBACK GRAPH_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_FON, &FonColor[GraphType]);
			GetCtrlVal(panelHandle, PANEL_GRID_COLOR, &GridColor[GraphType]);
			GetCtrlVal(panelHandle, PANEL_SCALE_Y, &AutoScaleY[GraphType]);
			GetCtrlVal(panelHandle, PANEL_SCALE_X, &AutoScaleX[GraphType]);
			GetCtrlVal(panelHandle, PANEL_Y_min, &MinY[GraphType]);
			GetCtrlVal(panelHandle, PANEL_Y_max, &MaxY[GraphType]);
			GetCtrlVal(panelHandle, PANEL_X_min, &MinX[GraphType]);
			GetCtrlVal(panelHandle, PANEL_X_max, &MaxX[GraphType]);

			DrawCtrl();
			break;
		}
	return 0;
}


void SaveParm(void)
{
FILE *fp;

fp=fopen(cfg_file, "wb");
if(fp == NULL) { MessagePopup("ERROR", "Can't open file for saving"); return; }
				   
fwrite(&ChannelAd, 4, 1, fp);
fwrite(&ChannelLoop, 4, 1, fp);
fwrite(&ChannelGain, 4, 1, fp);
fwrite(&AdcFormat, 4, 1, fp);
fwrite(&GraphType, 4, 1, fp);
fwrite(&SoftLoop, 4, 1, fp);
fwrite(&Rate, 4, 1, fp);
fwrite(&NPoints, 4, 1, fp);
fwrite(&SoftChannel, 4, 1, fp);
fwrite(GraphColor, 4, 3, fp);
fwrite(GridColor, 4, 3, fp);
fwrite(FonColor, 4, 3, fp);
fwrite(MinY, 8, 3, fp);
fwrite(MaxY, 8, 3, fp);
fwrite(MinX, 8, 3, fp);
fwrite(MaxX, 8, 3, fp);
fwrite(AutoScaleY, 4, 3, fp);
fwrite(AutoScaleX, 4, 3, fp);
			
fwrite(&ChannelOn[0], 4, 8, fp);	
fwrite(&ChannelArrayGain[0], 4, 8, fp);	

fwrite(&SynchroMode, 4, 1, fp);
fwrite(&SynchroTtlMask, 4, 1, fp);
fwrite(&SynchroTtlMode, 4, 1, fp);
fwrite(&SynchroAdChannel, 4, 1, fp);
fwrite(&SynchroAdGain, 4, 1, fp);
fwrite(&SynchroAdMode, 4, 1, fp);
fwrite(&SynchroAdSubMode1, 4, 1, fp);
fwrite(&SynchroAdSubMode2, 4, 1, fp);
fwrite(&SynchroAdPorog, 8, 1, fp);

fwrite(&RatekHz, 8, 1, fp);
fwrite(&InterKadrDelay, 8, 1, fp);

fclose(fp);


MessagePopup("Информация", "Сохранение прошло успешно");
}

void LoadParm(void)
{
FILE *fp;

fp=fopen(cfg_file, "rb");
if(fp == NULL) { MessagePopup("ERROR", "Can't open file for reading"); return; }
				   
fread(&ChannelAd, 4, 1, fp);
fread(&ChannelLoop, 4, 1, fp);
fread(&ChannelGain, 4, 1, fp);
fread(&AdcFormat, 4, 1, fp);
fread(&GraphType, 4, 1, fp);
fread(&SoftLoop, 4, 1, fp);
fread(&Rate, 4, 1, fp);
fread(&NPoints, 4, 1, fp);
fread(&SoftChannel, 4, 1, fp);
fread(GraphColor, 4, 3, fp);
fread(GridColor, 4, 3, fp);
fread(FonColor, 4, 3, fp);
fread(MinY, 8, 3, fp);
fread(MaxY, 8, 3, fp);
fread(MinX, 8, 3, fp);
fread(MaxX, 8, 3, fp);
fread(AutoScaleY, 4, 3, fp);
fread(AutoScaleX, 4, 3, fp);
			
fread(&ChannelOn[0], 4, 8, fp);	
fread(&ChannelArrayGain[0], 4, 8, fp);	

fread(&SynchroMode, 4, 1, fp);
fread(&SynchroTtlMask, 4, 1, fp);
fread(&SynchroTtlMode, 4, 1, fp);
fread(&SynchroAdChannel, 4, 1, fp);
fread(&SynchroAdGain, 4, 1, fp);
fread(&SynchroAdMode, 4, 1, fp);
fread(&SynchroAdSubMode1, 4, 1, fp);
fread(&SynchroAdSubMode2, 4, 1, fp);
fread(&SynchroAdPorog, 8, 1, fp);

fread(&RatekHz, 8, 1, fp);
fread(&InterKadrDelay, 8, 1, fp);

fclose(fp);

}


int CVICALLBACK SAVE_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			SaveParm();
			break;
		}
	return 0;
}


// freq in    Hz
int DrawSpektr(int handle, int ctrl, double *data, int n_points, double freq)
{
int i, j, k, n, index;						  
WindowConst WinConst;   
double df;
char    Unit[20]="V", cstr[200];  
double fundamental_frequency, f, max, sum_max;

//freq=CalculateFreq(DataNch, DataRate);
freq=RealRate;
df=freq/(n_points);
	
  for(i=0; i < n_points; i++) ArrayXRe[i] = data[i];
  for(i=0; i < n_points; i++) ArrayXIm[i] = 0;
  
  // find SNR
 // BlkHarrisWin (ArrayXRe, n_points);
  FillBpfWindow(Window, n_points,  5);
  for(i=0; i < n_points; i++) { ArrayXRe[i] = (double)ArrayXRe[i]*Window[i];  }
  

  FFT (ArrayXRe, ArrayXIm, n_points);
  for(i=0; i < n_points/2; i++) Conv_Spectrum[i]=sqrt(ArrayXRe[i]*ArrayXRe[i] + ArrayXIm[i]*ArrayXIm[i])/(n_points / 2.);
  for(i=0, max=0; i < n_points/2; i++) if(Conv_Spectrum[i] > max) max=Conv_Spectrum[i];
  FingSNR(Conv_Spectrum, n_points/2, max);

  SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_YNAME, "дБ");
  SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XNAME, "Гц");

for(i=0; i < n_points/2; i++)  Conv_Spectrum[i]=20.*log10(Conv_Spectrum[i]/max);

PlotWaveform (handle, ctrl, Conv_Spectrum, n_points/2, VAL_DOUBLE, 1.0, 0.0, 0.0, df, VAL_VERTICAL_BAR, 
	VAL_EMPTY_SQUARE, VAL_SOLID, 1,GraphColor[GraphType]);
SetCtrlVal(panelHandle, PANEL_THDN, SNR_parm);

/*
for(i=0; i < n_points/2; i++) Conv_Spectrum[i]=0;

freq=CalculateFreq(DataNch, DataRate);

// займемся математикой
ScaledWindow (data, n_points, 7, &WinConst);

AutoPowerSpectrum (data, n_points, 1./freq, Auto_Spectrum, &df);

SpectrumUnitConversion (Auto_Spectrum, n_points/2, 0, 1, 0, df, WinConst, Conv_Spectrum, Unit);

SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_YNAME, "дБ");
SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XNAME, "Гц");


PlotWaveform (handle, ctrl, Conv_Spectrum, n_points/2, VAL_DOUBLE, 1.0, 0.0, 0.0, df, VAL_VERTICAL_BAR, 
	VAL_EMPTY_SQUARE, VAL_SOLID, 1,GraphColor[GraphType]);


for(i=index=5; i < n_points/2; i++) if(Conv_Spectrum[i] > Conv_Spectrum[index]) index=i;

fundamental_frequency=index*freq/(n_points); 

//for(i=0; i < 5; i++)  Auto_Spectrum[i]=0;
HarmonicAnalyzer (Auto_Spectrum, n_points/2, n_points, 5, 3, freq, fundamental_frequency, harmonic_amplitudes,
				  harmonic_frequencies, &Thd, &Thdnoise);

SetCtrlVal(panelHandle, PANEL_THD, 20.*log(Thd/100.));
SetCtrlVal(panelHandle, PANEL_THDN, 20.*log(Thdnoise/100.));
SetCtrlVal(panelHandle, PANEL_FBASE, harmonic_frequencies[0]);

SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_NUM_CURSORS, 5);

    for(i = 0; i < 5; i++)
    	{
        SetCursorAttribute (panelHandle, PANEL_GRAPH, i+1, ATTR_CURSOR_POINT_STYLE, VAL_SOLID_CIRCLE);
        SetCursorAttribute (panelHandle, PANEL_GRAPH, i+1, ATTR_CROSS_HAIR_STYLE, VAL_VERTICAL_LINE);
        SetCursorAttribute (panelHandle, PANEL_GRAPH, i+1, ATTR_CURSOR_COLOR, VAL_BLUE);
		SetCursorAttribute (panelHandle, PANEL_GRAPH, i+1, ATTR_CURSOR_MODE, VAL_SNAP_TO_POINT);
        SetActiveGraphCursor (panelHandle, PANEL_GRAPH, i+1);
		
		j=harmonic_frequencies[i]/df;
        SetGraphCursor (panelHandle, PANEL_GRAPH, i+1, harmonic_frequencies[i],
                    Conv_Spectrum[j]);
    	}
*/

return 0;
}


int Gluks;

int DrawGraphScreen(void)
{
int result, i, j, nch, chan_index;
signed short int ad_value;

if(!DataOn) return 0;


DeleteGraphPlot (panelHandle, PANEL_GRAPH, -1, VAL_DELAYED_DRAW);

if(!DrawChanOn[SoftChannel]) 
	{
	DeleteGraphPlot (panelHandle, PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
	return 0;
	}

for(i=chan_index=0; i < 8; i++) if(i == SoftChannel) break; else if(DrawChanOn[i]) chan_index++;
chan_index=SoftChannel;
for(i=0; i < DataNPoints; i++) DataDouble[i]=Data[i*DataNch+chan_index];

for(i=0; i < DataNPoints-1; i++) if(fabs(DataDouble[i]-DataDouble[i+1]) > 100) { Gluks++;  break; }
SetCtrlVal(panelHandle, PANEL_GLUKS, Gluks);
	
switch(GraphType)
		{
		case 0:	// осциллограф
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_NUM_CURSORS, 0);
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XDIVISIONS, VAL_AUTO);	
			for(i=0; i <  DataNPoints; i++) DataDouble[i]=ConvertAdValue(DataDouble[i], DrawGain[SoftChannel], AdcFormat);
			
			ShowAcDc(DataDouble,  DataNPoints);
			
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_YNAME, (AdcFormat) ? "Вольты" : "Коды АЦП");
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XNAME, "Отсчеты");
			
			PlotWaveform (panelHandle, PANEL_GRAPH, DataDouble, DataNPoints, VAL_DOUBLE, 1.0, 0.0, 0.0, 1.0, VAL_CONNECTED_POINTS,
						  VAL_EMPTY_SQUARE, VAL_SOLID, 1, GraphColor[GraphType]);
			break;
			
		case 1:	// спектр
			for(i=0; i <  DataNPoints; i++) DataDouble[i]=ConvertAdValue(DataDouble[i], DrawGain[SoftChannel], 0);
			
//for(i=0; i <  DataNPoints; i++) DataDouble[i]=5.0*sin(3.14*i/20)+Random (0.0, 0.5);
			ShowAcDc(DataDouble,  DataNPoints);
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XDIVISIONS, VAL_AUTO);
			DrawSpektr(panelHandle, PANEL_GRAPH, DataDouble, DataNPoints, 1.0);
			break;
			
		case 2:	// гистограмма
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_NUM_CURSORS, 0);
			for(i=0; i <  DataNPoints; i++) DataDouble[i]=ConvertAdValue(DataDouble[i], DrawGain[SoftChannel], 0);
			
			ShowAcDc(DataDouble,  DataNPoints);
			
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_YNAME, "Гистограмма");
			SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XNAME, "Коды АЦП");
			DrawHyst(panelHandle, PANEL_GRAPH, DataDouble, DataNPoints, 1.0);
			break;
		}
		
return 0;
}

int CVICALLBACK GRAPH_REDRAW_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_GRAPH_TYPE, &GraphType);
			DrawGraphScreen();
			DrawCtrl();
			break;
		}
	return 0;
}

int CVICALLBACK GRAPH1_REDRAW_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			//GetCtrlVal(panelHandle, PANEL_SOFT_CHANNEL, &SoftChannel);
			if(!GraphType) GetCtrlVal(panelHandle, PANEL_DRAW_MODE, &AdcFormat);
			GetCtrlVal(panelHandle, PANEL_GRAPH_COLOR, &GraphColor[GraphType]);
			DrawGraphScreen();
			DrawCtrl();
			break;
		}
	return 0;
}

void ShowAcDc(double *data, int n)
{
ACDCEstimator (data, n, &Ac, &Dc);
StdDev (data, n, &Mx, &Dx);

SetCtrlVal(panelHandle, PANEL_AC, Ac);
SetCtrlVal(panelHandle, PANEL_DC, Dc);
SetCtrlVal(panelHandle, PANEL_MX, Mx);
SetCtrlVal(panelHandle, PANEL_DX, Dx);
}


int DrawHyst(int handle, int ctrl, double *data, int n_points, double freq)
{
int i, j, k, n, index,intervals, middle;						  
WindowConst WinConst;   
double mean, min, max;
char    Unit[20]="V", cstr[200];  
double fundamental_frequency, f;
double axis[100];
int hist[100];
int max_index, min_index;


intervals=10;

Mean (data, n_points, &mean);
MaxMin1D(data,n_points, &max, &max_index, &min, &min_index); 
if(min == max) {min--; max++; }
middle=(max+min)/2;
if(max-min < 10) 
	{
	{min=middle-5; max=middle+5;}
	SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XDIVISIONS, 10);
	SetAxisScalingMode (panelHandle, PANEL_GRAPH, VAL_BOTTOM_XAXIS, VAL_MANUAL,middle-5.0, middle+5.0);

	}
  else
  {
  SetAxisScalingMode (panelHandle, PANEL_GRAPH, VAL_BOTTOM_XAXIS, VAL_AUTOSCALE, middle-3.0, middle+5.0);
  SetCtrlAttribute (panelHandle, PANEL_GRAPH, ATTR_XDIVISIONS, VAL_AUTO);
  }
Histogram (data, n_points, min, max, hist, axis, intervals);
for(i=0; i < intervals; i++) axis[i] += 0.5;
PlotXY (panelHandle, PANEL_GRAPH, axis, hist, intervals, VAL_DOUBLE, VAL_INTEGER, 
		VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, 1, GraphColor[GraphType]);


return 0;
}

int CVICALLBACK SAVE_FILE_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
char fname[512];
FILE *fp;
int r;

	switch (event)
		{
		case EVENT_COMMIT:
			if(!DataOn)
				{
				MessagePopup("ОШИБКА", "Нечего сохранять");
				break;
				}
			r = FileSelectPopup ("", "*.dat", "", "Выберите имя файла", VAL_SAVE_BUTTON, 0, 1, 1, 1, fname);
			if(r == VAL_NO_FILE_SELECTED) break;
			
			fp=fopen(fname, "wb");
			if(fp == NULL)
				{
				MessagePopup("ОШИБКА", "Не могу открыть файл для записи");
				break;
				}
			
			fwrite(&DataOn, 4, 1, fp);
			fwrite(&DataNch, 4, 1, fp);
			fwrite(&DataNPoints, 4, 1, fp);
			fwrite(&DataRate, 4, 1, fp);
			fwrite(DrawChanOn, 4, 8, fp);
			fwrite(DrawGain, 4, 8, fp);
			fwrite(Data, 2, DataNPoints*DataNch, fp);
			
			fclose(fp);
			break;
		}
	return 0;
}

int CVICALLBACK LOAD_FILE_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
char fname[512];
FILE *fp;
int r;

	switch (event)
		{
		case EVENT_COMMIT:
			r = FileSelectPopup ("", "*.dat", "", "Выберите файл", VAL_LOAD_BUTTON, 0, 1, 1, 0, fname);
			if(r == VAL_NO_FILE_SELECTED) break;
			
			fp=fopen(fname, "rb");
			if(fp == NULL)
				{
				MessagePopup("ОШИБКА", "Не могу открыть файл");
				break;
				}
			
			fread(&DataOn, 4, 1, fp);
			fread(&DataNch, 4, 1, fp);
			fread(&DataNPoints, 4, 1, fp);
			fread(&DataRate, 4, 1, fp);
			fread(DrawChanOn, 4, 8, fp);
			fread(DrawGain, 4, 8, fp);
			fread(Data, 2, DataNPoints*DataNch, fp);
			
			fclose(fp);
			
			DrawGraphScreen();
			DrawCtrl();
			break;
		}
	return 0;
}

int CVICALLBACK CHANDRAW_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int i, j, n;
	
	switch (event)
		{
		case EVENT_COMMIT:
			for(n=0; n < 8; n++) if(control  ==  ChanDrawEvent[n]) break;
			if(n==8) break;
			
			SoftChannel = n;
			
			DrawGraphScreen();
			DrawCtrl();
			break;
		}
	return 0;
}

/*
17 -> 50 Hz


rate=ADCrate*nch
*/
double CalculateFreq(int nch, int rate)
{
double f;

f=24000000./rate;	// adc freq in Hz
f /= nch;



return f;
}


int CVICALLBACK RATE_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
double f;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_RATEHZ, &f);
			RatekHz=f;
			DrawCtrl();
			break;
		}
	return 0;
}



int E154SoftTestStart(int *channel_on, int *gain, int rate, int n, signed short int *data);
int E154SoftTestStop(int *channel_on, int *gain, int rate, int n, signed short int *data) ;

int CVICALLBACK TEST_RUN_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int result, i, j, nch;
signed short int ad_value;

result=E154SoftTestStart(ChannelOn, ChannelArrayGain, Rate, NPoints, Data);

if(!result)
	{
	SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_GREEN);
	
	}
  else
	  {
	  SetCtrlAttribute (panelHandle, PANEL_LED1, ATTR_OFF_COLOR, VAL_RED);
	  SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_RED);
	  }
return 0;
}

int CVICALLBACK TEST_STOP_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int result, i, j, nch;
signed short int ad_value;


result=E154SoftTestStop(ChannelOn, ChannelArrayGain, Rate, NPoints, Data);

if(!result)
	{
	SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_GREEN);
	
	}
  else
	  {
	  SetCtrlAttribute (panelHandle, PANEL_LED1, ATTR_OFF_COLOR, VAL_RED);
	  SetCtrlAttribute (panelHandle, PANEL_LED1_2, ATTR_OFF_COLOR, VAL_RED);
	  }
return 0;
}


int E154SoftTestStart(int *channel_on, int *gain, int rate, int n, signed short int *data)
{
int i, nch, j, n1, n2;
int result;
unsigned char chan_arr[8], gain_arr[8];


for(i=nch=0; i < 8; i++) if(channel_on[i]) nch++;
if(!nch) return 0;

//for(i=0; i < n; i++) for(j=0; j < nch; j++) data[i*nch+j]=1000*sin(3.14*i/200);

for(i=j=0; i < 8; i++)
	{
	if(channel_on[i])
		{
		chan_arr[j]=i;
		gain_arr[j]=gain[i];
		j++;
		}
	}
result=SetSoftParm(nch, gain_arr, chan_arr, rate); 
if(result) return result;

if(LDeviceIoControl(DIOC_RESET_PIPE3, NULL, 0, NULL, 0, TimeOut)) {  return 1; }

 result=SetStartStop(1) ;
 if(result) return result;
	return 0;
}

int E154SoftTestStop(int *channel_on, int *gain, int rate, int n, signed short int *data)
{
int i, nch, j, n1, n2;
int result;
unsigned char chan_arr[8], gain_arr[8];




 result=SetStartStop(0) ;
 if(result) return result;
 
 Delay(0.1);
 

	return 0;
}

#define MAX_PEAK_WIDTH 5
int FingSNR(double *ptr, int N, float max)
{
int i, j, k, n, index, l=0, r=0;
double f, f1, f2, f3, lmax, f_max=0;

  for(i=lmax=index=0; i < N; i++) if(ptr[i] > lmax) { lmax=ptr[i]; index=i; }

  for(f=lmax, l=r=i=index; ;)
    {
    if(l > 0) { l--; f+=ptr[l]; }
    if(r < N-1) { r++; f+=ptr[r]; }

    if(l <=0 && r >= N-1) break;
    if(l < index-MAX_PEAK_WIDTH) break;
    if(r > index+MAX_PEAK_WIDTH) break;
    }

#define START_NUM 2
  for(f=0, i=START_NUM; i < N; i++) if(i < l || i > r) f+=ptr[i]*ptr[i];
    else f_max += ptr[i]*ptr[i];
  f_max=sqrt(f_max);
  f=sqrt(f);

if(f < 0.00000001) f=0.0000001;
//f=0.33979;
if(f_max/f < 0.000001)
  {
  SNR_parm=-1;
  EOB_parm=-1;
  return 0;
  }

SNR_parm=20.*log10(f_max/f);
EOB_parm=(SNR_parm-1.76)/6.02;

if(EOB_parm < 8)
  i=5;
  
return 0;  
}

int FingSNRonground(double *ptr, int N, float max)
{
int i, j, k, n, index, l=0, r=0;
double f, f1, f2, f3, lmax, f_max=0;

for(f=0, i=START_NUM; i < N; i++)  f+=ptr[i]*ptr[i];
f=sqrt(f);

if(f < 0.00000001) f=0.0000001;

if(max/f < 0.000001)
  {
  SNR_parm=-1;
  EOB_parm=-1;
  return 0;
  }

SNR_parm=20.*log10(max/f);
EOB_parm=(SNR_parm-1.76)/6.02;

if(EOB_parm < 8)
  i=5;
  
return 0;  
}


long calc_one_turn(int *Ptr, int N, double *f1, double *a1)
{
int i, j, sg, key;
double f, a, arr[4], x, y, x_old;
long cnt;

for(i=3, key=a=cnt=f=0; i < N-3; i++)
	{
	if(!Ptr[i])
		{
		x=i;
		if(!key) { key=1; x_old=x; continue; }
		f+=(x-x_old);
		cnt++;
    x_old=x;
		continue;
		}
	if((long)Ptr[i]*(long)Ptr[i+1] < 0)
		{
		for(j=0; j < 4; j++) arr[j]=Ptr[i+j-1];
		x=zero_search(arr)+i-1;
		if(!key) { x_old=x; key=1; continue; }
		f+=(x-x_old);
		x_old=x;
		cnt++;
		}
	}

*f1=f/cnt;
*a1=a/cnt;

return cnt;
}

double zero_search(double *arr)
{
int i;
double d=0.001, f, x, xl, xr, fl, fr, y0, y1, y2, y3;

xl=1;
xr=2;
fl=y1=arr[1];
fr=y2=arr[2];
y0=arr[0];
y3=arr[3];


do
	{
	x=(xr+xl)/2.;

	// function begins
	f=y0*(1.-x)*(x-2.)*(x-3.)/6. + y1*x*(x-2)*(x-3)/2. - y2*x*(x-1.)*(x-3.)/2.+
		y3*x*(x-1.)*(x-2.)/6.;
	// function ends
	if(f*fl > 0) { fl=f; xl=x; }
			else     { fr=f; xr=x; }

	} while (fabs(f) > 0.001);

return x;
}



FillBpfWindow(double *ptr, int n, int type)
{
int i;

switch(type)
  {
  case RECT_WINDOW:
	for(i=0; i < n; i++)  ptr[i]=1.0;
	break;

  case HANN_WINDOW:
	for(i=0; i < n; i++)  ptr[i]=0.5*(1.-cos(2.*M_PI*(double)i/(double)n));
	break;

  case HAMMING_WINDOW:
	for(i=0; i < n; i++)  ptr[i]=0.54-0.46*cos(2.*M_PI*(double)i/(double)n);
	break;

  case BLACKMAN_WINDOW:
	for(i=0; i < n; i++) ptr[i]=0.42-0.5*cos(2.*M_PI*(double)i/(double)n)+
				 0.08*cos(4.*M_PI*(double)i/(double)n);
	break;

  case BARLETTE_WINDOW:
	for(i=0; i < n; i++) ptr[i]=(i <= (n-1)/2) ? 2.*(double)i/(double)n
				 : 2.-2.*(double)i/(double)n;
	break;

  case BLACKMAN_HARRIS_WINDOW:
	for(i=0; i < n; i++) ptr[i]=0.35875-0.48829*cos(2.*M_PI*(double)i/
				 (double)n)+0.14128*cos(4.*M_PI*(double)i/
				 (double)n)-0.01168*cos(6.*M_PI*(double)i/
				 (double)n);
	break;
  }
return 0;
}



//------------------------------------------------------------------------
// Поток, в котором осуществляется сбор данных
//------------------------------------------------------------------------
static DWORD WINAPI  ServiceReadThread(LPVOID lpp) 
{
	WORD RequestNumber;
	DWORD i;
	// идентификатор массива их двух событий
	HANDLE ReadEvent[2];
	// массив OVERLAPPED структур из двух элементов
	OVERLAPPED ReadOv[2];
//	DWORD BytesTransferred[2];
	struct IO_REQUEST_LUSBAPI IoReq[2];

	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(!WE154STOP_ADC(pModule)) { ReadThreadErrorNumber = 0x1; IsReadThreadComplete = TRUE; return 0x0; }
 	Sleep(100);

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
		IoReq[i].TimeOut = DataStep/RatekHz + 2000;
	}
		
	// делаем предварительный запрос на ввод данных
	RequestNumber = 0x0;
	if(!WE154ReadData(pModule, &IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = TRUE; return 0x0; }

	// запустим АЦП
	if(WE154START_ADC(pModule))
	{
		// цикл сбора данных
		for(i = 0x1; i < NDataBlock; i++)
		{
			// сделаем запрос на очередную порции данных
			RequestNumber ^= 0x1;
			if(!WE154ReadData(pModule, &IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }

			// ждём завершения операции сбора предыдущей порции данных
			if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) 
				{ ReadThreadErrorNumber = 0x3; break; }

			// запишем полученную порцию данных в файл
			memcpy(&GlobalPtr[(i-1) * DataStep], IoReq[RequestNumber^0x1].Buffer, 2*DataStep);
			
			//if(!WriteFile(	hFile,													// handle to file to write to
		    //					IoReq[RequestNumber^0x1].Buffer,					// pointer to data to write to file
			//					2*DataStep,	 											// number of bytes to write
	    	//					&FileBytesWritten,									// pointer to number of bytes written
			//			   	NULL			  											// pointer to structure needed for overlapped I/O
			//				   )) { ReadThreadErrorNumber = 0x4; break; }

			if(ReadThreadErrorNumber) break;
					  
			else if(StopCommandFlag) { StopCommandFlag=0; ReadThreadErrorNumber = 0x5; break; }
			
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
			memcpy(&GlobalPtr[(i-1) * DataStep], IoReq[RequestNumber^0x1].Buffer, 2*DataStep);
	//		if(!WriteFile(	hFile,													// handle to file to write to
	//	    					IoReq[RequestNumber^0x1].Buffer,					// pointer to data to write to file
	//							2*DataStep,	 											// number of bytes to write
	  //  						&FileBytesWritten,									// pointer to number of bytes written
		//				   	NULL			  											// pointer to structure needed for overlapped I/O
		//					   )) ReadThreadErrorNumber = 0x4;
			Counter++;
		}
	}
	else { ReadThreadErrorNumber = 0x6; }

	// остановим работу АЦП
	if(!WE154STOP_ADC(pModule)) ReadThreadErrorNumber = 0x1;
	// прервём возможно незавершённый асинхронный запрос на приём данных
	if(!CancelIo(WE154GetModuleHandle(pModule))) { ReadThreadErrorNumber = 0x7; }
	// освободим все идентификаторы событий
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadEvent[i]);
	// небольшая задержка
	Sleep(100);
	// установим флажок завершения работы потока сбора данных
	IsReadThreadComplete = TRUE;
						
	// теперь можно спокойно выходить из потока
	return 0x0;
}


int CVICALLBACK SYNCHRO_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SynchroPanel();
			break;
	}
	return 0;
}

int CVICALLBACK STOP_S_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			StopCommandFlag=1;
			break;
	}
	return 0;
}

	
int E154SoftForever(int *channel_on, int *gain, int rate, int n, signed short int *data)
{
unsigned char temp[128];
int i, nch, j, n1, n2;
int pnl, cnt;
int bad_offset=640;
int result;
unsigned char chan_arr[8], gain_arr[8];
struct ADC_PARS_E154 ap;
double f;
double range[4]={5.0, 1.6, 0.5, 0.16};


for(i=nch=0; i < 8; i++) if(channel_on[i]) nch++;
if(!nch) return 0;

//for(i=0; i < n; i++) for(j=0; j < nch; j++) data[i*nch+j]=1000*sin(3.14*i/200);

for(i=j=0; i < 8; i++)
	{
	if(channel_on[i])
		{
		chan_arr[j]=i | (gain[i] << 6);
		j++;
		}
	}

	if(!WE154GET_CUR_ADC_PARS(pModule, &ap)) return 1;
	
	// разберемся с синхронизацией
	switch(SynchroMode)
		{
			case 0: // no synchro
				ap.ClkSource = INT_ADC_CLOCK_E154;							// внутренний запуск АЦП
				ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// без трансляции тактовых импульсо АЦП
				ap.InputMode = NO_SYNC_E154;		// без синхронизации ввода данных
				break;
				
			case 1: // ttl
				ap.ClkSource = INT_ADC_CLOCK_E154;							// внутренний запуск АЦП
				ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// без трансляции тактовых импульсо АЦП
				ap.InputMode = TTL_START_SYNC_E154;		// без синхронизации ввода данных
				ap.SynchroAdPorog = SynchroTtlMask;		// без синхронизации ввода данных
				ap.SynchroAdChannel = SynchroTtlMode;		// без синхронизации ввода данных
				break;
				
			case 2: // analog
				ap.ClkSource = INT_ADC_CLOCK_E154;							// внутренний запуск АЦП
				ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// без трансляции тактовых импульсо АЦП
				ap.InputMode = ANALOG_SYNC_E154;		// без синхронизации ввода данных
				ap.SynchroAdType = SynchroAdMode;		// без синхронизации ввода данных
				ap.SynchroAdChannel=SynchroAdChannel | (SynchroAdGain << 6);	// канал АЦП при аналоговой синхронизации
				f=SynchroAdPorog*2047./range[SynchroAdGain];
				ap.SynchroAdPorog=f; 					// порог срабатывания АЦП при аналоговой синхронизации

				if(SynchroAdMode) ap.SynchroAdMode=!SynchroAdSubMode2; 				
				  else ap.SynchroAdMode=!SynchroAdSubMode1;

				break;
		}
	
	ap.ChannelsQuantity = nch; 		// кол-во активных канала
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)chan_arr[i];
	ap.AdcRate = RatekHz;					// частота работы АЦП в кГц
	ap.InterKadrDelay = InterKadrDelay;			// межкадровая задержка в мс
	// передадим требуемые параметры работы АЦП в модуль
	if(!WE154FILL_ADC_PARS(pModule, &ap)) return 1;

	if(!WE154GET_CUR_ADC_PARS(pModule, &ap)) return 1;
	
	sprintf(cstr, "Rate=%.2fkHz (Fadc=%.2fkHz, delay=%.3f ms", ap.KadrRate, ap.AdcRate, ap.InterKadrDelay);
	SetCtrlVal(panelHandle, PANEL_STRING, cstr);
	RealRate=ap.KadrRate;
	
	NDataBlock = (n*nch*2/DataStep)+1;
	GlobalPtr=data;
	
	
WE154START_ADC(pModule);
	
GetUserEvent (1, &pnl, &cnt);
			

WE154STOP_ADC(pModule);

	return 0;
}

	
	
	
int CVICALLBACK SOFTDEBUG_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
unsigned char temp[128];
int i, nch, j, n1, n2;
int bad_offset=640;
int result;
unsigned char chan_arr[8], gain_arr[8];
struct ADC_PARS_E154 ap;
double f;
double range[4]={5.0, 1.6, 0.5, 0.16};

switch (event)
	{
		case EVENT_COMMIT:
			E154SoftForever(ChannelOn, ChannelArrayGain, Rate, NPoints, Data);
			break;
	}
	return 0;
}
