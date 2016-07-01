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

static int Pnl;
void UpdateSynchroCtrl(void);

extern int SynchroMode;
extern int SynchroTtlMask;
extern int SynchroTtlMode;
extern int SynchroAdChannel;
extern int SynchroAdGain;
extern int SynchroAdMode;
extern int SynchroAdSubMode1;
extern int SynchroAdSubMode2;
extern double SynchroAdPorog;


void SynchroPanel(void)
{
if ((Pnl = LoadPanel (0, "test.uir", SYNCHRO_P)) < 0) return ;
//SetCtrlAttribute (WindowPanel, BMP_PNL_NUMERIC, ATTR_DIMMED, !PrintMode);

SetCtrlVal(Pnl, SYNCHRO_P_RINGSLIDE, SynchroMode);
SetCtrlVal(Pnl, SYNCHRO_P_TTL_MASK, SynchroTtlMask);
SetCtrlVal(Pnl, SYNCHRO_P_CHANNEL, SynchroTtlMode);
SetCtrlVal(Pnl, SYNCHRO_P_CHANNEL, SynchroAdChannel);
SetCtrlVal(Pnl, SYNCHRO_P_GAIN, SynchroAdGain);
SetCtrlVal(Pnl, SYNCHRO_P_ANALOG_TYPE, SynchroAdMode);
SetCtrlVal(Pnl, SYNCHRO_P_MODE1, SynchroAdSubMode1);
SetCtrlVal(Pnl, SYNCHRO_P_MODE2, SynchroAdSubMode2);
SetCtrlVal(Pnl, SYNCHRO_P_POROG, SynchroAdPorog);

UpdateSynchroCtrl();

InstallPopup (Pnl);
}



int CVICALLBACK DONE_S_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			UpdateSynchroCtrl();
			RemovePopup (0);
			break;
	}
	return 0;
}


int CVICALLBACK UPDATE_S_F (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			UpdateSynchroCtrl();
			break;
	}
	return 0;
}


void UpdateSynchroCtrl(void)
{
GetCtrlVal(Pnl, SYNCHRO_P_RINGSLIDE, &SynchroMode);
GetCtrlVal(Pnl, SYNCHRO_P_TTL_MASK, &SynchroTtlMask);
GetCtrlVal(Pnl, SYNCHRO_P_BINARYSWITCH, &SynchroTtlMode);
GetCtrlVal(Pnl, SYNCHRO_P_CHANNEL, &SynchroAdChannel);
GetCtrlVal(Pnl, SYNCHRO_P_GAIN, &SynchroAdGain);
GetCtrlVal(Pnl, SYNCHRO_P_ANALOG_TYPE, &SynchroAdMode);
GetCtrlVal(Pnl, SYNCHRO_P_MODE1, &SynchroAdSubMode1);
GetCtrlVal(Pnl, SYNCHRO_P_MODE2, &SynchroAdSubMode2);
GetCtrlVal(Pnl, SYNCHRO_P_POROG, &SynchroAdPorog);


switch(SynchroMode)
	{
	case 0:
		SetCtrlAttribute (Pnl, SYNCHRO_P_TTL_MASK, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_BINARYSWITCH, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_CHANNEL, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_GAIN, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_ANALOG_TYPE, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_MODE1, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_MODE2, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_POROG, ATTR_DIMMED, 1);
		break;
		
	case 1:
		SetCtrlAttribute (Pnl, SYNCHRO_P_TTL_MASK, ATTR_DIMMED, 0);
		SetCtrlAttribute (Pnl, SYNCHRO_P_BINARYSWITCH, ATTR_DIMMED, 0);
		SetCtrlAttribute (Pnl, SYNCHRO_P_CHANNEL, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_GAIN, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_ANALOG_TYPE, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_MODE1, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_MODE2, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_POROG, ATTR_DIMMED, 1);
		break;
		
	case 2:
		SetCtrlAttribute (Pnl, SYNCHRO_P_TTL_MASK, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_BINARYSWITCH, ATTR_DIMMED, 1);
		SetCtrlAttribute (Pnl, SYNCHRO_P_CHANNEL, ATTR_DIMMED, 0);
		SetCtrlAttribute (Pnl, SYNCHRO_P_GAIN, ATTR_DIMMED, 0);
		SetCtrlAttribute (Pnl, SYNCHRO_P_ANALOG_TYPE, ATTR_DIMMED, 0);
		SetCtrlAttribute (Pnl, SYNCHRO_P_MODE1, ATTR_DIMMED, SynchroAdMode);
		SetCtrlAttribute (Pnl, SYNCHRO_P_MODE2, ATTR_DIMMED, !SynchroAdMode);
		SetCtrlAttribute (Pnl, SYNCHRO_P_POROG, ATTR_DIMMED, 0);
		
		
		break;
		
	}

}


			
