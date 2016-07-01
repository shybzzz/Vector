//---------------------------------------------------------------------------
#ifndef AboutH
#define AboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <Graphics.hpp>
#include "LMDCustomBevelPanel.hpp"
#include "LMDCustomControl.hpp"
#include "LMDCustomGroupBox.hpp"
#include "LMDCustomPanel.hpp"
#include "LMDCustomPanelFill.hpp"
#include "LMDCustomParentPanel.hpp"
#include "LMDGroupBox.hpp"
#include "LMDStaticText.hpp"
#include "LMDButton.hpp"
#include "LMDCustomButton.hpp"
#include "PJVersionInfo.hpp"
#include "ESColorMemo.hpp"
#include "URLLink.hpp"

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class TAboutProgramPanel : public TForm
{
__published:	// IDE-managed Components
	TLMDGroupBox *VersionsLMDGroupBox;
	TLMDStaticText *E310DemoVersionLabelLMDStaticText;
	TLMDStaticText *LusbapiVersionLabelLMDStaticText;
	TLMDStaticText *E310DemoVersionLMDStaticText;
	TLMDStaticText *LusbapiVersionLMDStaticText;
	TLMDButton *OkLMDButton;
	TPJVersionInfo *PJVersionInfo;
	TESColorMemo *AboutESColorMemo;
	TStaticText *StaticText1;
	TStaticText *StaticText2;
	TURLLink *LcardUrlLink;
	TURLLink *URLLink;
	TURLLink *URLLink1;
	TESColorMemo *ESColorMemo;
	TLMDStaticText *LdevusbuVersionLabelLMDStaticText;
	TLMDStaticText *LdevusbuVersionLMDStaticTextLMDStaticText;
	void __fastcall OkLMDButtonClick(TObject *Sender);
	void __fastcall FormHide(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);

private:	// User declarations
	void __fastcall ApplicationShortCut(TWMKey &Msg, bool &Handled);
	AnsiString GetWinDir(void);

	TShortCutEvent OnShortCutAddress;

public:		// User declarations
	__fastcall TAboutProgramPanel(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutProgramPanel *AboutProgramPanel;
//---------------------------------------------------------------------------
#endif
