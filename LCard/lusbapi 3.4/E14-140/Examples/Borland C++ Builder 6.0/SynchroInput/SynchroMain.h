//---------------------------------------------------------------------------
#ifndef SynchroMainH
#define SynchroMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include "Zoom.h"
#include "Lusbapi.h"

#define	WM_LOAD_USB_DEVICE	         		WM_USER + 100
#define	WM_START_SYNCHRO_THREAD					WM_USER + 101

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TPaintBox *ViewerPaintBox;
	TPanel *ModulePanel;
	TGroupBox *InputModeGroupBox;
	TGroupBox *SynchroAdParsGroupBox;
	TStaticText *StaticText29;
	TComboBox *SynchroAdModeComboBox;
	TStaticText *StaticText30;
	TComboBox *SynchroAdTypeComboBox;
	TStaticText *StaticText31;
	TEdit *SynchroPorogEdit;
	TStaticText *StaticText32;
	TComboBox *SynchroAdcChannelComboBox;
	TStaticText *StaticText33;
	TComboBox *SynchroGainComboBox;
	TTrackBar *SynchroAdPorogTrackBar;
	TComboBox *InputModeComboBox;
	TStaticText *StaticText1;
	TStaticText *StaticText13;
	TShape *UsbLoadingLed;
	TGroupBox *GroupBox6;
	TStaticText *StaticText2;
	TStaticText *StaticText28;
	TStaticText *StaticText3;
	TStaticText *StaticText4;
	TStaticText *StaticText35;
	TStaticText *StaticText34;
	TStaticText *AdcRateStaticText;
	TStaticText *InterChannelDelayStaticText;
	TStaticText *ChannelRateStaticText;
	TGroupBox *GroupBox1;
	TShape *Shape1;
	TShape *Shape2;
	TShape *Shape3;
	TShape *Shape4;
	TStaticText *StaticText5;
	TStaticText *StaticText7;
	TStaticText *StaticText8;
	TStaticText *StaticText9;
	TComboBox *ActiveChannelComboBox;
	TStaticText *StaticText6;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall ViewerPaintBoxMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall ViewerPaintBoxPaint(TObject *Sender);
	void __fastcall ViewerPaintBoxMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall ViewerPaintBoxDblClick(TObject *Sender);
	void __fastcall SynchroAdPorogTrackBarChange(TObject *Sender);
	void __fastcall SynchroPorogEditKeyPress(TObject *Sender, char &Key);
	void __fastcall SynchroAdTypeComboBoxChange(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall InputModeComboBoxChange(TObject *Sender);
	void __fastcall SynchroAdcChannelComboBoxChange(TObject *Sender);
	void __fastcall SynchroGainComboBoxChange(TObject *Sender);
	void __fastcall SynchroAdModeComboBoxChange(TObject *Sender);
	void __fastcall ActiveChannelComboBoxChange(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);

private:	// User declarations
	void __fastcall ApplicationShortCut(TWMKey &Msg, bool &Handled);
	void __fastcall OnLoadUsbDevice(TMessage& Message);
	void __fastcall OnStartSynchroThread(TMessage& Message);
	void __fastcall SynchroThreadDone(TObject * /*Sender*/);
	void StartThread(void);
	void StopThread(void);
	void DisableAdPars(bool Flag);

   WORD MouseDownX, MouseDownY;

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);

	bool IsCommonGnd;
	int* Buffer;
	DWORD ChannelsQuantity;
   DWORD ChannelPoint;
   ZoomScale ZS;
   ZoomParam ZP;
	COLORREF rgb[16];
	TZoomer *Viewer;

	bool IsReenteringInProgress;

	bool IsStartSynchroThreadDone;
   bool IsSynchroThreadRunning;

	WORD ActiveChannel;
   WORD InputMode;
   WORD SynchroAdChannel, SynchroAdGain;
   int SynchroAdPorog;
   bool SynchroAdType, SynchroAdMode;
   int OldSynchroPorogTrackBar;

	// дескриптор устройства
	HANDLE ModuleHandle;
	// структура параметров работы АЦП модуля
	ADC_PARS_E140 ap;
	// структура с полной информацией о модуле
	MODULE_DESCRIPTION_E140 ModuleDescription;


   BEGIN_MESSAGE_MAP
   	VCL_MESSAGE_HANDLER(WM_LOAD_USB_DEVICE, TMessage, OnLoadUsbDevice)
   	VCL_MESSAGE_HANDLER(WM_START_SYNCHRO_THREAD, TMessage, OnStartSynchroThread)
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif

