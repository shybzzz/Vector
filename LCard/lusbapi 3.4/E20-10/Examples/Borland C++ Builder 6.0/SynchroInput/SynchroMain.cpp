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
// ��������� �� ��������� ������ E20-10
ILE2010 *pModule;

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
	ChannelPoint = 4*8192;			// ���-�� ����� �� �����

	// ������� ������ ��� Viewer
	Buffer = new int[ChannelPoint * ChannelsQuantity];
	if(!Buffer) { PostMessage(Handle, WM_LOAD_USB_DEVICE, 0x0, 0x0); return; }
	memset(Buffer, 0x0, sizeof(int)*ChannelPoint * ChannelsQuantity);

	// ������� Viewer � ��������� ��� ��������� ����������������
	Viewer = CreateOSC();

	// ��������� ��������� Viewer
//	ZS.Set(0, ChannelPoint, -8192, 8192, -8192, 8192, 6, "%-6.0f");
	ZS.Set(0.0, ChannelPoint, -8500.0, 8500.0, -8500.0, 8500.0, 6.0, "%-6.0f");
	ZP.Set(8, 8, (COLORREF)RGB(192,192, 192), (COLORREF)RGB(0, 0, 0), (COLORREF)RGB(0, 255, 0), (COLORREF)RGB(100, 100, 100));
	Viewer->SetParameters(30, &ZP);
	Viewer->MakeFont(Handle, 8);

	// ����� ��� �������� �������
	rgb[0] = RGB(255, 255, 255);	// ������ ����� - �����
	rgb[1] = RGB(0, 255, 255);		// ������ ����� - ���������
	rgb[2] = RGB(255, 255, 0);   	// ������ ����� - ������
	rgb[3] = RGB(255, 0, 0);		//	��������� ����� - �����

	Shape1->Brush->Color = (TColor)rgb[0];
	Shape2->Brush->Color = (TColor)rgb[1];
	Shape3->Brush->Color = (TColor)rgb[2];
	Shape4->Brush->Color = (TColor)rgb[3];

	ActiveChannelComboBox->ItemIndex = ActiveChannel = 0x0;
	Viewer->SetDataI(Buffer, ChannelPoint * ChannelsQuantity, ChannelsQuantity, ActiveChannel, &ZS, rgb);

	PlusOverloadLedsArray[0x0] = Channel1PlusOverloadLed;
	PlusOverloadLedsArray[0x1] = Channel2PlusOverloadLed;
	PlusOverloadLedsArray[0x2] = Channel3PlusOverloadLed;
	PlusOverloadLedsArray[0x3] = Channel4PlusOverloadLed;

	MinusOverloadLedsArray[0x0] = Channel1MinusOverloadLed;
	MinusOverloadLedsArray[0x1] = Channel2MinusOverloadLed;
	MinusOverloadLedsArray[0x2] = Channel3MinusOverloadLed;
	MinusOverloadLedsArray[0x3] = Channel4MinusOverloadLed;

	AdcStartPulseSourceArrayRadioButton[0x0] = InternalAdcStartPulseRadioButton;
	AdcStartPulseSourceArrayRadioButton[0x1] = InternalAdcStartPulseWithTranslationRadioButton;
	AdcStartPulseSourceArrayRadioButton[0x2] = ExternalAdcStartPulseOnRisingEdgeRadioButton;
	AdcStartPulseSourceArrayRadioButton[0x3] = ExternalAdcStartPulseOnFallingEdgeRadioButton;

	AdcConvertPulseSourceArrayRadioButton[0x0] = InternalAdcConvertPulseRadioButton;
	AdcConvertPulseSourceArrayRadioButton[0x1] = InternalAdcConvertPulseWithTranslationRadioButton;
	AdcConvertPulseSourceArrayRadioButton[0x2] = ExternalAdcConvertPulseOnRisingEdgeRadioButton;
	AdcConvertPulseSourceArrayRadioButton[0x3] = ExternalAdcConvertPulseOnFallingEdgeRadioButton;

	IsReenteringInProgress = true;

	// ����������������� ��������� ������������� ����� ������
	StartSourceIndex = INT_ADC_START_E2010;
	AdcStartPulseSourceArrayRadioButton[StartSourceIndex]->Checked = true;
	SynhroSourceIndex = INT_ADC_CLOCK_E2010;
	AdcConvertPulseSourceArrayRadioButton[SynhroSourceIndex]->Checked = true;

	IsReenteringInProgress = false;

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
	WORD i, j;   

	if(!Buffer) { Application->MessageBox("�� ���� �������� ������ ��� ����� ������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	Application->ProcessMessages();

	// �������� ������ ������������ DLL ����������
	if(GetDllVersion() != CURRENT_VERSION_LUSBAPI)
	{
		AnsiString ErrorMessage = "������������ ������ ���������� S211api.dll!\n";
		ErrorMessage += "�������: " + IntToStr(GetDllVersion() >> 0x10) + "." + IntToStr(GetDllVersion() & 0xFFFF) + "      ";
		ErrorMessage += "���������: " + IntToStr(CURRENT_VERSION_LUSBAPI >> 0x10) + "." + IntToStr(CURRENT_VERSION_LUSBAPI & 0xFFFF);
		Application->MessageBox(ErrorMessage.c_str(), "������!!!", MB_OK + MB_ICONINFORMATION);
		Close(); return;
	}

	// ���������� �������� ��������� �� ��������� ��� ������ E20-10
	pModule = static_cast<ILE2010 *>(CreateLInstance("e2010"));
	if(pModule == NULL) { Application->MessageBox("�� ���� �������� ��������� �� ������ E20-10!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	// ��������� ���������� ������ E20-10 � ������ MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI ����������� ������
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) { Application->MessageBox("�� ���� ���������� ������ 'E20-10' � ������ 127 ����������� ������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
//	if(!pModule->InitLDevice(0)) { Application->MessageBox("�� ���� ���������� ������ � ������� ����������� �����", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� ��������� ���������� ����������
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) { Application->MessageBox("�� ���� �������� ���������� ����������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� �������� ������ � ������� ����������� �����
	else if(!pModule->GetModuleName(ModuleName)) { Application->MessageBox("�� ���� ��������� �������� ������ � ������� ����������� �����!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������, ��� ��� 'E20-10'
	else if(StrComp(ModuleName, "E20-10")) { Application->MessageBox("� ������� ����������� ����� �� ������ 'E20-10'!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� �������� �������� ������ ���� USB
	else if(!pModule->GetUsbSpeed(&UsbSpeed)) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("�� ���� ���������� �������� ������ USB ����!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ������ ����� ���������� ��������� ������� ����� ��� ���� � ������ E20-10
	// ����� ��� ���� ������ �� ���������������� ������� DLL ����������
	else if(!pModule->LOAD_MODULE()) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("�� ���� ��������� ���� ������ E20-10!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ������ �������� �������� ������
 	else if(!pModule->TEST_MODULE()) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("������ E20-10 �� ����������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// ��������� ���� ������
	else if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { Application->MessageBox("�� ���� ��������� ���� � ������ E20-10!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// � ���� ��������� �������� ������?
	else if(strcmp(ModuleDescription.Module.DeviceName, "E20-10"))  { Application->MessageBox("� ���� �� ��������� �������� ������!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	// ����� ������� �������� ��������� ����������� ������ E20-10
	UsbLoadingLed->Brush->Color = clLime;
	UsbSpeedLed->Brush->Color = UsbSpeed ? clLime : (TColor)RGB(255, 150, 0);
	UsbSpeedModeStaticText->Caption = UsbSpeed ? "HS" : "FS";
	// �������� ���������������� ������������
	CopyMemory(OffsetArray, ModuleDescription.Adc.OffsetCalibration, sizeof(OffsetArray));
	CopyMemory(ScaleArray, ModuleDescription.Adc.ScaleCalibration, sizeof(ScaleArray));

	// ��������� ������� ��������� ������ ���
	if(!pModule->GET_ADC_PARS(&ap)) { Application->MessageBox("�� ���� ��������� ������� GET_ADC_PARS()!", "������!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	if(ModuleDescription.Module.Revision == REVISIONS_E2010[REVISION_A_E2010])
	{
		ap.IsAdcCorrectionEnabled = FALSE;		// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.A)
//		ap.OverloadMode = MARKER_OVERLOAD_E2010;		// �������� ����� ���������� ������� ������� ��� ������ �������� � ������� ��� (������ ��� Rev.A)
		ap.OverloadMode = CLIPPING_OVERLOAD_E2010; 	// ������� �������� ����� ���������� ������� ������� ���� ����������� ���� ����������� ������� ��� (������ ��� Rev.A)
	}
	else
		ap.IsAdcCorrectionEnabled = TRUE;		// �������� �������������� ������������� ������ �� ������ ������ (��� Rev.B � ����)
	// ��������� �������� ��������� ���
	ap.SynchroPars.StartSource = StartSourceIndex;					// �������� �������� ������ ���
	ap.SynchroPars.SynhroSource = SynhroSourceIndex;				// �������� ��������������� ���
	ap.SynchroPars.StartDelay = 0x0;									// �������� ������ ����� ������ � ������ �������� (��� Rev.B � ����)
	ap.SynchroPars.StopAfterNKadrs = 0x0;							// ������� ����� ������ ����� �������� ���-�� ������ �������� (��� Rev.B � ����)
	ap.SynchroPars.SynchroAdMode = NO_ANALOG_SYNCHRO_E2010;	// ��� ���������� ������������� (��� Rev.B � ����)
//	ap.SynchroPars.SynchroAdMode = ANALOG_SYNCHRO_ON_HIGH_LEVEL_E2010;
	ap.SynchroPars.SynchroAdChannel = 0x0;							// ����� ���������� ������������� (��� Rev.B � ����)
	ap.SynchroPars.SynchroAdPorog = 0;								// ����� ���������� ������������� � ����� ��� (��� Rev.B � ����)
	ap.SynchroPars.IsBlockDataMarkerEnabled = 0x0;				// ������������ ������ ����� ������ (������, ��������, ��� ���������� ������������� ����� �� ������) (��� Rev.B � ����)
	ap.ChannelsQuantity = ChannelsQuantity;			// ���-�� �������� �������
	// ��������� ����������� ������� ���������� �������
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i);
	// ������� ����� ����� ������������� � ����������� �� �������� USB
	ap.AdcRate = 2500.0; 													// ������� ������ ��� � ���
	if(UsbSpeed == USB11_LUSBAPI) ap.InterKadrDelay = 0.01;		// ����������� �������� � ��
	else ap.InterKadrDelay = 0.0;
	// ������� ���������� ������� �������� ��� ���� ������� �������
	AdcInputRangeIndex = ADC_INPUT_RANGE_3000mV_E2010;	// ������� �������� �3�
	// ������ ����������� ������� ������
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		ap.InputRange[i] = AdcInputRangeIndex;  	 		// ������� �������� �3�
		ap.InputSwitch[i] = ADC_INPUT_SIGNAL_E2010;		// �������� ����� - ������
	}
//	ap.InputSwitch[0x3] = ADC_INPUT_ZERO_E2010;			// �������� ����� - �����
	// ������� � ��������� ���������� ������ ��� ���������������� ������������ ���
	for(i = 0x0; i < ADC_INPUT_RANGES_QUANTITY_E2010; i++)
		for(j = 0x0; j  < ADC_CHANNELS_QUANTITY_E2010; j++)
		{
			// ������������� ��������
			ap.AdcOffsetCoefs[i][j] = ModuleDescription.Adc.OffsetCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
			// ������������� ��������
			ap.AdcScaleCoefs[i][j] = ModuleDescription.Adc.ScaleCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
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
void __fastcall TMainForm::ViewerPaintBoxMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
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

	// ����� �����������������...
	IsReenteringInProgress = false;
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
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::AdcStartPulseRadioButtonClick(TObject *Sender)
{
	WORD i;

	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	for(i = 0x0; i < INVALID_ADC_START_E2010; i++) if(AdcStartPulseSourceArrayRadioButton[i] == Sender) break;
	if(i == INVALID_ADC_START_E2010) Application->MessageBox("�� ���� ������� ����� �������� �������� ������ ���!","������ AdcStartPulseRadioButtonClick()!", MB_OK);
	else
	{
		// �������� �������� ������ ���
		ap.SynchroPars.StartSource = StartSourceIndex = i;
		// �������� � ������ ���������� ������ ���
		if(!pModule->SET_ADC_PARS(&ap))
		{
			pModule->GetLastErrorInfo(&LatsErrorInfo);
			ErrorStr = "�� ���� ��������� ������� SET_ADC_PARS().\n";
			ErrorStr += (AnsiString)(char *)LatsErrorInfo.ErrorString;
			Application->MessageBox(ErrorStr.c_str(), "������ AdcStartPulseRadioButtonClick()!", MB_OK + MB_ICONINFORMATION);
//			Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ AdcStartPulseRadioButtonClick()!", MB_OK + MB_ICONINFORMATION);
		}
		// �������� ����� ����� ������
		else StartThread();
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::AdcConvertPulseRadioButtonClick(TObject *Sender)
{
	WORD i;

	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������� ������� ����� �����
	StopThread();

	for(i = 0x0; i < INVALID_ADC_START_E2010; i++) if(AdcConvertPulseSourceArrayRadioButton[i] == Sender) break;
	if(i == INVALID_ADC_START_E2010) Application->MessageBox("�� ���� ������� ����� �������� ��������������� ���!","������ AdcConvertPulseRadioButtonClick()!", MB_OK);
	else
	{
		// �������� ���������������� ���
		ap.SynchroPars.SynhroSource = SynhroSourceIndex = i;
		// �������� � ������ ���������� ������ ���
		if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("�� ���� ��������� ������� SET_ADC_PARS()!", "������ AdcConvertPulseRadioButtonClick()!", MB_OK + MB_ICONINFORMATION);
		// �������� ����� ����� ������
		else StartThread();
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}
//---------------------------------------------------------------------------

