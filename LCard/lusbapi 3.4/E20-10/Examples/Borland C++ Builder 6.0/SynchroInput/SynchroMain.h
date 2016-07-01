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
	TGroupBox *AdcStartPulseSourceRadioGroup;
	TRadioButton *InternalAdcStartPulseRadioButton;
	TRadioButton *InternalAdcStartPulseWithTranslationRadioButton;
	TRadioButton *ExternalAdcStartPulseOnRisingEdgeRadioButton;
	TRadioButton *ExternalAdcStartPulseOnFallingEdgeRadioButton;
	TGroupBox *AdcConvertPulseSourceRadioGroup;
	TRadioButton *InternalAdcConvertPulseRadioButton;
	TRadioButton *InternalAdcConvertPulseWithTranslationRadioButton;
	TRadioButton *ExternalAdcConvertPulseOnRisingEdgeRadioButton;
	TRadioButton *ExternalAdcConvertPulseOnFallingEdgeRadioButton;
	TGroupBox *OverloadGroupBox;
	TShape *Channel1PlusOverloadLed;
	TShape *Channel1MinusOverloadLed;
	TStaticText *PlusOverloadStaticText;
	TStaticText *MinusOverloadStaticText;
	TShape *Channel2PlusOverloadLed;
	TShape *Channel2MinusOverloadLed;
	TShape *Channel4PlusOverloadLed;
	TShape *Channel3PlusOverloadLed;
	TShape *Channel3MinusOverloadLed;
	TShape *Channel4MinusOverloadLed;
	TStaticText *Channel1PlusOverloadStaticText;
	TStaticText *Channel2PlusOverloadStaticText;
	TStaticText *Channel4PlusOverloadStaticText;
	TStaticText *Channel3PlusOverloadStaticText;
	TShape *UsbSpeedLed;
	TStaticText *UsbSpeedModeStaticText;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall ViewerPaintBoxMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall ViewerPaintBoxPaint(TObject *Sender);
	void __fastcall ViewerPaintBoxMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall ViewerPaintBoxDblClick(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall ActiveChannelComboBoxChange(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall AdcStartPulseRadioButtonClick(TObject *Sender);
	void __fastcall AdcConvertPulseRadioButtonClick(TObject *Sender);

private:	// User declarations
	void __fastcall ApplicationShortCut(TWMKey &Msg, bool &Handled);
	void __fastcall OnLoadUsbDevice(TMessage& Message);
	void __fastcall OnStartSynchroThread(TMessage& Message);
	void __fastcall SynchroThreadDone(TObject * /*Sender*/);
	void StartThread(void);
	void StopThread(void);

	WORD MouseDownX, MouseDownY;
	AnsiString ErrorStr;
	LAST_ERROR_INFO_LUSBAPI LatsErrorInfo;

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);

	bool IsReenteringInProgress;
	bool IsStartSynchroThreadDone;
	bool IsSynchroThreadRunning;

	// дескриптор устройства
	HANDLE ModuleHandle;
	// структура параметров работы АЦП модуля
	ADC_PARS_E2010 ap;
	// структура с полной информацией о модуле
	MODULE_DESCRIPTION_E2010 ModuleDescription;
	// скорость работы шины USB
	BYTE UsbSpeed;
	// массивы корректировочных коэффициентов
	double OffsetArray[ADC_INPUT_RANGES_QUANTITY_E2010][ADC_CHANNELS_QUANTITY_E2010];
	double ScaleArray[ADC_INPUT_RANGES_QUANTITY_E2010][ADC_CHANNELS_QUANTITY_E2010];
	// индекс входного диапазона
	WORD AdcInputRangeIndex;
	// массивы светодидов перегрузки каналов
	TShape *PlusOverloadLedsArray[ADC_CHANNELS_QUANTITY_E2010];
	TShape *MinusOverloadLedsArray[ADC_CHANNELS_QUANTITY_E2010];
	// номер активного канала
	WORD ActiveChannel;

	int* Buffer;
	DWORD ChannelsQuantity;
	DWORD ChannelPoint;
	ZoomScale ZS;
	ZoomParam ZP;
	COLORREF rgb[16];
	TZoomer *Viewer;

	WORD StartSourceIndex, SynhroSourceIndex;
	TRadioButton *AdcStartPulseSourceArrayRadioButton[INVALID_ADC_START_E2010];
	TRadioButton *AdcConvertPulseSourceArrayRadioButton[INVALID_ADC_START_E2010];

	BEGIN_MESSAGE_MAP
		VCL_MESSAGE_HANDLER(WM_LOAD_USB_DEVICE, TMessage, OnLoadUsbDevice)
		VCL_MESSAGE_HANDLER(WM_START_SYNCHRO_THREAD, TMessage, OnStartSynchroThread)
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif

