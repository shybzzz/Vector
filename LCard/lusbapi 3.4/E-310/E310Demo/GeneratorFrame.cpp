//---------------------------------------------------------------------------
#include <vcl.h>
#include <math.h>
#pragma hdrstop

#include "GeneratorFrame.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "LMDBaseEdit"
#pragma link "LMDCustomBevelPanel"
#pragma link "LMDCustomButtonGroup"
#pragma link "LMDCustomControl"
#pragma link "LMDCustomEdit"
#pragma link "LMDCustomGroupBox"
#pragma link "LMDCustomPanel"
#pragma link "LMDCustomPanelFill"
#pragma link "LMDCustomParentPanel"
#pragma link "LMDCustomRadioGroup"
#pragma link "LMDEdit"
#pragma link "LMDGroupBox"
#pragma link "LMDRadioGroup"
#pragma link "LMDStaticText"
#pragma link "LMDButton"
#pragma link "LMDComboBox"
#pragma link "LMDCustomButton"
#pragma link "LMDCustomComboBox"
#pragma link "LMDBaseControl"
#pragma link "LMDBaseGraphicControl"
#pragma link "LMDBaseLabel"
#pragma link "LMDControl"
#pragma link "LMDCustomLabel"
#pragma link "LMDLabel"
#pragma link "LMDButtonControl"
#pragma link "LMDCheckBox"
#pragma link "LMDCustomCheckBox"
#pragma resource "*.dfm"

// ������ ���������� ����������
TE310GeneratorFrame *E310GeneratorFrame;


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TE310GeneratorFrame::TE310GeneratorFrame(TComponent* Owner) : TFrame(Owner)
{
	WORD i;
	double Ampl = sqrt(2.0) * 0.77459666924148;		// ��������� ������ 0 �� �� �������� 600 ��

	// �����������
	DecimalSeparator = '.';

	// ������� ������ ��������������� ��������
	ControlElementsAddr = NULL;
	ControlElementsState = NULL;
	// ������� ����� ���-�� ����������� ���������
	ControlElementsQuantity = 0x0;

	// ������������ ��� ������� DDS
	MAX_FREQ_DDS_CODE = 0x1 << 24;
	// ����������� �������� ������� ������������ ������� ���������� � ���
	MIN_MASTER_CLOCK = 1.0;
	MAX_MASTER_CLOCK = 50000.0;
	// ������� ����������� ������������ ������� ���������� � ���
	INTERNAL_MASTER_CLOCK = 50000.0;
	// ������������� ���c��� ����������� ���������� ���������� ��� ���������� DDS
	IncrementIntervalMultiplierArray[0x0] = 1.0;		IncrementIntervalMultiplierArray[0x1] = 5.0;
	IncrementIntervalMultiplierArray[0x2] = 100.0;	IncrementIntervalMultiplierArray[0x3] = 500.0;

	// ������������� ���c��� �������� ��������� ������ ���������� � ��
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310]		= 10.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_PLUS_06_DB_E310]		= 6.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_PLUS_04_DB_E310]		= 4.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_PLUS_03_DB_E310]		= 3.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_00_DB_E310]			= 0.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_02_DB_E310]	= -2.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_03_DB_E310]	= -3.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_06_DB_E310]	= -6.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_09_DB_E310]	= -9.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_12_DB_E310]	= -12.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_14_DB_E310]	= -14.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_18_DB_E310]	= -18.;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_21_DB_E310]	= -21.0;
	GeneratorGaindBArray[ANALOG_OUTPUT_GAIN_MINUS_24_DB_E310]	= -24.0;
	// ���������� ������� ��������� �������� �� ������ 10 � 50 �� � ��
	for(i = 0x0; i < ANALOG_OUTPUT_GAINS_QUANTITY_E310; i++)
	{
		Output10OhmdBArray[i] = GeneratorGaindBArray[i];
		Output50OhmdBArray[i] = GeneratorGaindBArray[i] - 6.0;
	}

	// ���������� ������� ��������� �������� �� ������ 10 � 50 �� � �
	for(i = 0x0; i < ANALOG_OUTPUT_GAINS_QUANTITY_E310; i++)
	{
		Output10OhmVArray[i] = Ampl * pow(10.0, Output10OhmdBArray[i]/20.0);
		Output50OhmVArray[i] = Ampl * pow(10.0, Output50OhmdBArray[i]/20.0);
	}

	// ������������� ComboBox � ��������� �������� ��������� ������ ����������
	InitAnalogOutputsGaindBComboBox();

	// ������� ��������� ����������
	GeneratorState = GENERATOR_IS_OFF;

	// ��������� ������ �����������������
	IsReenteringInProgress = true;

	// -=== ����� ���� �����������
	MasterClockSourceLMDRadioGroup->ItemIndex = -1;
	ExternalMasterClockValueLMDEdit->Text = "";
	InternalMasterClockValueLMDStaticText->Caption = "";

	CyclicAutoScanLMDRadioGroup->ItemIndex = -1;

	DesiredStartFrequencyValueLMDEdit->Text = "";
	ActualStartFrequencyValueLMDStaticText->Caption = "";
	DesiredFrequencyIncrementValueLMDEdit->Text = "";
	ActualFrequencyIncrementLMDStaticText->Caption = "";
	NumberOfIncrementsValueLMDEdit->Text = "";
	FinalFrequencyValueLMDStaticText->Caption = "";
	BaseIntervalTypeLMDRadioGroup->ItemIndex = -1;
	BaseIntervalMultiplierLMDRadioGroup->ItemIndex = -1;
	BaseIntervalsNumberLMDEdit->Text = "";
	DurationLMDStaticText->Caption = "";

	AnalogOutputsTypeLMDRadioGroup->ItemIndex = -1;
	AnalogOutputsGaindBLMDComboBox->ItemIndex = -1;
	ApmlitudeOutput50OhmInVLMDStaticText->Caption = "";
	ApmlitudeOutput50OhmIndBLMDStaticText->Caption = "";
	ApmlitudeOutput10OhmInVLMDStaticText->Caption = "";
	ApmlitudeOutput10OhmIndBLMDStaticText->Caption = "";
	OffsetTypeOutput10OhmLMDRadioGroup->ItemIndex = 1;
	DesiredOffsetValueOutput10OhmLMDEdit->Text = "";
	ActualOffsetValueOutput10OhmLMDStaticText->Caption = "";

	IncrementTypeLMDRadioGroup->ItemIndex = -1;
	CtrlLineTypeLMDRadioGroup->ItemIndex = -1;
	InterrupLineTypeLMDRadioGroup->ItemIndex = -1;
	EnaSyncoutLMDCheckBox->Enabled = false;
	SyncoutTypeLMDRadioGroup->ItemIndex = -1;
	EnaSquareWaveOutputLMDCheckBox->Enabled = false;
	// -==========================

	// ������� ������ �����������������
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
// ������������� ComboBox � ��������� �������� ��������� ������ ����������
//---------------------------------------------------------------------------
void TE310GeneratorFrame::InitAnalogOutputsGaindBComboBox(void)
{
	WORD i;

	AnalogOutputsGaindBLMDComboBox->Items->Clear();
	for(i = 0x0; i < ANALOG_OUTPUT_GAINS_QUANTITY_E310; i++)
		AnalogOutputsGaindBLMDComboBox->Items->Add(GeneratorGaindBArray[i]);
	AnalogOutputsGaindBLMDComboBox->DropDownCount = ANALOG_OUTPUT_GAINS_QUANTITY_E310;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::GetControlElementState(void)
{
	WORD i;
	WORD count;

	// ������ ����� ���-�� ����������� ���������
	for(i = count = 0x0; i < this->ComponentCount; i++)
	{
		TControl *Control;
		Control = dynamic_cast<TControl *>(this->Components[i]);
		if(Control) count++;
	}

	// ���� ����� - ������� ��� ��� ������
	if(!ControlElementsAddr || !ControlElementsState || (ControlElementsQuantity != count))
	{
		if(ControlElementsAddr)		{ delete[] ControlElementsAddr; ControlElementsAddr = NULL; }
		if(ControlElementsState)	{ delete[] ControlElementsState; ControlElementsState = NULL; }

		ControlElementsQuantity = count;

		ControlElementsAddr = new TControl* [ControlElementsQuantity];
		ControlElementsState = new bool[ControlElementsQuantity];
	}

	// �������� ������ � ��������� ���� ����������� ���������
	for(i = count = 0x0; i < this->ComponentCount; i++)
	{
		ControlElementsAddr[count] = NULL;
		ControlElementsState[count] = false;

		TControl *Control;
		Control = dynamic_cast<TControl *>(this->Components[i]);
		if(Control)
		{
			ControlElementsAddr[count]		= Control;
			ControlElementsState[count]	= Control->Enabled;
			count++;
			if(ControlElementsQuantity == count) break;
		}
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::ControlElements(bool EnabledFlag)
{
	WORD i;

	for(i = 0x0; i < ControlElementsQuantity; i++)
	{
		if(ControlElementsAddr[i] == ControlGeneratorLMDButton) continue;
		if(EnabledFlag)
		{
			if(ControlElementsState[i])
				ControlElementsAddr[i]->Enabled = EnabledFlag;
		}
		else ControlElementsAddr[i]->Enabled = EnabledFlag;
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::WinControlElements(TWinControl *WinControlElement, bool EnabledFlag)
{
	WORD i;

	for(i = 0x0; i < WinControlElement->ControlCount; i++)
	{
		if(WinControlElement->Controls[i] == ControlGeneratorLMDButton) continue;

		TWinControl *WinControl;
		WinControl = dynamic_cast<TWinControl *>(WinControlElement->Controls[i]);
		if(WinControl) WinControlElements(dynamic_cast<TWinControl *>(WinControlElement->Controls[i]), EnabledFlag);

		TControl *Control;
		Control = dynamic_cast<TControl *>(WinControlElement->Controls[i]);
		if(Control) { Control->Enabled = EnabledFlag;  }
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TE310GeneratorFrame::~TE310GeneratorFrame()
{
	//
	if(pModule)
		if(!pModule->STOP_GENERATOR()) { Application->MessageBox("�� ���� ��������� ������� STOP_GENERATOR()","������ ~TE310GeneratorFrame()!!!", MB_OK + MB_ICONINFORMATION); return; }

	if(ControlElementsAddr)		{ delete[] ControlElementsAddr; ControlElementsAddr = NULL; }
	if(ControlElementsState)	{ delete[] ControlElementsState; ControlElementsState = NULL; }
}

//---------------------------------------------------------------------------
// ��������� ���������� ��������� ������ ���������� �� ������
//---------------------------------------------------------------------------
void TE310GeneratorFrame::InitGeneratorFramePars(ILE310 * const pModule, GENERATOR_PARS_E310 * const GeneratorPars)
{
	// �������� ��������� �� ��������� ������
	this->pModule = pModule;
	// �������� ��������� ������ ����������
	this->GeneratorPars = *GeneratorPars;

	// ���� ����� ��������� ���������
	if(GeneratorState == GENERATOR_IS_ON)
	{
		// �������� ��������� ����������
		GeneratorState ^= 0x1;

		ControlElements(true);
		ControlGeneratorLMDButton->Font->Color = clGreen;
		ControlGeneratorLMDButton->Caption = "������ ����������";
	}

	// �������� �������� ������� ����������
	MasterClockSourceLMDRadioGroup->ItemIndex = this->GeneratorPars.MasterClockSource;
	// �������� �������� ������� ����������
	InternalMasterClockValueLMDStaticText->Caption = FormatFloat("0.00", INTERNAL_MASTER_CLOCK);
	// ���� ����� - ���������� ������� �������� �������
	if(this->GeneratorPars.MasterClock < MIN_MASTER_CLOCK) this->GeneratorPars.MasterClock = MIN_MASTER_CLOCK;
	else if(this->GeneratorPars.MasterClock > MAX_MASTER_CLOCK) this->GeneratorPars.MasterClock = MAX_MASTER_CLOCK;
	ExternalMasterClockValueLMDEdit->Text = FormatFloat("0.00", this->GeneratorPars.MasterClock);
	if(this->GeneratorPars.MasterClockSource == INTERNAL_MASTER_CLOCK_E310)
	{
		InternalMasterClockValueLMDStaticText->Visible = true;
		ExternalMasterClockValueLMDEdit->Visible = false;
	}
	else if(this->GeneratorPars.MasterClockSource == EXTERNAL_MASTER_CLOCK_E310)
	{
		InternalMasterClockValueLMDStaticText->Visible = false;
		ExternalMasterClockValueLMDEdit->Visible = true;
	}
//	else return false;
	// ��� ������������ ���������������� ��������� �������
	CyclicAutoScanLMDRadioGroup->ItemIndex = this->GeneratorPars.CyclicAutoScanType;
//	CyclicAutoScanLMDRadioGroupChange(CyclicAutoScanLMDRadioGroup, NULL);
//	IncrementTypeLMDRadioGroupChange(IncrementTypeLMDRadioGroup, NULL);

	// ��������� ������� ���������� � ���
	DesiredStartFrequencyValueLMDEdit->Text = FormatFloat("0.0000", this->GeneratorPars.StartFrequency);
	// ������� ���������� � ���
	DesiredFrequencyIncrementValueLMDEdit->Text = FormatFloat("0.0000", this->GeneratorPars.FrequencyIncrements);
	// ���-�� ���������� ������� ������������
	NumberOfIncrementsValueLMDEdit->Text = FormatFloat("0", this->GeneratorPars.NumberOfIncrements);

	// ��� ����� "INTERRUPT" ��� �������� ������ ����������
	InterrupLineTypeLMDRadioGroup->ItemIndex = this->GeneratorPars.InterrupLineType;
	// ���������� ������� ���������� �� �������� ������ "������"
	EnaSquareWaveOutputLMDCheckBox->Checked = this->GeneratorPars.SquareWaveOutputEna ? true : false;

	// ��� ��������� ����������
	BaseIntervalTypeLMDRadioGroup->ItemIndex = this->GeneratorPars.IncrementIntervalPars.BaseIntervalType;
	// ������ ���������� ��������� ����������
	BaseIntervalMultiplierLMDRadioGroup->ItemIndex = this->GeneratorPars.IncrementIntervalPars.MultiplierIndex;
	// ���-�� ������ (���� MCLK, ���� �������� �������) � ��������� ����������
	BaseIntervalsNumberLMDEdit->Text = FormatFloat("0", this->GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber);

	CyclicAutoScanLMDRadioGroupChange(CyclicAutoScanLMDRadioGroup, NULL);
	IncrementTypeLMDRadioGroupChange(IncrementTypeLMDRadioGroup, NULL);

	// ��� ����������� ������� �� ������� 10 � 50 ��
	AnalogOutputsTypeLMDRadioGroup->ItemIndex = this->GeneratorPars.AnalogOutputsPars.SignalType;
	// ������ �������� ��������� ������ ����������
	AnalogOutputsGaindBLMDComboBox->ItemIndex = this->GeneratorPars.AnalogOutputsPars.GainIndex;
	// ��������� �������� �� ���������� ������� 10 � 50 ��
	CalculateAnalogOutputsGains();
	// ��� �������� �� ������ 10 ��
	OffsetTypeOutput10OhmLMDRadioGroup->ItemIndex = this->GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource;
	OffsetTypeOutput10OhmLMDRadioGroupChange(OffsetTypeOutput10OhmLMDRadioGroup, NULL);
	// �������� ����������� �������� �� ������ 10 �� � �
	DesiredOffsetValueOutput10OhmLMDEdit->Text = this->GeneratorPars.AnalogOutputsPars.Output10OhmOffset;
}

//---------------------------------------------------------------------------
// ��������� ������� ��� ��������� ������ ���������� �� ������ ������
//---------------------------------------------------------------------------
void TE310GeneratorFrame::GetGeneratorFramePars(GENERATOR_PARS_E310 * const GeneratorPars)
{
	// ������� ���������
	GetGeneratorPars();
	// ��������� ��
	*GeneratorPars = this->GeneratorPars;
}


//---------------------------------------------------------------------------
// ��������� ������� ��� ��������� ������ ����������
//---------------------------------------------------------------------------
void TE310GeneratorFrame::GetGeneratorPars(void)
{
	try
	{
		// �������� �������� ������� ����������
		GeneratorPars.MasterClockSource = MasterClockSourceLMDRadioGroup->ItemIndex;
		// �������� �������� ������� ����������
		if(GeneratorPars.MasterClockSource == INTERNAL_MASTER_CLOCK_E310)
			GeneratorPars.MasterClock = INTERNAL_MASTER_CLOCK;
		else
			GeneratorPars.MasterClock = ExternalMasterClockValueLMDEdit->Text.ToDouble();
		// ��� ������������ ���������������� ��������� �������
		GeneratorPars.CyclicAutoScanType = CyclicAutoScanLMDRadioGroup->ItemIndex;

		// ��������� ������� ���������� � ���
		GeneratorPars.StartFrequency = DesiredStartFrequencyValueLMDEdit->Text.ToDouble();
		// ������� ���������� � ���
		GeneratorPars.FrequencyIncrements = DesiredFrequencyIncrementValueLMDEdit->Text.ToDouble();
		// ���-�� ���������� ������� ������������
		GeneratorPars.NumberOfIncrements = NumberOfIncrementsValueLMDEdit->Text.ToDouble();

		// ��� ��������� ����������
		GeneratorPars.IncrementIntervalPars.BaseIntervalType = BaseIntervalTypeLMDRadioGroup->ItemIndex;
		// ������ ���������� ��������� ����������
		GeneratorPars.IncrementIntervalPars.MultiplierIndex = BaseIntervalMultiplierLMDRadioGroup->ItemIndex;
		// ���-�� ������ (���� MCLK, ���� �������� �������) � ��������� ����������
		GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber = BaseIntervalsNumberLMDEdit->Text.ToDouble();

		// ��� ����������� ������� �� ������� 10 � 50 ��
		GeneratorPars.AnalogOutputsPars.SignalType = AnalogOutputsTypeLMDRadioGroup->ItemIndex;
		// ������ �������� ��������� ������ ����������
		GeneratorPars.AnalogOutputsPars.GainIndex = AnalogOutputsGaindBLMDComboBox->ItemIndex;
		// ��� �������� �� ������ 10 ��
		GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource = OffsetTypeOutput10OhmLMDRadioGroup->ItemIndex;
		// �������� ����������� �������� �� ������ 10 �� � �
		GeneratorPars.AnalogOutputsPars.Output10OhmOffset = DesiredOffsetValueOutput10OhmLMDEdit->Text.ToDouble();


		// ��� ������������� ������� ����������
		GeneratorPars.IncrementType = IncrementTypeLMDRadioGroup->ItemIndex;
		// ��� ����� "CTRL" ��� (���������� �������������� �������)/������ ����������: ���������� (�� MCU)
		GeneratorPars.CtrlLineType = CtrlLineTypeLMDRadioGroup->ItemIndex;
		// ��� ����� "INTERRUPT" ��� �������� ������ ����������
		GeneratorPars.InterrupLineType = InterrupLineTypeLMDRadioGroup->ItemIndex;
		// ������������ ��������� ������������� �� ����� "SYNCOUT"
		GeneratorPars.SynchroOutEna = EnaSyncoutLMDCheckBox->Checked ? 0x1 : 0x0;
		GeneratorPars.SynchroOutType = SyncoutTypeLMDRadioGroup->ItemIndex;
		// ���������� ������� ���������� �� �������� ������ "������"
		GeneratorPars.SquareWaveOutputEna = EnaSquareWaveOutputLMDCheckBox->Checked ? 0x1 : 0x0;
	}
	catch(...)
	{
		Application->MessageBox("���������� ����������.", "������ GetGeneratorPars()!!!", MB_OK + MB_ICONINFORMATION);
	}

}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::MasterClockSourceLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	try
	{
		//
		GeneratorPars.MasterClockSource = MasterClockSourceLMDRadioGroup->ItemIndex;
		//
		if(GeneratorPars.MasterClockSource == INTERNAL_MASTER_CLOCK_E310)
		{
//			InternalMasterClockValueLMDStaticText->Caption = FormatFloat("0.00", MAX_MASTER_CLOCK);
			GeneratorPars.MasterClock = InternalMasterClockValueLMDStaticText->Caption.ToDouble();
			InternalMasterClockValueLMDStaticText->Visible = true;
			ExternalMasterClockValueLMDEdit->Visible = false;
		}
		else if(GeneratorPars.MasterClockSource == EXTERNAL_MASTER_CLOCK_E310)
		{
//			ExternalMasterClockValueLMDEdit->Text = FormatFloat("0.00", GeneratorPars.MasterClock);
			GeneratorPars.MasterClock = ExternalMasterClockValueLMDEdit->Text.ToDouble();
			InternalMasterClockValueLMDStaticText->Visible = false;
			ExternalMasterClockValueLMDEdit->Visible = true;
		}

		// ����� �����������������...
		IsReenteringInProgress = false;

		// ���������� �������
		DesiredStartFrequencyValueLMDEditChange(DesiredStartFrequencyValueLMDEdit);
		DesiredFrequencyIncrementValueLMDEditChange(DesiredFrequencyIncrementValueLMDEdit);
		if(GeneratorPars.IncrementType == AUTO_INCREMENT_E310)
		{
			WinControlElements(IncrementIntervalParsLMDGroupBox, true);
			if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
			{
				IncrementIntervalDiplay(true);
				// ��������� ��������� �������� ���������� � ��
				CalculateIncrementInterval();
			}
			else
				IncrementIntervalDiplay(false);
		}
		else
			WinControlElements(IncrementIntervalParsLMDGroupBox, false);
	}
	catch(EConvertError&)
	{
			Application->MessageBox("���������� ������ �������������� ������ � �����.", "������ ExternalMasterClockValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::ExternalMasterClockValueLMDEditKeyPress(TObject *Sender, char &Key)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	if(!isdigit(Key))
	{
		if(Key != VK_BACK)
		{
			if(Key != '.')
			{
				Key = NULL;
			}
		}
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::ExternalMasterClockValueLMDEditChange(TObject *Sender)
{
	double DesiredExternalMasterClockFrequency;

	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// �������� ������ � �������� �������� ������������ �������
	if(!ExternalMasterClockValueLMDEdit->Text.IsEmpty())
	{
		try
		{
			// ��������� ������ �������� ���������� ������� �������� ������������ �������
			DesiredExternalMasterClockFrequency = ExternalMasterClockValueLMDEdit->Text.ToDouble();
			// ���� ����� - ����������
			if(DesiredExternalMasterClockFrequency < MIN_MASTER_CLOCK) DesiredExternalMasterClockFrequency = MIN_MASTER_CLOCK;
			else if(DesiredExternalMasterClockFrequency > MAX_MASTER_CLOCK) DesiredExternalMasterClockFrequency = MAX_MASTER_CLOCK;
		}
		catch(EConvertError&)
		{
			if(!ExternalMasterClockValueLMDEdit->Text.IsEmpty())
				Application->MessageBox("���������� ������ �������������� ������ � �����.", "������ ExternalMasterClockValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// �������� ������� �������� ������������ �������
		GeneratorPars.MasterClock = DesiredExternalMasterClockFrequency;
		ExternalMasterClockValueLMDEdit->Text = FormatFloat("0.0", DesiredExternalMasterClockFrequency);

		// ����� �����������������...
		IsReenteringInProgress = false;

		// ���������� �������
		DesiredStartFrequencyValueLMDEditChange(DesiredStartFrequencyValueLMDEdit);
		DesiredFrequencyIncrementValueLMDEditChange(DesiredFrequencyIncrementValueLMDEdit);
		if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
		{
			IncrementIntervalDiplay(true);
			// ��������� ��������� �������� ���������� � ��
			CalculateIncrementInterval();
		}
		else
			IncrementIntervalDiplay(false);
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::CyclicAutoScanLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ��� ������������ ���������������� ��������� �������
	GeneratorPars.CyclicAutoScanType = CyclicAutoScanLMDRadioGroup->ItemIndex;
	// ������ ������� ��� CyclicAutoScanType ������ NO_CYCLIC_AUTOSCAN_E310
	if(GeneratorPars.CyclicAutoScanType != NO_CYCLIC_AUTOSCAN_E310)
	{
		// ��� ������������� ������� ����������: ���������� (��������������)
		IncrementTypeLMDRadioGroup->Enabled = false;
		IncrementTypeLMDRadioGroup->ItemIndex = AUTO_INCREMENT_E310;
		// ��� ����� "CTRL" ��� (���������� �������������� �������)/������ ����������: ���������� (�� MCU)
		CtrlLineTypeLMDRadioGroup->Enabled = false;
		CtrlLineTypeLMDRadioGroup->ItemIndex =INTERNAL_CTRL_LINE_E310;
		// ������������ ��������� ������������� �� ����� "SYNCOUT"
		SyncoutLineParsLMDGroupBox->Enabled = false;
		WinControlElements(SyncoutLineParsLMDGroupBox, false);
		EnaSyncoutLMDCheckBox->Checked = true;
		SyncoutTypeLMDRadioGroup->ItemIndex = SYNCOUT_AT_END_OF_SCAN_E310;
	}
	else
	{
		// ��� ������������� ������� ����������: ���������� (��������������)
		IncrementTypeLMDRadioGroup->Enabled = true;
		IncrementTypeLMDRadioGroup->ItemIndex = GeneratorPars.IncrementType;
		// ��� ����� "CTRL" ��� (���������� �������������� �������)/������ ����������: ���������� (�� MCU)
		CtrlLineTypeLMDRadioGroup->Enabled = true;
		CtrlLineTypeLMDRadioGroup->ItemIndex = GeneratorPars.CtrlLineType;
		// ������������ ��������� ������������� �� ����� "SYNCOUT"
		SyncoutLineParsLMDGroupBox->Enabled = true;
		WinControlElements(SyncoutLineParsLMDGroupBox, true);
		EnaSyncoutLMDCheckBox->Checked = GeneratorPars.SynchroOutEna ? true : false;
		SyncoutTypeLMDRadioGroup->ItemIndex = GeneratorPars.SynchroOutType;
	}

	// ����� �����������������...
	IsReenteringInProgress = false;

	// ���� ����� - ���������� �������� ����������
	IncrementTypeLMDRadioGroupChange(IncrementIntervalParsLMDGroupBox, NULL);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::IncrementTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������ ������� ��� CyclicAutoScanType ������ NO_CYCLIC_AUTOSCAN_E310
	if(GeneratorPars.CyclicAutoScanType == NO_CYCLIC_AUTOSCAN_E310)
	{
		// ��� ������������� ������� ����������
		GeneratorPars.IncrementType = IncrementTypeLMDRadioGroup->ItemIndex;
		//
		if(GeneratorPars.IncrementType == AUTO_INCREMENT_E310)
		{
			IncrementIntervalParsLMDGroupBox->Enabled = true;
			WinControlElements(IncrementIntervalParsLMDGroupBox, true);

			// ����� �����������������...
			IsReenteringInProgress = false;

			BaseIntervalTypeLMDRadioGroupChange(BaseIntervalTypeLMDRadioGroup, NULL);
		}
		else
		{
			IncrementIntervalParsLMDGroupBox->Enabled = false;
			WinControlElements(IncrementIntervalParsLMDGroupBox, false);
		}
	}
	else
	{
		if(IncrementTypeLMDRadioGroup->ItemIndex == AUTO_INCREMENT_E310)
		{
			IncrementIntervalParsLMDGroupBox->Enabled = true;
			WinControlElements(IncrementIntervalParsLMDGroupBox, true);

			// ����� �����������������...
			IsReenteringInProgress = false;

			BaseIntervalTypeLMDRadioGroupChange(BaseIntervalTypeLMDRadioGroup, NULL);
		}
		else
		{
			IncrementIntervalParsLMDGroupBox->Enabled = false;
			WinControlElements(IncrementIntervalParsLMDGroupBox, false);
		}
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::CtrlLineTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ��� ����� "CTRL" ��� (���������� �������������� �������)/������ ����������
	GeneratorPars.CtrlLineType = CtrlLineTypeLMDRadioGroup->ItemIndex;

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::InterrupLineTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ��� ����� "INTERRUPT" ��� �������� ������ ����������
	GeneratorPars.InterrupLineType = InterrupLineTypeLMDRadioGroup->ItemIndex;

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::EnaSyncoutLMDCheckBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ���������� ������������� ���������� �� �������� ����� "SYNCOUT"
	GeneratorPars.SynchroOutEna = EnaSyncoutLMDCheckBox->Checked ? TRUE : FALSE;

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::SyncoutTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ��� ������������ ������������� ����������
	GeneratorPars.SynchroOutType = SyncoutTypeLMDRadioGroup->ItemIndex;

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::EnaSquareWaveOutputLMDCheckBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ���������� ������� ���������� �� �������� ������ "������"
	GeneratorPars.SquareWaveOutputEna = EnaSquareWaveOutputLMDCheckBox->Checked ? TRUE : FALSE;

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredStartFrequencyValueLMDEditKeyPress(TObject *Sender, char &Key)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	if(!isdigit(Key))
	{
		if(Key != VK_BACK)
		{
			if(Key != '.')
			{
				Key = NULL;
			}
		}
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredStartFrequencyValueLMDEditChange(TObject *Sender)
{
	DWORD StartFreqCode;
	double DesiredStartFrequency;

	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// �������� ������ � ���������� ��������� ��������
	if(!DesiredStartFrequencyValueLMDEdit->Text.IsEmpty())
	{
		DesiredStartFrequencyDiplay(true);
		try
		{
			// ��������� ������ �������� ���������� ��������� �������
			DesiredStartFrequency = DesiredStartFrequencyValueLMDEdit->Text.ToDouble();
			// ������ ��� ���������� ��������� �������
			StartFreqCode = (DesiredStartFrequency / GeneratorPars.MasterClock) * MAX_FREQ_DDS_CODE + 0.5;
			// ���� ����� - ����������
			if(StartFreqCode > MAX_FREQ_DDS_CODE) StartFreqCode = MAX_FREQ_DDS_CODE;
		}
		catch(EConvertError&)
		{
			if(!DesiredStartFrequencyValueLMDEdit->Text.IsEmpty())
				Application->MessageBox("���������� ������ �������������� ������ � �����.", "������ DesiredStartFrequencyValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// �������� ���������� ��������� ������� ����������
		ActualStartFrequency = GeneratorPars.MasterClock * StartFreqCode / MAX_FREQ_DDS_CODE;
		ActualStartFrequencyValueLMDStaticText->Caption = FormatFloat("0.0000", ActualStartFrequency);
		// ��������� ���������� �������� ������� ����������
		CalculateStopFrequency();
	}
	else
	{
		ActualStartFrequency = -10.0;
		DesiredStartFrequencyDiplay(false);
		CalculateStopFrequency();
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::DesiredStartFrequencyDiplay(bool Flag)
{
	ActualStartFrequencyLMDStaticText->Enabled = Flag;
	ActualStartFrequencyValueLMDStaticText->Enabled = Flag;
	ActualStartFrequencyValueLMDStaticText->Caption = Flag ? "" : "-----";
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredFrequencyIncrementValueLMDEditKeyPress(TObject *Sender, char &Key)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	if(!isdigit(Key))
	{
		if(Key != VK_BACK)
		{
			if(Key != '.')
			{
				if(Key != '-')
				{
					Key = NULL;
				}
			}
		}
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredFrequencyIncrementValueLMDEditChange(TObject *Sender)
{
	DWORD DeltaFreqCode;
	double DesiredDeltaFrequency;
	double DeltaFreqSign;

	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// �������� ������ � ���������� �������� ����������
	if(!DesiredFrequencyIncrementValueLMDEdit->Text.IsEmpty() && (DesiredFrequencyIncrementValueLMDEdit->Text != AnsiString("-")))
	{
		DesiredDeltaFrequencyDiplay(true);
		try
		{
			// ��������� ������ �������� ���������� ������� ����������
			DesiredDeltaFrequency = DesiredFrequencyIncrementValueLMDEdit->Text.ToDouble();
			DeltaFreqSign = (DesiredDeltaFrequency >= 0.0) ? 1.0 : (-1.0);
			// ������ ��� ���������� ������� ����������
			DeltaFreqCode = (fabs(DesiredDeltaFrequency) / GeneratorPars.MasterClock) * MAX_FREQ_DDS_CODE + 0.5;
			// ���� ����� - ����������
			if(DeltaFreqCode > MAX_FREQ_DDS_CODE) DeltaFreqCode = MAX_FREQ_DDS_CODE;
		}
		catch(EConvertError&)
		{
			if(!DesiredFrequencyIncrementValueLMDEdit->Text.IsEmpty())
				Application->MessageBox("���������� ������ �������������� ������ � �����.", "������ DesiredStartFrequencyValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// �������� ���������� ������� ���������� ����������
		ActualDeltaFrequency = GeneratorPars.MasterClock * DeltaFreqSign * DeltaFreqCode / MAX_FREQ_DDS_CODE;
		ActualFrequencyIncrementLMDStaticText->Caption = FormatFloat("0.0000", ActualDeltaFrequency);
		// ��������� ���������� �������� ������� ����������
		CalculateStopFrequency();
	}
	else
	{
		ActualDeltaFrequency = 0.0;
		DesiredDeltaFrequencyDiplay(false);
		CalculateStopFrequency();
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::DesiredDeltaFrequencyDiplay(bool Flag)
{
	ActualFrequencyIncrementLabeLMDStaticText->Enabled = Flag;
	ActualFrequencyIncrementLMDStaticText->Enabled = Flag;
	ActualFrequencyIncrementLMDStaticText->Caption = Flag ? "" : "-----";
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::NumberOfIncrementsValueLMDEditKeyPress(TObject *Sender, char &Key)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	if(!isdigit(Key))
	{
		if(Key != VK_BACK)
		{
			if(Key != '.')
			{
				Key = NULL;
			}
		}
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::NumberOfIncrementsValueLMDEditChange(TObject *Sender)
{
	int NumberOfIncrementsValue;

	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	NumberOfIncrementsValue = abs(NumberOfIncrementsValueLMDEdit->Text.ToIntDef(0x1));
	if(NumberOfIncrementsValue > 4095) NumberOfIncrementsValue = 4095;
	NumberOfIncrementsValueLMDEdit->Text = FormatFloat("0", GeneratorPars.NumberOfIncrements = NumberOfIncrementsValue);
	// ��������� ���������� �������� ������� ����������
	CalculateStopFrequency();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
// ������� ���������� ���������� �������� ������� ����������
//---------------------------------------------------------------------------
bool TE310GeneratorFrame::CalculateStopFrequency(void)
{
	// �������� �� ��������������� - ���� �� ������ ����!
	if((ActualStopFrequency < 0.0) || (ActualStartFrequency < 0.0))
	{
		FinalFrequencyLabelLMDStaticText->Enabled = false;
		FinalFrequencyValueLMDStaticText->Enabled = false;
		FinalFrequencyValueLMDStaticText->Caption = "-----";
		FinalFrequencyUnitLMDStaticText->Enabled = false;
		return false;
	}
	else
	{
		DWORD StopFreqCode;
		double DesiredStopFrequency;

		// �������� �������� �������
		DesiredStopFrequency = ActualStartFrequency + ActualDeltaFrequency * GeneratorPars.NumberOfIncrements;
		if((0.0 < DesiredStopFrequency) && (DesiredStopFrequency < (GeneratorPars.MasterClock / 2.0)))
		{
			// ������ ��� ���������� ������� ����������
			StopFreqCode = (DesiredStopFrequency / GeneratorPars.MasterClock) * MAX_FREQ_DDS_CODE + 0.5;
			// ���� ����� - ����������
			if(StopFreqCode > MAX_FREQ_DDS_CODE) StopFreqCode = MAX_FREQ_DDS_CODE;
			// �������� ���������� ������� ���������� ����������
			ActualStopFrequency = GeneratorPars.MasterClock * StopFreqCode / MAX_FREQ_DDS_CODE;

			FinalFrequencyLabelLMDStaticText->Enabled = true;
			FinalFrequencyLabelLMDStaticText->Enabled = true;
			FinalFrequencyValueLMDStaticText->Enabled = true;
			FinalFrequencyValueLMDStaticText->Caption = FormatFloat("0.0000", ActualStopFrequency);
			FinalFrequencyUnitLMDStaticText->Enabled = true;
		}
		else
		{
			if(DesiredStopFrequency < 0.0) Application->MessageBox("�������� ������� ��������� ������ 0.0!!!","������ CalculateStopFrequency()!!!", MB_OK + MB_ICONINFORMATION);
			else Application->MessageBox("�������� ������� ��������� ������ MasterClock/2.0!!!","������ CalculateStopFrequency()!!!", MB_OK + MB_ICONINFORMATION);
			FinalFrequencyLabelLMDStaticText->Enabled = false;
			FinalFrequencyValueLMDStaticText->Enabled = false;
			FinalFrequencyValueLMDStaticText->Caption = "-----";
			FinalFrequencyUnitLMDStaticText->Enabled = false;
			return false;
		}
		return true;
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::BaseIntervalTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ��� ��������� ����������
	GeneratorPars.IncrementIntervalPars.BaseIntervalType = BaseIntervalTypeLMDRadioGroup->ItemIndex;
	if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
	{
		IncrementIntervalDiplay(true);
		// ��������� ��������� �������� ���������� � ��
		CalculateIncrementInterval();
	}
	else
		IncrementIntervalDiplay(false);

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::BaseIntervalMultiplierLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������ ���������� ��������� ����������
	GeneratorPars.IncrementIntervalPars.MultiplierIndex = BaseIntervalMultiplierLMDRadioGroup->ItemIndex;

	if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
	{
		IncrementIntervalDiplay(true);
		// ��������� ��������� �������� ���������� � ��
		CalculateIncrementInterval();
	}
	else
		IncrementIntervalDiplay(false);

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::BaseIntervalsNumberLMDEditKeyPress(TObject *Sender, char &Key)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	if(!isdigit(Key))
	{
		if(Key != VK_BACK)
		{
			if(Key != '.')
			{
				Key = NULL;
			}
		}
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::BaseIntervalsNumberLMDEditChange(TObject *Sender)
{
	DWORD IncrementIntervalNumber;

	if(GeneratorPars.IncrementType == AUTO_INCREMENT_E310)
	{
		// �����������������...
		if(IsReenteringInProgress) return;
		else IsReenteringInProgress = true;

		// �������� ������ � ���-��� ������ (���� MCLK, ���� �������� �������) � ��������� ����������
		if(BaseIntervalsNumberLMDEdit->Text.IsEmpty()) BaseIntervalsNumberLMDEdit->Text = 2;
		//
		try
		{
			// ��������� ������ ���������� ���-�� ������ (���� MCLK, ���� �������� �������) � ��������� ����������
			IncrementIntervalNumber = BaseIntervalsNumberLMDEdit->Text.ToIntDef(2);
			// ���� ����� - ����������
			if(IncrementIntervalNumber >= 2047) IncrementIntervalNumber = 2047;
			else if(IncrementIntervalNumber < 2) IncrementIntervalNumber = 2;
		}
		catch(EConvertError&)
		{
			Application->MessageBox("���������� ������ �������������� ������ � �����.", "������ IncrementIntervalNumberLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// �������� ���-�� ������ (���� MCLK, ���� �������� �������) � ��������� ����������
		GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber = IncrementIntervalNumber;
		BaseIntervalsNumberLMDEdit->Text = FormatFloat("0", IncrementIntervalNumber);

		if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
		{
			IncrementIntervalDiplay(true);
			// ��������� ��������� �������� ���������� � ��
			CalculateIncrementInterval();
		}
		else IncrementIntervalDiplay(false);

		// ����� �����������������...
		IsReenteringInProgress = false;
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::IncrementIntervalDiplay(bool Flag)
{
	DurationLabelLMDLabel->Enabled = Flag;
	DurationLMDStaticText->Enabled = Flag;
	DurationLMDStaticText->Caption = Flag ? "" : "-----";
	DurationUnitLMDStaticText->Enabled = Flag;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::CalculateIncrementInterval(void)
{
	DurationLMDStaticText->Caption = FormatFloat("0.00000", GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber * (IncrementIntervalMultiplierArray[GeneratorPars.IncrementIntervalPars.MultiplierIndex] / GeneratorPars.MasterClock));
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::AnalogOutputsTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ��� ����������� ������� �� ������� 10 � 50 ��
	GeneratorPars.AnalogOutputsPars.SignalType = AnalogOutputsTypeLMDRadioGroup->ItemIndex;

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::AnalogOutputsGaindBLMDComboBoxChange(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// ������ �������� ��������� ������ ����������
	GeneratorPars.AnalogOutputsPars.GainIndex = AnalogOutputsGaindBLMDComboBox->ItemIndex;
	// ��������� �������� �� ���������� ������� 10 � 50 ��
	CalculateAnalogOutputsGains();

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::CalculateAnalogOutputsGains(void)
{
	// ��������� ������� �� ������ 10 �� � �
	ApmlitudeOutput10OhmInVLMDStaticText->Caption = FormatFloat("0.000", Output10OhmVArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);
	// ��������� ������� �� ������ 10 �� � ��
	ApmlitudeOutput10OhmIndBLMDStaticText->Caption = FormatFloat("0.0", Output10OhmdBArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);
	// ��������� ������� �� ������ 50 �� � �
	ApmlitudeOutput50OhmInVLMDStaticText->Caption = FormatFloat("0.000", Output50OhmVArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);
	// ��������� ������� �� ������ 50 �� � ��
	ApmlitudeOutput50OhmIndBLMDStaticText->Caption = FormatFloat("0.0", Output50OhmdBArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);

}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::OffsetTypeOutput10OhmLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// �������� ��� �������� �� ������ 10 ��
	GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource = OffsetTypeOutput10OhmLMDRadioGroup->ItemIndex;
	//
	if(GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource == INTERNAL_OUTPUT_10_OHM_OFFSET_E310)
	{
		OffsetValueLMDGroupBox->Enabled = true;
		WinControlElements(OffsetValueLMDGroupBox, true);
	}
	else
	{
		OffsetValueLMDGroupBox->Enabled = false;
		WinControlElements(OffsetValueLMDGroupBox, false);
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredOffsetValueOutput10OhmLMDEditKeyPress(TObject *Sender, char &Key)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	if(!isdigit(Key))
	{
		if(Key != VK_BACK)
		{
			if(Key != '.')
			{
				if(Key != '-')
				{
					Key = NULL;
				}
			}
		}
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredOffsetValueOutput10OhmLMDEditChange(TObject *Sender)
{
	LONG OffsetValueCode;
	double DesiredOffsetValue;

	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// �������� ������ � ���������� ��������� ����������� �������� �� ������ 10 �� � �
	if(!DesiredOffsetValueOutput10OhmLMDEdit->Text.IsEmpty() && (DesiredOffsetValueOutput10OhmLMDEdit->Text != AnsiString("-")))
	{
		DesiredOffsetValueDiplay(true);
		try
		{
			// ��������� ������ �������� ����������� ����������� ��������
			DesiredOffsetValue = DesiredOffsetValueOutput10OhmLMDEdit->Text.ToDouble();
			// ������ ��� ����������� ����������� ��������
			OffsetValueCode = 255.0*(DesiredOffsetValue + 4.0)/8.0 + 0.5;
			// ���� ����� - ����������
			if(OffsetValueCode < 0x0 ) OffsetValueCode = 0x0;
			else if(OffsetValueCode > 255) OffsetValueCode = 255;
		}
		catch(EConvertError&)
		{
			if(!DesiredOffsetValueOutput10OhmLMDEdit->Text.IsEmpty())
				Application->MessageBox("���������� ������ �������������� ������ � �����.", "������ DesiredStartFrequencyValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// �������� ���������� �������� ����������� ��������
		ActualOffsetValue = 8.0*OffsetValueCode/255.0 - 4.0;
		ActualOffsetValueOutput10OhmLMDStaticText->Caption = FormatFloat("0.000", ActualOffsetValue);
	}
	else
	{
		DesiredOffsetValueDiplay(false);
	}

	// ����� �����������������...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::DesiredOffsetValueDiplay(bool Flag)
{
	ActualOffsetValueOutput10OhmLabelLMDStaticText->Enabled = Flag;
	ActualOffsetValueOutput10OhmLMDStaticText->Enabled = Flag;
	ActualOffsetValueOutput10OhmLMDStaticText->Caption = Flag ? "" : "-----";
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::ControlGeneratorLMDButtonClick(TObject *Sender)
{
	// �����������������...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	do
	{
		if((GeneratorState ^ 0x1) == GENERATOR_IS_ON)
		{
			// ��������� ������� ��� ��������� ������ ����������
			GetGeneratorPars();
			//
			if(!pModule->SET_GENERATOR_PARS(&GeneratorPars)) { Application->MessageBox("�� ���� ��������� ������� SET_GENERATOR_PARS()","������ ControlGeneratorLMDButtonClick()!!!", MB_OK + MB_ICONINFORMATION); break; }
			// ��������� ���������
			if(!pModule->START_GENERATOR()) { Application->MessageBox("�� ���� ��������� ������� START_GENERATOR()","������ ControlGeneratorLMDButtonClick()!!!", MB_OK + MB_ICONINFORMATION); break; }

			GetControlElementState();
			ControlElements(false);
			ControlGeneratorLMDButton->Font->Color = clMaroon;
			ControlGeneratorLMDButton->Caption = "������� ����������";
		}
		else
		{
			ControlElements(true);
			ControlGeneratorLMDButton->Font->Color = clGreen;
			ControlGeneratorLMDButton->Caption = "������ ����������";
			// ������� ����������
			if(!pModule->STOP_GENERATOR()) { Application->MessageBox("�� ���� ��������� ������� STOP_GENERATOR()","������ ControlGeneratorLMDButtonClick()!!!", MB_OK + MB_ICONINFORMATION); break; }
		}

		// ������� ��������� ����������
		GeneratorState ^= 0x1;
	}
	while(false);

	// ����� �����������������...
	IsReenteringInProgress = false;
}
//---------------------------------------------------------------------------


