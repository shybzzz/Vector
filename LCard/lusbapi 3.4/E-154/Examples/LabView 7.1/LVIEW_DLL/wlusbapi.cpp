#include <windows.h>
#include "lusbapi.h"
#include "wlusbapi.h"
#pragma argsused

static ILE154 *pModule;
// ���������� ����������
static HANDLE ModuleHandle;
// �������� ������
static char ModuleName[7];
// �������� ������ ���� USB
static BYTE UsbSpeed;
// ��������� � ������ ����������� � ������
static MODULE_DESCRIPTION_E154 ModuleDescription;
// ��������� ���������� ������ ��� ������
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
// ���������� ������� ������ ����������
//------------------------------------------------------------------------
__declspec(dllexport) DWORD WINAPI WE154GetDllVersion(VOID)
{
        return GetDllVersion();
}
//------------------------------------------------------------------------
// ���������� ������� ������ ����������
//------------------------------------------------------------------------
void BadExit(void);

__declspec(dllexport) DWORD WINAPI LV_OpenE154(VOID)
{
int i;

	BadExit();

	// ��������� �������� ��������� �� ���������
	pModule = static_cast<ILE154 *>(CreateLInstance("e154"));
	if(!pModule) { BadExit(); return 0; }

	// ��������� ���������� ������ E14-154 � ������ WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	// ���-������ ����������?
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) { BadExit(); return 0; }

	// ��������� ��������� ���������� ����������
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) { BadExit(); return 0; }

	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) { BadExit(); return 0; }

	// ��������, ��� ��� 'E14-154'
	if(strcmp(ModuleName, "E154")) { BadExit(); return 0; }

	// ��������� �������� �������� ������ ���� USB
	if(!pModule->GetUsbSpeed(&UsbSpeed)) { BadExit(); return 0; }

	// ������� ���������� �� ���� ������
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
	// ��������� ��������� ������
	if(pModule)
	{
		// ��������� ��������� ������
		pModule->ReleaseLInstance();

		// ������� ��������� �� ��������� ������
		pModule = NULL;
	}
}
// -----------------------------------------------------------------------------
// ---------------------------------- E154 -------------------------------------
// -----------------------------------------------------------------------------
//------------------------------------------------------------------------
// �������� ���������� ������������ ������
//------------------------------------------------------------------------
__declspec(dllexport) LPVOID WINAPI WE154CreateInstance(VOID)
{
    return CreateLInstance("E154");
}
//------------------------------------------------------------------------
//  ������������ ���������� ����������
//------------------------------------------------------------------------
__declspec(dllexport) HANDLE WINAPI WE154GetModuleHandle(LPVOID lptr)
{
    return ((ILE154*)lptr)->GetModuleHandle();
}
//------------------------------------------------------------------------
//  ������� ����������� ���� ��� ������� � USB ������
//------------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154OpenLDevice(LPVOID lptr, WORD VirtualSlot)
{
    return (BOOL)((ILE154*)lptr)->OpenLDevice(VirtualSlot);
}
//------------------------------------------------------------------------
//  ��������� ������� ����������� ����
//------------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154CloseLDevice(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->CloseLDevice();
}
// -----------------------------------------------------------------------
//  ��������� ��������� �� ����������
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ReleaseLDevice(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->ReleaseLInstance();
}
// -----------------------------------------------------------------------
//  ������ �������� ������
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154GetModuleName(LPVOID lptr, BYTE *ModuleName)
{
    return (BOOL)((ILE154*)lptr)->GetModuleName(ModuleName);
}
// -----------------------------------------------------------------------
//  ������� ������ ������ � ��������� ������� ���������� ������������ �������
// -----------------------------------------------------------------------
/*__declspec(dllexport) INT WINAPI WE154GetLastErrorString(LPVOID lptr, LPTSTR lpBuffer, DWORD nSize)
{
    return ((ILE154*)lptr)->GetLastErrorString(lpBuffer, nSize);
} */
// -----------------------------------------------------------------------
//  ���������� ������ �����
// -----------------------------------------------------------------------
/*__declspec(dllexport) BOOL WINAPI WE154GET_LBIOS_VERSION(LPVOID lptr, DWORD *LbiosVersion)
{
    return (BOOL)((ILE154*)lptr)->GET_LBIOS_VERSION(LbiosVersion);
} */
// -----------------------------------------------------------------------
// ��������� ������� ���������� ������ ���
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154GET_CUR_ADC_PARS(LPVOID lptr, ADC_PARS_E154 *wam)
{
 if(((ILE154*)lptr)->GET_ADC_PARS(wam)) {
    return TRUE;
  } else return FALSE;
}
// -----------------------------------------------------------------------
//  ���������� ��������� ���������� ������ ���
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154FILL_ADC_PARS(LPVOID lptr, ADC_PARS_E154 *wam)
{
 if(((ILE154*)lptr)->SET_ADC_PARS(wam)) {
    return TRUE;
  } else return FALSE;
}
// -----------------------------------------------------------------------
//  ������ ������ ���
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154START_ADC(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->START_ADC();
}
// -----------------------------------------------------------------------
//  ������� ���
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154STOP_ADC(LPVOID lptr)
{
    return (BOOL)((ILE154*)lptr)->STOP_ADC();
}
// -----------------------------------------------------------------------
//  ���� ����� �������� � ��� ������
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ADC_KADR(LPVOID lptr, SHORT *Data)
{
    return (BOOL)((ILE154*)lptr)->ADC_KADR(Data);
}
// -----------------------------------------------------------------------
//  ����������� ���� � ��������� ����������� ������ ��� ������
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
//  ��������� ������ ������ � ��� ������
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ReadData(LPVOID lptr, IO_REQUEST_LUSBAPI *ReadRequest)
{

    return (BOOL)((ILE154*)lptr)->ReadData(ReadRequest);
}
// -----------------------------------------------------------------------
//  ����������� ����� �� �������� ����� ���
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI LV_OUTDA_E154(double value)
{
short val;

val=value*128./5.12;
    return (BOOL)pModule->DAC_SAMPLE(&val, 0);
}
// -----------------------------------------------------------------------
//  ������� ���������� �������� ����� �������� ��������� �������
// -----------------------------------------------------------------------
static int TtlOutEnabled;
__declspec(dllexport) BOOL WINAPI WE154ENABLE_TTL_OUT(LPVOID lptr, BOOL flag)
{
    return (BOOL)((ILE154*)lptr)->ENABLE_TTL_OUT(flag ? true : false);
}
// -----------------------------------------------------------------------
//   ������� ������ ������� ����� �������� ��������� �������
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
//  ������� ������ �� �������� ����� �������� ��������� �������
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
//  ����������/���������� ������ ������ � ���� ������
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154ENABLE_FLASH_WRITE(LPVOID lptr, BOOL EnableFlashWrite)
{
    return (BOOL)((ILE154*)lptr)->ENABLE_FLASH_WRITE(EnableFlashWrite ? true : false);
}
// -----------------------------------------------------------------------
//  ������� ����� �� ����
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154D_FLASH_ARRAY(LPVOID lptr, unsigned char *ptr)
{
    return (BOOL)((ILE154*)lptr)->WRITE_FLASH_ARRAY((unsigned char *)ptr);
}
// -----------------------------------------------------------------------
//  ������ ����� � ����
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI  WEREAD_FLASH_ARRAY(LPVOID lptr, unsigned char *ptr)
{
    return (BOOL)((ILE154*)lptr)->READ_FLASH_ARRAY((unsigned char *) ptr);
}
// -----------------------------------------------------------------------
//  ������� ��������� ���������� � ������ �� ����
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154GET_MODULE_DESCR(LPVOID lptr, MODULE_DESCRIPTION_E154 *wmd)
{
 if(((ILE154*)lptr)->GET_MODULE_DESCRIPTION(wmd)) {
    return TRUE;
 } else return FALSE;
}
// -----------------------------------------------------------------------
//  ������� ��������� ���������� � ������ � ����
// -----------------------------------------------------------------------
__declspec(dllexport) BOOL WINAPI WE154SAVE_MODULE_DESCR(LPVOID lptr, MODULE_DESCRIPTION_E154 *wmd)
{
 return (BOOL)((ILE154*)lptr)->SAVE_MODULE_DESCRIPTION(wmd);
}
// -----------------------------------------------------------------------
//             ��������� ����������� ������� ����� ������
//               ��� �������� ���� usb � ����� suspend
// -----------------------------------------------------------------------
/*__declspec(dllexport) BOOL WINAPI WE154SetSuspendModeFlag(LPVOID lptr, BOOL SuspendModeFlag)
{
    return (BOOL)((ILE154*)lptr)->SetSuspendModeFlag(SuspendModeFlag ? true : false);
}
  */

//------------------------------------------------------------------------
// ���������� ������� ������ ����������
//------------------------------------------------------------------------
__declspec(dllexport) DWORD WINAPI LV_CreateChannelE154(DWORD channel, DWORD gain)
{
return (channel & 7) | ((gain & 3) << 6);
}


// -----------------------------------------------------------------------
//  ��������� ���������� ����������� ����� � ����
// -----------------------------------------------------------------------
// Nch - ����� �������� ������� (�� 1 �� 128)
// Channels - ������ � �������� �������
// Rate ������� ������ ������� ��� � ��
// AdcDelay - ����� �������������� ��� (������������ ��������), � ��
// InputMode, SynchroAdType, SynchroAdMode, SynchroAdChanne ��������� ������������� (�������� ��������)
// SynchroAdPorog ����� ������������ � �������
// SetRate ������� �������������� ������� ������ ������� ��� � ��
// SetAdcDelay ������� ������������� ������������ �������� (��)
__declspec(dllexport) BOOL WINAPI LV_SET_ADC_SYCHRO_READ_PARAMETERS(DWORD Nch, DWORD *Channels,
	float Rate, float AdcDelay, DWORD InputMode, DWORD SynchroAdType, DWORD SynchroAdMode, DWORD SynchroAdChannel,
   float SynchroAdPorog, float *SetRate, float *SetAdcDelay)
{
BOOL result;
double f;
int i;
double adc_ms, rate_ms, inter_kadr_ms;

if(!Nch) return false;

// ������� ������� ��������� ������ ���
if(!pModule->GET_ADC_PARS(&ap)) return false;

// ��������� �������� ��������� ���
ap.ClkSource = INT_ADC_CLOCK_E154;							// ���������� ������ ���
ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E154; 	// ��� ���������� �������� �������� ���
ap.InputMode = InputMode;
ap.SynchroAdType=SynchroAdType;
ap.SynchroAdMode=SynchroAdMode;
ap.SynchroAdChannel=SynchroAdChannel;
ap.SynchroAdPorog=SynchroAdPorog*2048./ADC_INPUT_RANGES_E154[(SynchroAdChannel >> 6) & 3];

ap.ChannelsQuantity = Nch; 		// ���-�� �������� ������
for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)Channels[i];

if(AdcDelay < 0.007) ap.AdcRate=120.;
  else ap.AdcRate = 1./AdcDelay;					// ������� ������ ��� � ���

// ��������� ��� � ��
if(Rate > 120000) Rate=120000;
adc_ms=1./ap.AdcRate;
rate_ms=1000./Rate;
inter_kadr_ms=rate_ms-(Nch-1)*adc_ms;
if(inter_kadr_ms < 0) inter_kadr_ms=0;
ap.InterKadrDelay = inter_kadr_ms;			// ����������� �������� � ��

// ��������� ��������� ��������� ������ ��� � ������
if(!pModule->SET_ADC_PARS(&ap)) return false;

if(!pModule->GET_ADC_PARS(&ap)) return false;

*SetAdcDelay=1./ap.AdcRate;
*SetRate=ap.KadrRate*1000.;

return true;
}

DWORD WINAPI ServiceReadThread(PVOID /*Context*/);
// ������� ������ ��������� � ��������
void ShowThreadErrorMessage(void);

// ������������� ������ ����� ������
HANDLE hReadThread;
DWORD ReadTid;


// ���-�� ���������� �������� (������� 32) ��� �. ReadData()
DWORD DataStep = 32*1024;	// ����� �������� ������ �� 64 ��

// ����� ������
SHORT *AdcBuffer, *GlobalPtr;

// ������ ���������� ������ ������ ����� ������
bool IsReadThreadComplete;
// ����� ������ ��� ���������� ����� ������
WORD ReadThreadErrorNumber;

DWORD DataReadN, DataReadAll;;

// �������� �������-���������
DWORD Counter = 0x0, OldCounter = 0xFFFFFFFF;

// NPoints - ����� ��������
// Data - ������������� ������ � ������ ���
// TimeOut	������� ��, �� ���������� �������� ������� �������� ���������� ���� ���� ������ �� ���� ������� (��� ���� ���������� ������� ������)
// result 1 ��� ����������� ������, 0 �����
__declspec(dllexport) BOOL WINAPI LV_GET_ADC_SYNCHRO_ARRAY(DWORD NPoints, SHORT *Data, DWORD TimeOut)
{
double f;
int i;
int n_wait, n;
   // ���� ������ ���� �� ����� �� ��������� ����� ��������
	WORD RequestNumber;
	// ������������� ������� �� ���� �������
	HANDLE ReadEvent[2];
	// ������ OVERLAPPED �������� �� ���� ���������
	OVERLAPPED ReadOv[2];
	IO_REQUEST_LUSBAPI IoReq[2];

if(!ap.ChannelsQuantity ) return false;
if(!NPoints) return false;

if(!TimeOut) n_wait=250;	// 5 sec
  else n_wait=TimeOut/20;

// ������� ������ ���������� ������ ����� ������
IsReadThreadComplete = false;

// ���� ������ �� �������� ��� ����� ������
AdcBuffer = NULL;

// ���� �� ������ ����� ����� ������
hReadThread = NULL;

// ������� ���� ������ ������ ����� ������
ReadThreadErrorNumber = 0x0;

DataReadN=0;
DataReadAll=NPoints*ap.ChannelsQuantity;
GlobalPtr=Data;

if(DataReadAll < 128*1024)
	{
   // ������� ����� ����� ������� 64 ������ (32 �����)
   n=DataReadAll/32;
   if(DataReadAll % 32) n++;
   n *= 32;

	AdcBuffer = new SHORT[n];
	if(!AdcBuffer) return false;

	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if(!pModule->STOP_ADC()) { free(AdcBuffer); return false; }

	// ��������� ����������� ��� ����� ������ ���������
	// ������ �������
	ReadEvent[0] = CreateEvent(NULL, FALSE , FALSE, NULL);
	// ������������� ��������� ���� OVERLAPPED
	ZeroMemory(&ReadOv[0], sizeof(OVERLAPPED)); ReadOv[0].hEvent = ReadEvent[0];
	// ��������� ��������� IoReq
	IoReq[0].Buffer = AdcBuffer;
	IoReq[0].NumberOfWordsToPass = n;
	IoReq[0].NumberOfWordsPassed = 0x0;
	IoReq[0].Overlapped = &ReadOv[0];
	IoReq[0].TimeOut = n/ap.AdcRate + 10000;

	// ������ ��������������� ������ �� ���� ������
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]);  return 0x0; }

	// �������� ���
	if(pModule->START_ADC())
	{
	// ��� ���������� �������� ����� ���������� ������ ������
	if(WaitForSingleObject(ReadEvent[0], IoReq[0].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; }
     else
       memcpy(Data, AdcBuffer, DataReadAll*2);
	}
	else { ReadThreadErrorNumber = 0x6; }

	// ��������� ������ ���
	if(!pModule->STOP_ADC()) ReadThreadErrorNumber = 0x1;
	// ������ �������� ������������� ����������� ������ �� ���� ������
	if(!CancelIo(pModule->GetModuleHandle())) { ReadThreadErrorNumber = 0x7; }
	// ��������� ��� �������������� �������
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadEvent[i]);
	// ��������� ��������
  //	Sleep(100);
	// ��������� ������ ���������� ������ ������ ����� ������
	// ������ ����� �������� �������� �� ������
   free(AdcBuffer);

   return (ReadThreadErrorNumber) ? false :  true;
   }

// ������� ������ ��� �����
AdcBuffer = new SHORT[2*DataStep];
if(!AdcBuffer) return false;

// ������ � ��������� ����� ����� ������
hReadThread = CreateThread(0, 0x2000, ServiceReadThread, 0, 0, &ReadTid);
if(!hReadThread) { free(AdcBuffer); return false; }


// ���� ������ ���������� ������ � �������� ��������� ������ ����������
for(i=0; i < n_wait && !IsReadThreadComplete; i++)
	{
		//if(OldCounter != Counter) { printf(" Counter %3u from %3u\r", Counter, NDataBlock); OldCounter = Counter; }
		//else
      Sleep(20);
	}

// ��� ��������� ������ ������ ����� ������
WaitForSingleObject(hReadThread, INFINITE);

if(ReadThreadErrorNumber) { free(AdcBuffer); return false; }

free(AdcBuffer);

return true;
}



//------------------------------------------------------------------------
// �����, � ������� �������������� ���� ������
//------------------------------------------------------------------------
DWORD WINAPI ServiceReadThread(PVOID /*Context*/)
{
	WORD RequestNumber;
	DWORD i;
	// ������������� ������� �� ���� �������
	HANDLE ReadEvent[2];
	// ������ OVERLAPPED �������� �� ���� ���������
	OVERLAPPED ReadOv[2];
	IO_REQUEST_LUSBAPI IoReq[2];

	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if(!pModule->STOP_ADC()) { ReadThreadErrorNumber = 0x1; IsReadThreadComplete = true; return 0x0; }

	// ��������� ����������� ��� ����� ������ ���������
	for(i = 0x0; i < 0x2; i++)
	{
		// ������ �������
		ReadEvent[i] = CreateEvent(NULL, FALSE , FALSE, NULL);
		// ������������� ��������� ���� OVERLAPPED
		ZeroMemory(&ReadOv[i], sizeof(OVERLAPPED)); ReadOv[i].hEvent = ReadEvent[i];
		// ��������� ��������� IoReq
		IoReq[i].Buffer = AdcBuffer + i*DataStep;
		IoReq[i].NumberOfWordsToPass = DataStep;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &ReadOv[i];
		IoReq[i].TimeOut = DataStep/ap.AdcRate + 10000;
	}

	// ������ ��������������� ������ �� ���� ������
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// �������� ���
	if(pModule->START_ADC())
	{
		// ���� ����� ������
		for(;;)
		{
			// ������� ������ �� ��������� ������ ������
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }

			// ��� ���������� �������� ����� ���������� ������ ������
			if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) { ReadThreadErrorNumber = 0x3; break; }

         if((DataReadAll-DataReadN) < DataStep)
         	{
            // ��������� ������ ������
            memcpy(&GlobalPtr[DataReadN], IoReq[RequestNumber^0x1].Buffer, 2*(DataReadAll-DataReadN));
            break;
            }
           else
           	{
            memcpy(&GlobalPtr[DataReadAll], IoReq[RequestNumber^0x1].Buffer, 2*DataStep);
            DataReadN += DataStep;
            }
			// ������� ���������� ������ ������ � ����
			if(ReadThreadErrorNumber) break;
//			else if(kbhit()) { ReadThreadErrorNumber = 0x5; break; }
			else Sleep(20);
			Counter++;
		}

	}
	else { ReadThreadErrorNumber = 0x6; }

	// ��������� ������ ���
	if(!pModule->STOP_ADC()) ReadThreadErrorNumber = 0x1;
	// ������ �������� ������������� ����������� ������ �� ���� ������
	if(!CancelIo(pModule->GetModuleHandle())) { ReadThreadErrorNumber = 0x7; }
	// ��������� ��� �������������� �������
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadEvent[i]);
	// ��������� ��������
	Sleep(100);
	// ��������� ������ ���������� ������ ������ ����� ������
	IsReadThreadComplete = true;
	// ������ ����� �������� �������� �� ������
	return 0x0;
}


int GlobalPacketSize;
SHORT *RealTimePtr;
int RealTimeReadIndex;
DWORD RealBufferSize;
static int StopEvent;
static int OverflowFlag;
static int UserIndex;

// BufferSize - ������ ������ � ���-������� ������, � ������� ����� ���������� ������ �� ������ (����� ���� ���� 1 ��, ���� �� ��� �������)
// PacketSize - ������ ������ ������� � �����, ���������� � ������� ����� ������ ����� ��������� �������� �� PacketSize ����
// ������ ������������ � FIFO �������� RealBufferSize ������� ��  GlobalPacketSize
// ������� ��������� ��������� ������ - RealTimeReadIndex
// ������� ��������� ��������� ����������  UserIndex
__declspec(dllexport) BOOL WINAPI LV_START_REAL_TIME_SAMPLING(DWORD BufferSize, DWORD PacketSize)
{
double f;
int i;
int n_wait, n;

// ���� ������ ���� �� ����� �� ��������� ����� ��������
WORD RequestNumber;

// ������������� ������� �� ���� �������
HANDLE ReadEvent[2];

// ������ OVERLAPPED �������� �� ���� ���������
OVERLAPPED ReadOv[2];
IO_REQUEST_LUSBAPI IoReq[2];

if(!ap.ChannelsQuantity ) return false;
if(BufferSize < 1024  || (BufferSize & 1)) return false;
if(PacketSize > BufferSize/2) PacketSize=BufferSize/2;
if(PacketSize < 32) PacketSize=32;
if(PacketSize % 32) { PacketSize=(PacketSize/32); PacketSize=(PacketSize+1)*32; }

// ������� ������ ���������� ������ ����� ������
IsReadThreadComplete = false;

// ���� ������ �� �������� ��� ����� ������
RealTimeReadIndex=0;
StopEvent=0;
GlobalPacketSize=PacketSize;
RealBufferSize=BufferSize;
AdcBuffer = NULL;
UserIndex=0;

// ���� �� ������ ����� ����� ������
hReadThread = NULL;

// ������� ���� ������ ������ ����� ������
ReadThreadErrorNumber = 0x0;

// ������� ������ ��� �����
AdcBuffer = new SHORT[2*PacketSize];
if(!AdcBuffer) return false;

RealTimePtr = new SHORT[RealBufferSize];
if(!RealTimePtr) { free(AdcBuffer); return false; }

// ������ � ��������� ����� ����� ������
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

// ��� ��������� ������ ������ ����� ������
WaitForSingleObject(hReadThread, INFINITE);

free(AdcBuffer); free(RealTimePtr);
AdcBuffer=NULL;
RealTimePtr=NULL;

if(ReadThreadErrorNumber) return false;

return true;
}


// index ���������� ������� ��������� ������������� ������
// overflow_flag ��������� �������� ��������, ��� ��������� ������������ ������
__declspec(dllexport) BOOL WINAPI LV_GET_REAL_TIME_INDEX(DWORD *index, DWORD *overflow_flag, DWORD *user_index)
{

*overflow_flag=OverflowFlag;
*index=RealTimeReadIndex;
*user_index=UserIndex;

if(ReadThreadErrorNumber) return false;

return true;
}

static int cnt=0;

// Data ������ � ������� ����� ���������� ������
// NPoints ������� �������� �������������
__declspec(dllexport) BOOL WINAPI LV_GET_REAL_TIME_DATA(SHORT *Data, DWORD NPoints)
{
DWORD n;
DWORD i;


// ������ ������������ � FIFO �������� RealBufferSize ������� ��  GlobalPacketSize
// ������� ��������� ��������� ������ - RealTimeReadIndex
// ������� ��������� ��������� ����������  UserIndex
if((UserIndex+NPoints) <= RealBufferSize)
         	{
            memcpy(Data, &RealTimePtr[UserIndex], 2*NPoints);
            UserIndex += NPoints;
            if(UserIndex >= RealBufferSize) UserIndex=0;
//            for(i=0, ++cnt; i < NPoints; i++) Data[i]=cnt;
            }
           else
           	{
            // 1. ������� ����� �� ����� ������
            n=RealBufferSize-UserIndex;
            memcpy(Data, &RealTimePtr[UserIndex], 2*n);

            UserIndex=0;

            // ��������� �����
            memcpy(&Data[n], &RealTimePtr[UserIndex], 2*(NPoints-n));
            UserIndex += (NPoints-n);
            }

            //cnt++;

return true;
}



//------------------------------------------------------------------------
// �����, � ������� �������������� ���� ������
//------------------------------------------------------------------------
DWORD WINAPI ServiceRealTImeReadThread(PVOID /*Context*/)
{
	WORD RequestNumber;
	DWORD i;
	// ������������� ������� �� ���� �������
	HANDLE ReadEvent[2];
	// ������ OVERLAPPED �������� �� ���� ���������
	OVERLAPPED ReadOv[2];
	IO_REQUEST_LUSBAPI IoReq[2];
   DWORD n;DWORD OldPointer;

	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if(!pModule->STOP_ADC()) { ReadThreadErrorNumber = 0x1; IsReadThreadComplete = true; return 0x0; }

	// ��������� ����������� ��� ����� ������ ���������
	for(i = 0x0; i < 0x2; i++)
	{
		// ������ �������
		ReadEvent[i] = CreateEvent(NULL, FALSE , FALSE, NULL);
		// ������������� ��������� ���� OVERLAPPED
		ZeroMemory(&ReadOv[i], sizeof(OVERLAPPED)); ReadOv[i].hEvent = ReadEvent[i];
		// ��������� ��������� IoReq
		IoReq[i].Buffer = AdcBuffer + i*GlobalPacketSize;
		IoReq[i].NumberOfWordsToPass = GlobalPacketSize;
		IoReq[i].NumberOfWordsPassed = 0x0;
		IoReq[i].Overlapped = &ReadOv[i];
		IoReq[i].TimeOut = 1000;
	}

	// ������ ��������������� ������ �� ���� ������
	RequestNumber = 0x0;
	if(!pModule->ReadData(&IoReq[RequestNumber])) { CloseHandle(ReadEvent[0]); CloseHandle(ReadEvent[1]); ReadThreadErrorNumber = 0x2; IsReadThreadComplete = true; return 0x0; }

	// �������� ���
	if(pModule->START_ADC())
	{
		// ���� ����� ������
		for(;;)
		{
			// ������� ������ �� ��������� ������ ������
			RequestNumber ^= 0x1;
			if(!pModule->ReadData(&IoReq[RequestNumber])) { ReadThreadErrorNumber = 0x2; break; }

			// ��� ���������� �������� ����� ���������� ������ ������
         for(;;)
         	{
            if(StopEvent) break;
				if(WaitForSingleObject(ReadEvent[RequestNumber^0x1], IoReq[RequestNumber^0x1].TimeOut) == WAIT_TIMEOUT) continue;
            	else break;
            }
         if(StopEvent) break;

			// ������ ������������ � FIFO �������� RealBufferSize ������� ��  GlobalPacketSize
			// ������� ��������� ��������� ������ - RealTimeReadIndex
			// ������� ��������� ��������� ����������  UserIndex
         OldPointer=RealTimeReadIndex;
         if((RealTimeReadIndex+GlobalPacketSize) <= RealBufferSize)
         	{
            memcpy(&RealTimePtr[RealTimeReadIndex], IoReq[RequestNumber^0x1].Buffer, 2*GlobalPacketSize);
            RealTimeReadIndex += GlobalPacketSize;

	         // �������� �� ������������
   	      if(UserIndex > OldPointer && UserIndex <= RealTimeReadIndex)   OverflowFlag=1;

            if(RealTimeReadIndex == RealBufferSize) RealTimeReadIndex=0;
            }
           else
           	{
            // 1. ������� ����� �� ����� ������
            n=RealBufferSize-RealTimeReadIndex;
            memcpy(&RealTimePtr[RealTimeReadIndex], IoReq[RequestNumber^0x1].Buffer, 2*n);

	         // �������� �� ������������
            RealTimeReadIndex += n;
   	      if(UserIndex > OldPointer && UserIndex <= RealTimeReadIndex)   OverflowFlag=1;

            RealTimeReadIndex=0;

            // ��������� �����
            memcpy(&RealTimePtr[RealTimeReadIndex], &IoReq[RequestNumber^0x1].Buffer[n], 2*(GlobalPacketSize-n));
            RealTimeReadIndex += (GlobalPacketSize-n);
            }


			if(ReadThreadErrorNumber) break;
				else Sleep(20);
		}

	}
	else { ReadThreadErrorNumber = 0x6; }

	// ��������� ������ ���
	if(!pModule->STOP_ADC()) ReadThreadErrorNumber = 0x1;
	// ������ �������� ������������� ����������� ������ �� ���� ������
	if(!CancelIo(pModule->GetModuleHandle())) { ReadThreadErrorNumber = 0x7; }
	// ��������� ��� �������������� �������
	for(i = 0x0; i < 0x2; i++) CloseHandle(ReadEvent[i]);
	// ��������� ��������
	Sleep(20);
	// ��������� ������ ���������� ������ ������ ����� ������
	IsReadThreadComplete = true;
	// ������ ����� �������� �������� �� ������
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


