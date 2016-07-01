//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SynchroMain.h"
#include "SynchroThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

// �������� ����� ���������
TMainForm *MainForm;
// ����� ����� � ����������� ������
TSynchroThread *SynchroThread;
// ��������� �� ��������� ������ E14-440
ILE440 *pModule;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// ������� Create
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
	DecimalSeparator = '.';

	// ������� �� ������� ESCAPE
	Application->OnShortCut = &ApplicationShortCut;

	// ������� �������������� ������ �����
	Width = GetSystemMetrics(SM_CXFULLSCREEN) - GetSystemMetrics(SM_CXFULLSCREEN)/20;

	ChannelsQuantity = 0x4;			// ����� ���������� ������ �������� ������
	ChannelPoint = 8192;				// ���-�� ����� �� �����

	// ������� ������ ��� Viewer
	Buffer = new int[ChannelPoint * ChannelsQuantity];
	if(!Buffer) { PostMessage(Handle, WM_LOAD_USB_DEVICE, 0x0, 0x0); return; }
	memset(Buffer, 0x0, sizeof(int)*ChannelPoint * ChannelsQuantity);

	// ������� Viewer � ��������� ��� ��������� ����������������
	Viewer = CreateOSC();
                                                                                                       // RGB(192,192, 192)
	// ��������� ��������� Viewer
	ZS.Set(0, ChannelPoint, -8192, 8192, -8192, 8192, 6, "%-6.0f");
	ZP.Set(8, 8, (COLORREF)RGB(192,192, 192), (COLORREF)RGB(0, 0, 0), (COLORREF)RGB(0, 255, 0), (COLORREF)RGB(100, 100, 100));
	Viewer->SetParameters(30, &ZP);
	Viewer->MakeFont(Handle, 8);

	// ����� ��� �������� �������
	rgb[0] = RGB(255, 255, 255);	// ������ ����� - �����
	rgb[1] = RGB(0, 255, 255);		// ������ ����� - ���������
	rgb[2] = RGB(255, 255, 0);   	// ������ ����� - ������
	rgb[3] = RGB(255, 0, 0);		//	��������� ����� - �����
	rgb[4] = RGB(0, 255, 0);		//	��������� ����� - �����

	Shape1->Brush->Color = (TColor)rgb[0];
	Shape2->Brush->Color = (TColor)rgb[1];
	Shape3->Brush->Color = (TColor)rgb[2];
	Shape4->Brush->Color = (TColor)rgb[3];

	ActiveChannelComboBox->ItemIndex = ActiveChannel = 0x0;
	Viewer->SetDataI(Buffer, ChannelPoint * ChannelsQuantity, ChannelsQuantity, ActiveChannel, &ZS, rgb);

	IsReenteringInProgress = false;

	// ����������������� ��������� ���������� �������������
	InputModeComboBox->ItemIndex = InputMode;
	if(InputMode == ANALOG_SYNC_E440) DisableAdPars(false);
	else DisableAdPars(true);
	SynchroAdTypeComboBox->ItemIndex = (SynchroAdType = true);
	SynchroAdModeComboBox->ItemIndex = SynchroAdMode;
	SynchroAdcChannelComboBox->ItemIndex = SynchroAdChannel;
	SynchroGainComboBox->ItemIndex = SynchroAdGain;
	SynchroPorogEdit->Text = (SynchroAdPorog = 0x0);

	// ��������� ��� ���������� ���
	ViewerPaintBox->ControlStyle = ViewerPaintBox->ControlStyle << csOpaque;
}

//---------------------------------------------------------------------------
// ��������� ������� ������� ESCAPE �� ����������
//---------------------------------------------------------------------------
void __fastcall TMainForm::ApplicationShortCut(TWMKey &Msg, bool &Handled)
{
	if(Msg.CharCode == VK_ESCAPE)
   	if(Application->MessageBox("������������� ������ ��������� ������?", "����������� ����������!", MB_YESNO + MB_ICONQUESTION) == IDYES)
      	Application->Terminate();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
	// ��������� �������� ������
	PostMessage(Handle, WM_LOAD_USB_DEVICE, 0x0, 0x0);
}

//---------------------------------------------------------------------------
// ��������� �������� ������
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnLoadUsbDevice(TMessage& Message)
{
	char ModuleName[7];
	WORD i;   

	if(!Buffer) { Application->MessageBox("�� ���� �������� ������ ��� ����� ������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	Application->ProcessMessages();

	// �������� ������ ������������ DLL ����������
	if(GetDllVersion() != CURRENT_VERSION_LUSBAPI)
	{
		AnsiString ErrorMessage = "������������ ������ ���������� Lusbapi.dll!\n";
		ErrorMessage += "�������: " + IntToStr(GetDllVersion() >> 0x10) + "." + IntToStr(GetDllVersion() & 0xFFFF) + "      ";
		ErrorMessage += "���������: " + IntToStr(CURRENT_VERSION_LUSBAPI >> 0x10) + "." + IntToStr(CURRENT_VERSION_LUSBAPI & 0xFFFF);
		Application->MessageBox(ErrorMessage.c_str(), "������!!!", MB_OK + MB_ICONINFORMATION);
		Close(); return;
	}

	// ���������� �������� ��������� �� ��������� ��� ������ E14-440
	pModule = static_cast<ILE440 *>(CreateLInstance("e440"));
	if(pModule == NULL) { Application->MessageBox("�� ���� �������� ��������� �� ������ E14-440!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	// ��������� ���������� ������ E14-440 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) { Application->MessageBox("�� ���� ���������� ������ 'E14-440' � ������ 127 ����������� ������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
//	if(!pModule->InitLDevice(0)) { Application->MessageBox("�� ���� ���������� ������ � ������� ����������� �����", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� ��������� ���������� ����������
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) { Application->MessageBox("�� ���� �������� ���������� ����������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� �������� ������ � ������� ����������� �����
	else if(!pModule->GetModuleName(ModuleName)) { Application->MessageBox("�� ���� ��������� �������� ������ � ������� ����������� �����!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������, ��� ��� 'E14-440'
	else if(StrComp(ModuleName, "E440")) { Application->MessageBox("� ������� ����������� ����� �� ������ 'E14-440'!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ������ ����� ���������� ��������� ������� LBIOS � ������ E14-440
	// ��� LBIOS'� ������ �� ���������������� ������� DLL ����������
	else if(!pModule->LOAD_MODULE()) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("�� ���� ��������� LBIOS � ������ E14-440!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ������ �������� �������� ������
 	else if(!pModule->TEST_MODULE()) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("������ E14-440 �� ����������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� ���� ������
	else if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { Application->MessageBox("�� ���� ��������� ���� � ������ E14-440!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// � ���� ��������� �������� ������?
	else if(strcmp(ModuleDescription.Module.DeviceName, "E14-440"))  { Application->MessageBox("� ���� �� ��������� �������� ������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	UsbLoadingLed->Brush->Color = clLime;

	// ��������� ������� ��������� ������ ���
	if(!pModule->GET_ADC_PARS(&ap)) { Application->MessageBox("�� ���� ��������� ������� GET_ADC_PARS()!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� �������� ��������� ���
	ap.IsCorrectionEnabled = true;						// �������� ������������� ������ �� ������ �������� DSP
	ap.ChannelsQuantity = ChannelsQuantity;			// ������ �������� ������
	// ����� ���������� ����������� ������� �������: ���. ��� � ����� ������?
	IsCommonGnd = false;
	// ��������� ����������� ������� ���������� �������
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i |
   																				(IsCommonGnd << 0x5)/*����� �����*/ |
                                                               (0x1 << 0x6)/*�������� 4*/);
	ap.AdcRate = 250.0;										// ������� ������ ��� � ���
	ap.InterKadrDelay = 0.0;								// ����������� �������� � ���
	// ����� ������������ ��������� ������������� ������������, ������� �������� � ���� ������
	for(i = 0x0; i < ADC_CALIBR_COEFS_QUANTITY_E440; i++)
	{
		ap.AdcOffsetCoefs[i] =  ModuleDescription.Adc.OffsetCalibration[i];
		ap.AdcScaleCoefs[i] =  ModuleDescription.Adc.ScaleCalibration[i];
	}
	// �������� � ������ ���������� ������ ���
	if(!pModule->SET_ADC_PARS(&ap)) { Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	AdcRateStaticText->Caption = FormatFloat("0.0", ap.AdcRate);;
	InterChannelDelayStaticText->Caption = FormatFloat("0.0000", ap.InterKadrDelay);
	ChannelRateStaticText->Caption = FormatFloat("0.00", ap.KadrRate);

	// ������ ����� ��������� ����� ����� � ����������� ������
	IsStartSynchroThreadDone = true;
	StartThread();
}

//---------------------------------------------------------------------------
// c������ MouseMove
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewerPaintBoxMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	UINT modKeys = 0x0;
	if(Shift.Contains(ssShift)) modKeys |= MK_SHIFT;
//	if(Shift.Contains(ssAlt)) modKeys |= ;
	if(Shift.Contains(ssCtrl)) modKeys |= MK_CONTROL;
	if(Shift.Contains(ssLeft)) modKeys |= MK_LBUTTON;
	if(Shift.Contains(ssRight)) modKeys |= MK_RBUTTON;
	if(Shift.Contains(ssMiddle)) modKeys |= MK_MBUTTON;
//	if(Shift.Contains(ssDouble)) modKeys |= ;

	if(Viewer) Viewer->OnMouseMove(ViewerPaintBox->ClientRect, modKeys, MAKELONG(X, Y));
	ViewerPaintBox->Invalidate();
}

//---------------------------------------------------------------------------
// c������ Paint
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewerPaintBoxPaint(TObject *Sender)
{
	if(Viewer) Viewer->OnPaint(ViewerPaintBox->Canvas->Handle, ViewerPaintBox->ClientRect, 1);
}

//---------------------------------------------------------------------------
//	c������ MouseDown
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewerPaintBoxMouseDown(TObject *Sender,
							      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if(Viewer)
	{
		MouseDownX = (WORD)X;
		MouseDownY = (WORD)Y;
		if(Shift.Contains(ssLeft)) Viewer->OnButtonDown(ViewerPaintBox->ClientRect, MK_LBUTTON, MAKELONG(X,Y), 0);
		else if(Shift.Contains(ssRight)) Viewer->OnButtonDown(ViewerPaintBox->ClientRect, MK_RBUTTON, MAKELONG(X,Y), 0);

		ViewerPaintBox->Invalidate();
	}
}

//---------------------------------------------------------------------------
// c������ DblClick
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewerPaintBoxDblClick(TObject *Sender)
{
	Viewer->OnButtonDown(ViewerPaintBox->ClientRect, MK_LBUTTON, MAKELONG(MouseDownX,MouseDownY), -1);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TMainForm::DisableAdPars(bool Flag)
{
	SynchroAdParsGroupBox->Enabled = Flag ? false : true;
	SynchroAdParsGroupBox->Font->Color = Flag ? clGray : clBlack;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::InputModeComboBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	if(InputModeComboBox->ItemIndex == ANALOG_SYNC_E440) DisableAdPars(false);
	else DisableAdPars(true);

	// �������� ���������� ������������� ������ ���
	InputMode = (WORD)( InputModeComboBox->ItemIndex);
	ap.InputMode = InputMode;
	ap.SynchroAdType = SynchroAdType;
	ap.SynchroAdMode = SynchroAdMode;
	ap.SynchroAdChannel = (WORD)(SynchroAdChannel | (SynchroAdGain << 6));
	ap.SynchroAdPorog = (SHORT)SynchroAdPorog;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ InputModeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// �������� ����� ����� ������
	else StartThread();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroAdcChannelComboBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	// �������� ���������� ������������� ������ ���
   SynchroAdChannel = (WORD)(SynchroAdcChannelComboBox->ItemIndex);
   ap.SynchroAdChannel = (WORD)(SynchroAdChannel | (SynchroAdGain << 6));
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// �������� ����� ����� ������
	else StartThread();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroGainComboBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	// �������� ���������� ������������� ������ ���
   SynchroAdGain = (WORD)(SynchroGainComboBox->ItemIndex);
   ap.SynchroAdChannel = (WORD)(SynchroAdChannel | (SynchroAdGain << 6));
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// �������� ����� ����� ������
	else StartThread();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroAdPorogTrackBarChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	SynchroAdPorog = SynchroAdPorogTrackBar->Position;
   if(SynchroAdPorog > 8191) SynchroAdPorog = 8191;
   else if(SynchroAdPorog < -8191) SynchroAdPorog = -8192;
	SynchroPorogEdit->Text = " " + FormatFloat("0.", SynchroAdPorogTrackBar->Position);

	// �������� ���������� ������������� ������ ���
   ap.SynchroAdPorog = (SHORT)SynchroAdPorog;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// �������� ����� ����� ������
	else StartThread();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroPorogEditKeyPress(TObject *Sender, char &Key)
{
	if(!isdigit(Key))
		if(Key != VK_BACK	)
			if(Key != '-')
				if(Key != '.')
            {
				   SynchroAdPorog = (int)(SynchroPorogEdit->Text.ToIntDef(0));
				   if(SynchroAdPorog > 8191) SynchroAdPorog = 8191;
				   else if(SynchroAdPorog < -8191) SynchroAdPorog = -8192;
				   SynchroPorogEdit->Text = SynchroAdPorog;
					SynchroAdPorogTrackBar->Position = SynchroAdPorog;

			   	Key = NULL;
            }
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroAdTypeComboBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

   if(SynchroAdTypeComboBox->ItemIndex) { SynchroAdModeComboBox->Items->Strings[0] = "  �����-�����"; SynchroAdModeComboBox->Items->Strings[1] = "  ������-����"; }
   else { SynchroAdModeComboBox->Items->Strings[0] = "  ����"; SynchroAdModeComboBox->Items->Strings[1] = "  ����"; }
	SynchroAdType = SynchroAdTypeComboBox->ItemIndex;
   SynchroAdModeComboBox->ItemIndex = SynchroAdMode;

	// �������� ���������� ������������� ������ ���
   ap.SynchroAdType = SynchroAdType;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// �������� ����� ����� ������
	else StartThread();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroAdModeComboBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	// �������� ���������� ������������� ������ ���
   SynchroAdMode = SynchroAdModeComboBox->ItemIndex;
   ap.SynchroAdMode = SynchroAdMode;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// �������� ����� ����� ������
	else StartThread();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	// ��������� ��������� ������
	if(pModule) { pModule->ReleaseLInstance(); pModule = NULL; }
	// ��������� ������� Viewer
	if(Viewer) { Viewer->Release(); Viewer = NULL; }
	// ��������� ������
	if(Buffer) { delete[] Buffer; Buffer = NULL; }
}

//---------------------------------------------------------------------------
// ������ ������ ����� ������
//---------------------------------------------------------------------------
void TMainForm::StartThread(void)
{
	if(!IsSynchroThreadRunning && IsStartSynchroThreadDone)
   {
      IsStartSynchroThreadDone = false;
      PostMessage(Handle, WM_START_SYNCHRO_THREAD, 0x0, 0x0);
	}
}

//---------------------------------------------------------------------------
// ���������� ��� ������ ������
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnStartSynchroThread(TMessage& Message)
{
	IsSynchroThreadRunning = false;
	SynchroThread = new TSynchroThread(false);
	if(!SynchroThread) { Application->MessageBox("�� ���� ������� ����� 'SynchroThread'!","������ OnStartSynchroThread()!", MB_OK); Close(); return; }
	SynchroThread->OnTerminate = SynchroThreadDone;
   IsStartSynchroThreadDone = true;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroThreadDone(TObject * /*Sender*/)
{
	IsSynchroThreadRunning = false;
}

//---------------------------------------------------------------------------
// ������� ������ ����� ������
//---------------------------------------------------------------------------
void TMainForm::StopThread(void)
{
	if(SynchroThread && IsStartSynchroThreadDone)
   {
   	while(!IsSynchroThreadRunning){ Application->ProcessMessages(); Sleep(50); };
		delete SynchroThread;
		while(IsSynchroThreadRunning) { Application->ProcessMessages(); Sleep(50); };
		SynchroThread = NULL;
   }
}

//---------------------------------------------------------------------------
// ��������� ��������� ������
//---------------------------------------------------------------------------
void __fastcall TMainForm::ActiveChannelComboBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	ActiveChannel = (WORD)ActiveChannelComboBox->ItemIndex;
   Viewer->SetDataI(Buffer, ChannelPoint * ChannelsQuantity, ChannelsQuantity, ActiveChannel, &ZS, rgb);

	// �������� ����� ����� ������
	StartThread();

	// ����� �����������������...
	IsReenteringInProgress = false;
}
//---------------------------------------------------------------------------

