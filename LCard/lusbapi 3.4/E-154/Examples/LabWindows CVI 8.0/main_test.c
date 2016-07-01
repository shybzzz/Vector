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
int InitE154(void);
int CloseE154(void);
int TestLinkE154(void);
void AddToLog(char *msg);
int BurnFlash(void);
int ShowSerialNumber(void);
int LoadManualPanel(void);


int CurrentTestIndex;
int TestMode;	// 0 manual, 1 auto
int SpeedTestFlag;	// 0 full, 1 speed
int LoopMode;
int StopOnError;
int RunOn;
int Cycles;
int Errors;
int RunBurnOn;
int RunBurnMode;

extern signed short *Data;
extern double *DataDouble;
extern double *Conv_Spectrum;
extern double *Auto_Spectrum, *ArrayXRe, *ArrayXIm, *Window;
double *Conv_Spectrum_Array[8];


static int panelHandle;
extern unsigned char cstr[1024], Rx[10240], Tx[1024];
extern unsigned char drive_name[20], dir_name[1024], file_name[256];
extern unsigned char cfg_file[1024];
extern LPVOID pModule;
extern int LtrInitOk;
extern int LtrError;
extern int E154InitOk;	
void InitTests(int who);
int GlobalTestPanelHandle;
int StopEvent;
int ScreenHeight, ScreenWidth;

int main (int argc, char *argv[])
{

 int i, j, i1, len, Handle, ErrorCode, n=10, n1;
 int pnl, cnt;
 
 char str[180], *str1, str2[100];
 
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


 Data=malloc(2*1000000);
 DataDouble=malloc(8*200000);
 Conv_Spectrum=malloc(8*100000);
 Auto_Spectrum=malloc(8*100000);
 Auto_Spectrum=malloc(8*100000);
 ArrayXRe=malloc(8*100000);
 ArrayXIm=malloc(8*100000);
 Window=malloc(8*100000);
 
 for(i=0; i < 8; i++) 
 	{ 
	Conv_Spectrum_Array[i]=malloc(8*100000); if(Conv_Spectrum_Array[i] == NULL) { MessagePopup("ERROR", "No memory"); return 0; };
	}
 
 if(Data == NULL || DataDouble == NULL || Conv_Spectrum  == NULL || Window==NULL ||
	 Auto_Spectrum == NULL || ArrayXIm == NULL || ArrayXRe == NULL) {MessagePopup("ERROR", "No memory"); return 0; }
 GlobalTestPanelHandle=panelHandle;
 
 InitE154();
 if(E154InitOk != TRUE)
	{
	MessagePopup("ERROR", "E-154 not found");
	}
LoadManualPanel(); 

 return 0;
}



int CVICALLBACK MANUAL_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SetCtrlAttribute (panelHandle, NALADKA_P_TIMER, ATTR_ENABLED, 0);
			LoadManualPanel();
			break;
	}
	return 0;
}

