//---------------------------------------------------------------------------
#ifndef MainFormH
#define MainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "LMDCustomBevelPanel.hpp"
#include "LMDCustomControl.hpp"
#include "LMDCustomGroupBox.hpp"
#include "LMDCustomPanel.hpp"
#include "LMDCustomParentPanel.hpp"
#include "LMDGroupBox.hpp"
#include "LMDBaseControl.hpp"
#include "LMDBaseGraphicControl.hpp"
#include "LMDBaseShape.hpp"
#include "LMDButton.hpp"
#include "LMDButtonControl.hpp"
#include "LMDCheckBox.hpp"
#include "LMDControl.hpp"
#include "LMDCustomButton.hpp"
#include "LMDCustomCheckBox.hpp"
#include "LMDCustomPanelFill.hpp"
#include "LMDShapeControl.hpp"
#include "LMDStaticText.hpp"
#include <Menus.hpp>
#include "Lusbapi.h"
#include "LMDCustomButtonGroup.hpp"
#include "LMDCustomRadioGroup.hpp"
#include "LMDRadioGroup.hpp"
#include "BMPage.h"
#include <ComCtrls.hpp>

// ������ ���������� ���������
#define	WM_START_INIT_MODULE_THREAD			(WM_USER + 100)

// ��������� ����������� ������ E-310
#define	MAX_VIRTUAL_SLOTS_QUANTITY				(0x7)				// ���-�� ������������ ����������� ������
//
enum {	ALL_MODULE_ELEMENTS_ENABLED = false, ALL_MODULE_ELEMENTS_DISABLED = true };

// ������� MCLK � ���
const double MCLK_DDS = 50000.0;
const double MAX_OUTPUT_FREQ_DDS = 0x007FFFFF * MCLK_DDS / (0x1 << 24);

// ��������� ��� ������ � ���������� ���������� DDS
struct DDS_INCREMENT_INTERVAL
{
	WORD IncrementIntervalType;
	WORD CycleIncrementIntervalMultiplier;
	WORD CycleIncrementIntervalValue;
	WORD ClockIncrementIntervalMultiplier;
	WORD ClockIncrementIntervalValue;
};

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
	__published:	// IDE-managed Components
	TLMDGroupBox *GeneratorParsLMDGroupBox;
	TLMDGroupBox *LoadingTestsLMDGroupBox;
	TLMDShapeControl *ModuleDetectionLMDShapeControl;
	TLMDStaticText *ModuleDetectionLMDStaticText;
	TLMDStaticText *VirtualSlotLabelLMDStaticText;
	TLMDStaticText *VirtualSlotLMDStaticText;
	TLMDButton *FindModuleLMDButton;
	TLMDGroupBox *SaveIniSettingLMDGroupBox;
	TLMDCheckBox *AutoSaveSettingsLMDCheckBox;
	TButton *SaveSettingsButton;
	TPopupMenu *PopupMenu;
	TMenuItem *RebootModuleMenuItem;
	TMenuItem *N1;
	TMenuItem *SaveDefaultSettings;
	TMenuItem *SaveCurrentSettings;
	TMenuItem *N3;
	TMenuItem *AboutProgram;
	TMenuItem *BreakLine;
	TMenuItem *ApplicationExit;
		void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall AutoSaveSettingsLMDCheckBoxChange(TObject *Sender);
	void __fastcall SaveSettingsButtonClick(TObject *Sender);
	void __fastcall FindModuleLMDButtonClick(TObject *Sender);
	void __fastcall ApplicationExitClick(TObject *Sender);
	void __fastcall SaveDefaultSettingsClick(TObject *Sender);
	void __fastcall SaveCurrentSettingsClick(TObject *Sender);
	void __fastcall AboutProgramClick(TObject *Sender);

	private:	// User declarations
		void __fastcall ApplicationShortCut(TWMKey &Msg, bool &Handled);
		void ControlElements(TWinControl *WinControlElement, bool EnabledFlag);

		// ������ � ������� ����������� ������ E-310
		void StartInitModuleThread(void);
		void __fastcall OnStartInitModuleThread(TMessage& Message);
		void __fastcall InitDevicesThreadDone(TObject * /*Sender*/);
		BOOL WaitFortInitDevicesThreadDone(void);

		// ������ ���������� ������ ����������
//		TE310GeneratorFrame *GeneratorFrame;
		// ������ ��� ����������� ���������� ������ ���������
		bool IsProgramInstanceAlreadyExist;
		// ������ ��� ������ � ������� ����������� ������
		bool IsInitDevicesThreadDone;

	public:		// User declarations
		__fastcall TMainForm(TComponent* Owner);

		// ��������� �� ���������
		bool CreateDefaultIniFile(AnsiString IniFileName);

		// ������������� ��������� � ������
		bool InitApplication(TIniFile *IniFile);
		bool InitLoadingParams(TIniFile *IniFile);

		// ������� ������� ����������
		bool IsProgramLaunching;
		// ������ ��� ������ � ������� ����������� ������ E-310
		bool IsInitDevicesThreadRunning;
		// ������� ���������� ����������
		bool IsAppTerminated;
		// ������ �����������������
		bool IsReenteringInProgress;
		
		// ������ �������� ������ E-310
		bool IsModuleLoaded;
		// ������� b ���������� ������
		BYTE ModuleRevision, ModuleModification;
		// �������� ������ ���� USB
		BYTE UsbSpeed;
		// ����� ������ ������ E-310
		WORD VirtualSlot;
		// ����� ����������� ���������
		WORD InitDevicesMode;

		// ��������� ���������� � ������ E-310
		MODULE_DESCRIPTION_E310 ModuleDescription;
		// ��������� ��������� ������ ������
		GENERATOR_PARS_E310 GeneratorPars;
		
		// ���� ��������
		TIniFile *IniFile;

	BEGIN_MESSAGE_MAP
		VCL_MESSAGE_HANDLER(WM_START_INIT_MODULE_THREAD, TMessage, OnStartInitModuleThread)
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
