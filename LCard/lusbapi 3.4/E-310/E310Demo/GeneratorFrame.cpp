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

// панель параметров генератора
TE310GeneratorFrame *E310GeneratorFrame;


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TE310GeneratorFrame::TE310GeneratorFrame(TComponent* Owner) : TFrame(Owner)
{
	WORD i;
	double Ampl = sqrt(2.0) * 0.77459666924148;		// амплитуда равная 0 дБ на нагрузке 600 Ом

	// разделитель
	DecimalSeparator = '.';

	// обнулим адреса вспомогательных массивов
	ControlElementsAddr = NULL;
	ControlElementsState = NULL;
	// обнулим общее кол-во управляющих элементов
	ControlElementsQuantity = 0x0;

	// максимальный код частоты DDS
	MAX_FREQ_DDS_CODE = 0x1 << 24;
	// минимаксные значения частоты тактирующего сигнала генератора в кГц
	MIN_MASTER_CLOCK = 1.0;
	MAX_MASTER_CLOCK = 50000.0;
	// частота внутреннего тактирующего сигнала генератора в кГц
	INTERNAL_MASTER_CLOCK = 50000.0;
	// инициализация масcива умножителей интервалов приращения для микросхемы DDS
	IncrementIntervalMultiplierArray[0x0] = 1.0;		IncrementIntervalMultiplierArray[0x1] = 5.0;
	IncrementIntervalMultiplierArray[0x2] = 100.0;	IncrementIntervalMultiplierArray[0x3] = 500.0;

	// инициализация масcива усилений выходного тракта генератора в дБ
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
	// заполнение массива доступных амплитуд на выходе 10 и 50 Ом в дБ
	for(i = 0x0; i < ANALOG_OUTPUT_GAINS_QUANTITY_E310; i++)
	{
		Output10OhmdBArray[i] = GeneratorGaindBArray[i];
		Output50OhmdBArray[i] = GeneratorGaindBArray[i] - 6.0;
	}

	// заполнение массива доступных амплитуд на выходе 10 и 50 Ом в В
	for(i = 0x0; i < ANALOG_OUTPUT_GAINS_QUANTITY_E310; i++)
	{
		Output10OhmVArray[i] = Ampl * pow(10.0, Output10OhmdBArray[i]/20.0);
		Output50OhmVArray[i] = Ampl * pow(10.0, Output50OhmdBArray[i]/20.0);
	}

	// инициализация ComboBox с индексами усиления выходного тракта генератора
	InitAnalogOutputsGaindBComboBox();

	// сбросим состояние генератора
	GeneratorState = GENERATOR_IS_OFF;

	// установим флажок реентерабельности
	IsReenteringInProgress = true;

	// -=== сброс всех компонентов
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

	// сбросим флажок реентерабельности
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
// инициализация ComboBox с индексами усиления выходного тракта генератора
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

	// найдем общее кол-во управляющих элементов
	for(i = count = 0x0; i < this->ComponentCount; i++)
	{
		TControl *Control;
		Control = dynamic_cast<TControl *>(this->Components[i]);
		if(Control) count++;
	}

	// если нужно - выделим под это память
	if(!ControlElementsAddr || !ControlElementsState || (ControlElementsQuantity != count))
	{
		if(ControlElementsAddr)		{ delete[] ControlElementsAddr; ControlElementsAddr = NULL; }
		if(ControlElementsState)	{ delete[] ControlElementsState; ControlElementsState = NULL; }

		ControlElementsQuantity = count;

		ControlElementsAddr = new TControl* [ControlElementsQuantity];
		ControlElementsState = new bool[ControlElementsQuantity];
	}

	// запомним адреса и состояния всех управляющих элементов
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
		if(!pModule->STOP_GENERATOR()) { Application->MessageBox("Не могу выполнить функцию STOP_GENERATOR()","Ошибка ~TE310GeneratorFrame()!!!", MB_OK + MB_ICONINFORMATION); return; }

	if(ControlElementsAddr)		{ delete[] ControlElementsAddr; ControlElementsAddr = NULL; }
	if(ControlElementsState)	{ delete[] ControlElementsState; ControlElementsState = NULL; }
}

//---------------------------------------------------------------------------
// попробуем отобразить параметры работы генератора на панели
//---------------------------------------------------------------------------
void TE310GeneratorFrame::InitGeneratorFramePars(ILE310 * const pModule, GENERATOR_PARS_E310 * const GeneratorPars)
{
	// сохраним указатель на интерфейс модуля
	this->pModule = pModule;
	// сохраним параметры работы генератора
	this->GeneratorPars = *GeneratorPars;

	// если нужно остановим генератор
	if(GeneratorState == GENERATOR_IS_ON)
	{
		// измениим состояние генератора
		GeneratorState ^= 0x1;

		ControlElements(true);
		ControlGeneratorLMDButton->Font->Color = clGreen;
		ControlGeneratorLMDButton->Caption = "Запуск генератора";
	}

	// источник тактовой частоты генератора
	MasterClockSourceLMDRadioGroup->ItemIndex = this->GeneratorPars.MasterClockSource;
	// величина тактовой частоты генератора
	InternalMasterClockValueLMDStaticText->Caption = FormatFloat("0.00", INTERNAL_MASTER_CLOCK);
	// если нужно - подрихтуем внешнюю тактовую частоту
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
	// тип циклического автосканирования выходного сигнала
	CyclicAutoScanLMDRadioGroup->ItemIndex = this->GeneratorPars.CyclicAutoScanType;
//	CyclicAutoScanLMDRadioGroupChange(CyclicAutoScanLMDRadioGroup, NULL);
//	IncrementTypeLMDRadioGroupChange(IncrementTypeLMDRadioGroup, NULL);

	// начальная частота генератора в кГц
	DesiredStartFrequencyValueLMDEdit->Text = FormatFloat("0.0000", this->GeneratorPars.StartFrequency);
	// частота приращения в кГц
	DesiredFrequencyIncrementValueLMDEdit->Text = FormatFloat("0.0000", this->GeneratorPars.FrequencyIncrements);
	// кол-во приращений частоты сканирования
	NumberOfIncrementsValueLMDEdit->Text = FormatFloat("0", this->GeneratorPars.NumberOfIncrements);

	// тип линии "INTERRUPT" для останова работы генератора
	InterrupLineTypeLMDRadioGroup->ItemIndex = this->GeneratorPars.InterrupLineType;
	// разрешение сигнала генератора на цифровом выходе "Меандр"
	EnaSquareWaveOutputLMDCheckBox->Checked = this->GeneratorPars.SquareWaveOutputEna ? true : false;

	// тип интервала приращения
	BaseIntervalTypeLMDRadioGroup->ItemIndex = this->GeneratorPars.IncrementIntervalPars.BaseIntervalType;
	// индекс умножителя интервала приращения
	BaseIntervalMultiplierLMDRadioGroup->ItemIndex = this->GeneratorPars.IncrementIntervalPars.MultiplierIndex;
	// кол-во циклов (либо MCLK, либо периодов сигнала) в интервале приращения
	BaseIntervalsNumberLMDEdit->Text = FormatFloat("0", this->GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber);

	CyclicAutoScanLMDRadioGroupChange(CyclicAutoScanLMDRadioGroup, NULL);
	IncrementTypeLMDRadioGroupChange(IncrementTypeLMDRadioGroup, NULL);

	// тип аналогового сигнала на выходах 10 и 50 Ом
	AnalogOutputsTypeLMDRadioGroup->ItemIndex = this->GeneratorPars.AnalogOutputsPars.SignalType;
	// индекс усиления выходного тракта генератора
	AnalogOutputsGaindBLMDComboBox->ItemIndex = this->GeneratorPars.AnalogOutputsPars.GainIndex;
	// вычисляем усиления на аналоговых выходах 10 и 50 Ом
	CalculateAnalogOutputsGains();
	// тип смещения на выходе 10 Ом
	OffsetTypeOutput10OhmLMDRadioGroup->ItemIndex = this->GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource;
	OffsetTypeOutput10OhmLMDRadioGroupChange(OffsetTypeOutput10OhmLMDRadioGroup, NULL);
	// величина внутреннего смещение на выходе 10 Ом в В
	DesiredOffsetValueOutput10OhmLMDEdit->Text = this->GeneratorPars.AnalogOutputsPars.Output10OhmOffset;
}

//---------------------------------------------------------------------------
// попробуем собрать все параметры работы генератора из данных панели
//---------------------------------------------------------------------------
void TE310GeneratorFrame::GetGeneratorFramePars(GENERATOR_PARS_E310 * const GeneratorPars)
{
	// соберем параметры
	GetGeneratorPars();
	// возвратим их
	*GeneratorPars = this->GeneratorPars;
}


//---------------------------------------------------------------------------
// попробуем собрать все параметры работы генератора
//---------------------------------------------------------------------------
void TE310GeneratorFrame::GetGeneratorPars(void)
{
	try
	{
		// источник тактовой частоты генератора
		GeneratorPars.MasterClockSource = MasterClockSourceLMDRadioGroup->ItemIndex;
		// величина тактовой частоты генератора
		if(GeneratorPars.MasterClockSource == INTERNAL_MASTER_CLOCK_E310)
			GeneratorPars.MasterClock = INTERNAL_MASTER_CLOCK;
		else
			GeneratorPars.MasterClock = ExternalMasterClockValueLMDEdit->Text.ToDouble();
		// тип циклического автосканирования выходного сигнала
		GeneratorPars.CyclicAutoScanType = CyclicAutoScanLMDRadioGroup->ItemIndex;

		// начальная частота генератора в кГц
		GeneratorPars.StartFrequency = DesiredStartFrequencyValueLMDEdit->Text.ToDouble();
		// частота приращения в кГц
		GeneratorPars.FrequencyIncrements = DesiredFrequencyIncrementValueLMDEdit->Text.ToDouble();
		// кол-во приращений частоты сканирования
		GeneratorPars.NumberOfIncrements = NumberOfIncrementsValueLMDEdit->Text.ToDouble();

		// тип интервала приращения
		GeneratorPars.IncrementIntervalPars.BaseIntervalType = BaseIntervalTypeLMDRadioGroup->ItemIndex;
		// индекс умножителя интервала приращения
		GeneratorPars.IncrementIntervalPars.MultiplierIndex = BaseIntervalMultiplierLMDRadioGroup->ItemIndex;
		// кол-во циклов (либо MCLK, либо периодов сигнала) в интервале приращения
		GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber = BaseIntervalsNumberLMDEdit->Text.ToDouble();

		// тип аналогового сигнала на выходах 10 и 50 Ом
		GeneratorPars.AnalogOutputsPars.SignalType = AnalogOutputsTypeLMDRadioGroup->ItemIndex;
		// индекс усиления выходного тракта генератора
		GeneratorPars.AnalogOutputsPars.GainIndex = AnalogOutputsGaindBLMDComboBox->ItemIndex;
		// тип смещения на выходе 10 Ом
		GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource = OffsetTypeOutput10OhmLMDRadioGroup->ItemIndex;
		// величина внутреннего смещение на выходе 10 Ом в В
		GeneratorPars.AnalogOutputsPars.Output10OhmOffset = DesiredOffsetValueOutput10OhmLMDEdit->Text.ToDouble();


		// тип инкрементации частоты генератора
		GeneratorPars.IncrementType = IncrementTypeLMDRadioGroup->ItemIndex;
		// тип линии "CTRL" для (управления инкрементацией частоты)/старта генератора: внутренняя (от MCU)
		GeneratorPars.CtrlLineType = CtrlLineTypeLMDRadioGroup->ItemIndex;
		// тип линии "INTERRUPT" для останова работы генератора
		GeneratorPars.InterrupLineType = InterrupLineTypeLMDRadioGroup->ItemIndex;
		// формирование выходного синхросигнала на линии "SYNCOUT"
		GeneratorPars.SynchroOutEna = EnaSyncoutLMDCheckBox->Checked ? 0x1 : 0x0;
		GeneratorPars.SynchroOutType = SyncoutTypeLMDRadioGroup->ItemIndex;
		// разрешение сигнала генератора на цифровом выходе "Меандр"
		GeneratorPars.SquareWaveOutputEna = EnaSquareWaveOutputLMDCheckBox->Checked ? 0x1 : 0x0;
	}
	catch(...)
	{
		Application->MessageBox("Обнаружено исключение.", "Ошибка GetGeneratorPars()!!!", MB_OK + MB_ICONINFORMATION);
	}

}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::MasterClockSourceLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
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

		// конец реентерабельности...
		IsReenteringInProgress = false;

		// подрихтуем частоты
		DesiredStartFrequencyValueLMDEditChange(DesiredStartFrequencyValueLMDEdit);
		DesiredFrequencyIncrementValueLMDEditChange(DesiredFrequencyIncrementValueLMDEdit);
		if(GeneratorPars.IncrementType == AUTO_INCREMENT_E310)
		{
			WinControlElements(IncrementIntervalParsLMDGroupBox, true);
			if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
			{
				IncrementIntervalDiplay(true);
				// вычисляем временнОй интервал приращения в мс
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
			Application->MessageBox("Обнаружена ошибка преобразования строки в число.", "Ошибка ExternalMasterClockValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
	}

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::ExternalMasterClockValueLMDEditKeyPress(TObject *Sender, char &Key)
{
	// реентерабельность...
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

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::ExternalMasterClockValueLMDEditChange(TObject *Sender)
{
	double DesiredExternalMasterClockFrequency;

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// проверим строку с частотой внешнего тактирующего сигнала
	if(!ExternalMasterClockValueLMDEdit->Text.IsEmpty())
	{
		try
		{
			// попробуем узнать величину задаваемой частоты внешнего тактирующего сигнала
			DesiredExternalMasterClockFrequency = ExternalMasterClockValueLMDEdit->Text.ToDouble();
			// если нужно - подрихтуем
			if(DesiredExternalMasterClockFrequency < MIN_MASTER_CLOCK) DesiredExternalMasterClockFrequency = MIN_MASTER_CLOCK;
			else if(DesiredExternalMasterClockFrequency > MAX_MASTER_CLOCK) DesiredExternalMasterClockFrequency = MAX_MASTER_CLOCK;
		}
		catch(EConvertError&)
		{
			if(!ExternalMasterClockValueLMDEdit->Text.IsEmpty())
				Application->MessageBox("Обнаружена ошибка преобразования строки в число.", "Ошибка ExternalMasterClockValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// запомним частоту внешнего тактирующего сигнала
		GeneratorPars.MasterClock = DesiredExternalMasterClockFrequency;
		ExternalMasterClockValueLMDEdit->Text = FormatFloat("0.0", DesiredExternalMasterClockFrequency);

		// конец реентерабельности...
		IsReenteringInProgress = false;

		// подрихтуем частоты
		DesiredStartFrequencyValueLMDEditChange(DesiredStartFrequencyValueLMDEdit);
		DesiredFrequencyIncrementValueLMDEditChange(DesiredFrequencyIncrementValueLMDEdit);
		if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
		{
			IncrementIntervalDiplay(true);
			// вычисляем временнОй интервал приращения в мс
			CalculateIncrementInterval();
		}
		else
			IncrementIntervalDiplay(false);
	}

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::CyclicAutoScanLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// тип циклического автосканирования выходного сигнала
	GeneratorPars.CyclicAutoScanType = CyclicAutoScanLMDRadioGroup->ItemIndex;
	// особые условия при CyclicAutoScanType равном NO_CYCLIC_AUTOSCAN_E310
	if(GeneratorPars.CyclicAutoScanType != NO_CYCLIC_AUTOSCAN_E310)
	{
		// тип инкрементации частоты генератора: внутренняя (автоматическая)
		IncrementTypeLMDRadioGroup->Enabled = false;
		IncrementTypeLMDRadioGroup->ItemIndex = AUTO_INCREMENT_E310;
		// тип линии "CTRL" для (управления инкрементацией частоты)/старта генератора: внутренняя (от MCU)
		CtrlLineTypeLMDRadioGroup->Enabled = false;
		CtrlLineTypeLMDRadioGroup->ItemIndex =INTERNAL_CTRL_LINE_E310;
		// формирование выходного синхросигнала на линии "SYNCOUT"
		SyncoutLineParsLMDGroupBox->Enabled = false;
		WinControlElements(SyncoutLineParsLMDGroupBox, false);
		EnaSyncoutLMDCheckBox->Checked = true;
		SyncoutTypeLMDRadioGroup->ItemIndex = SYNCOUT_AT_END_OF_SCAN_E310;
	}
	else
	{
		// тип инкрементации частоты генератора: внутренняя (автоматическая)
		IncrementTypeLMDRadioGroup->Enabled = true;
		IncrementTypeLMDRadioGroup->ItemIndex = GeneratorPars.IncrementType;
		// тип линии "CTRL" для (управления инкрементацией частоты)/старта генератора: внутренняя (от MCU)
		CtrlLineTypeLMDRadioGroup->Enabled = true;
		CtrlLineTypeLMDRadioGroup->ItemIndex = GeneratorPars.CtrlLineType;
		// формирование выходного синхросигнала на линии "SYNCOUT"
		SyncoutLineParsLMDGroupBox->Enabled = true;
		WinControlElements(SyncoutLineParsLMDGroupBox, true);
		EnaSyncoutLMDCheckBox->Checked = GeneratorPars.SynchroOutEna ? true : false;
		SyncoutTypeLMDRadioGroup->ItemIndex = GeneratorPars.SynchroOutType;
	}

	// конец реентерабельности...
	IsReenteringInProgress = false;

	// если нужно - подрихтуем интервал приращения
	IncrementTypeLMDRadioGroupChange(IncrementIntervalParsLMDGroupBox, NULL);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::IncrementTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// особые условия при CyclicAutoScanType равном NO_CYCLIC_AUTOSCAN_E310
	if(GeneratorPars.CyclicAutoScanType == NO_CYCLIC_AUTOSCAN_E310)
	{
		// тип инкрементации частоты генератора
		GeneratorPars.IncrementType = IncrementTypeLMDRadioGroup->ItemIndex;
		//
		if(GeneratorPars.IncrementType == AUTO_INCREMENT_E310)
		{
			IncrementIntervalParsLMDGroupBox->Enabled = true;
			WinControlElements(IncrementIntervalParsLMDGroupBox, true);

			// конец реентерабельности...
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

			// конец реентерабельности...
			IsReenteringInProgress = false;

			BaseIntervalTypeLMDRadioGroupChange(BaseIntervalTypeLMDRadioGroup, NULL);
		}
		else
		{
			IncrementIntervalParsLMDGroupBox->Enabled = false;
			WinControlElements(IncrementIntervalParsLMDGroupBox, false);
		}
	}

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::CtrlLineTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// тип линии "CTRL" для (управления инкрементацией частоты)/старта генератора
	GeneratorPars.CtrlLineType = CtrlLineTypeLMDRadioGroup->ItemIndex;

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::InterrupLineTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// тип линии "INTERRUPT" для останова работы генератора
	GeneratorPars.InterrupLineType = InterrupLineTypeLMDRadioGroup->ItemIndex;

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::EnaSyncoutLMDCheckBoxChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// разрешение синхросигнала генератора на выходной линии "SYNCOUT"
	GeneratorPars.SynchroOutEna = EnaSyncoutLMDCheckBox->Checked ? TRUE : FALSE;

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::SyncoutTypeLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// тип формирования синхросигнала генератора
	GeneratorPars.SynchroOutType = SyncoutTypeLMDRadioGroup->ItemIndex;

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::EnaSquareWaveOutputLMDCheckBoxChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// разрешение сигнала генератора на цифровом выходе "Меандр"
	GeneratorPars.SquareWaveOutputEna = EnaSquareWaveOutputLMDCheckBox->Checked ? TRUE : FALSE;

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredStartFrequencyValueLMDEditKeyPress(TObject *Sender, char &Key)
{
	// реентерабельность...
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

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredStartFrequencyValueLMDEditChange(TObject *Sender)
{
	DWORD StartFreqCode;
	double DesiredStartFrequency;

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// проверим строку с задаваемой стартовой частотой
	if(!DesiredStartFrequencyValueLMDEdit->Text.IsEmpty())
	{
		DesiredStartFrequencyDiplay(true);
		try
		{
			// попробуем узнать величину задаваемой стартовой частоты
			DesiredStartFrequency = DesiredStartFrequencyValueLMDEdit->Text.ToDouble();
			// узнаем код задаваемой стартовой частоты
			StartFreqCode = (DesiredStartFrequency / GeneratorPars.MasterClock) * MAX_FREQ_DDS_CODE + 0.5;
			// если нужно - подрихтуем
			if(StartFreqCode > MAX_FREQ_DDS_CODE) StartFreqCode = MAX_FREQ_DDS_CODE;
		}
		catch(EConvertError&)
		{
			if(!DesiredStartFrequencyValueLMDEdit->Text.IsEmpty())
				Application->MessageBox("Обнаружена ошибка преобразования строки в число.", "Ошибка DesiredStartFrequencyValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// вычислим актуальную стартовую частоту генератора
		ActualStartFrequency = GeneratorPars.MasterClock * StartFreqCode / MAX_FREQ_DDS_CODE;
		ActualStartFrequencyValueLMDStaticText->Caption = FormatFloat("0.0000", ActualStartFrequency);
		// вычисляем актуальную конечную частоту генератора
		CalculateStopFrequency();
	}
	else
	{
		ActualStartFrequency = -10.0;
		DesiredStartFrequencyDiplay(false);
		CalculateStopFrequency();
	}

	// конец реентерабельности...
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
	// реентерабельность...
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

	// конец реентерабельности...
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

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// проверим строку с задаваемой частотой приращения
	if(!DesiredFrequencyIncrementValueLMDEdit->Text.IsEmpty() && (DesiredFrequencyIncrementValueLMDEdit->Text != AnsiString("-")))
	{
		DesiredDeltaFrequencyDiplay(true);
		try
		{
			// попробуем узнать величину задаваемой частоты приращения
			DesiredDeltaFrequency = DesiredFrequencyIncrementValueLMDEdit->Text.ToDouble();
			DeltaFreqSign = (DesiredDeltaFrequency >= 0.0) ? 1.0 : (-1.0);
			// узнаем код задаваемой частоты приращения
			DeltaFreqCode = (fabs(DesiredDeltaFrequency) / GeneratorPars.MasterClock) * MAX_FREQ_DDS_CODE + 0.5;
			// если нужно - подрихтуем
			if(DeltaFreqCode > MAX_FREQ_DDS_CODE) DeltaFreqCode = MAX_FREQ_DDS_CODE;
		}
		catch(EConvertError&)
		{
			if(!DesiredFrequencyIncrementValueLMDEdit->Text.IsEmpty())
				Application->MessageBox("Обнаружена ошибка преобразования строки в число.", "Ошибка DesiredStartFrequencyValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// вычислим актуальную частоту приращения генератора
		ActualDeltaFrequency = GeneratorPars.MasterClock * DeltaFreqSign * DeltaFreqCode / MAX_FREQ_DDS_CODE;
		ActualFrequencyIncrementLMDStaticText->Caption = FormatFloat("0.0000", ActualDeltaFrequency);
		// вычисляем актуальную конечную частоту генератора
		CalculateStopFrequency();
	}
	else
	{
		ActualDeltaFrequency = 0.0;
		DesiredDeltaFrequencyDiplay(false);
		CalculateStopFrequency();
	}

	// конец реентерабельности...
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
	// реентерабельность...
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

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::NumberOfIncrementsValueLMDEditChange(TObject *Sender)
{
	int NumberOfIncrementsValue;

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	NumberOfIncrementsValue = abs(NumberOfIncrementsValueLMDEdit->Text.ToIntDef(0x1));
	if(NumberOfIncrementsValue > 4095) NumberOfIncrementsValue = 4095;
	NumberOfIncrementsValueLMDEdit->Text = FormatFloat("0", GeneratorPars.NumberOfIncrements = NumberOfIncrementsValue);
	// вычисляем актуальную конечную частоту генератора
	CalculateStopFrequency();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
// функция вычисления актуальной конечной частоты генератора
//---------------------------------------------------------------------------
bool TE310GeneratorFrame::CalculateStopFrequency(void)
{
	// проверим на отрицительность - чего не должно быть!
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

		// вычислим конечную частоту
		DesiredStopFrequency = ActualStartFrequency + ActualDeltaFrequency * GeneratorPars.NumberOfIncrements;
		if((0.0 < DesiredStopFrequency) && (DesiredStopFrequency < (GeneratorPars.MasterClock / 2.0)))
		{
			// узнаем код задаваемой частоты приращения
			StopFreqCode = (DesiredStopFrequency / GeneratorPars.MasterClock) * MAX_FREQ_DDS_CODE + 0.5;
			// если нужно - подрихтуем
			if(StopFreqCode > MAX_FREQ_DDS_CODE) StopFreqCode = MAX_FREQ_DDS_CODE;
			// вычислим актуальную частоту приращения генератора
			ActualStopFrequency = GeneratorPars.MasterClock * StopFreqCode / MAX_FREQ_DDS_CODE;

			FinalFrequencyLabelLMDStaticText->Enabled = true;
			FinalFrequencyLabelLMDStaticText->Enabled = true;
			FinalFrequencyValueLMDStaticText->Enabled = true;
			FinalFrequencyValueLMDStaticText->Caption = FormatFloat("0.0000", ActualStopFrequency);
			FinalFrequencyUnitLMDStaticText->Enabled = true;
		}
		else
		{
			if(DesiredStopFrequency < 0.0) Application->MessageBox("Конечная частота оказалась меньше 0.0!!!","Ошибка CalculateStopFrequency()!!!", MB_OK + MB_ICONINFORMATION);
			else Application->MessageBox("Конечная частота оказалась больше MasterClock/2.0!!!","Ошибка CalculateStopFrequency()!!!", MB_OK + MB_ICONINFORMATION);
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
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// тип интервала приращения
	GeneratorPars.IncrementIntervalPars.BaseIntervalType = BaseIntervalTypeLMDRadioGroup->ItemIndex;
	if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
	{
		IncrementIntervalDiplay(true);
		// вычисляем временнОй интервал приращения в мс
		CalculateIncrementInterval();
	}
	else
		IncrementIntervalDiplay(false);

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::BaseIntervalMultiplierLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// индекс умножителя интервала приращения
	GeneratorPars.IncrementIntervalPars.MultiplierIndex = BaseIntervalMultiplierLMDRadioGroup->ItemIndex;

	if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
	{
		IncrementIntervalDiplay(true);
		// вычисляем временнОй интервал приращения в мс
		CalculateIncrementInterval();
	}
	else
		IncrementIntervalDiplay(false);

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::BaseIntervalsNumberLMDEditKeyPress(TObject *Sender, char &Key)
{
	// реентерабельность...
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

	// конец реентерабельности...
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
		// реентерабельность...
		if(IsReenteringInProgress) return;
		else IsReenteringInProgress = true;

		// проверим строку с кол-вом циклов (либо MCLK, либо периодов сигнала) в интервале приращения
		if(BaseIntervalsNumberLMDEdit->Text.IsEmpty()) BaseIntervalsNumberLMDEdit->Text = 2;
		//
		try
		{
			// попробуем узнать задаваемое кол-во циклов (либо MCLK, либо периодов сигнала) в интервале приращения
			IncrementIntervalNumber = BaseIntervalsNumberLMDEdit->Text.ToIntDef(2);
			// если нужно - подрихтуем
			if(IncrementIntervalNumber >= 2047) IncrementIntervalNumber = 2047;
			else if(IncrementIntervalNumber < 2) IncrementIntervalNumber = 2;
		}
		catch(EConvertError&)
		{
			Application->MessageBox("Обнаружена ошибка преобразования строки в число.", "Ошибка IncrementIntervalNumberLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// запомним кол-во циклов (либо MCLK, либо периодов сигнала) в интервале приращения
		GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber = IncrementIntervalNumber;
		BaseIntervalsNumberLMDEdit->Text = FormatFloat("0", IncrementIntervalNumber);

		if(GeneratorPars.IncrementIntervalPars.BaseIntervalType == CLOCK_PERIOD_INCREMENT_INTERVAL_E310)
		{
			IncrementIntervalDiplay(true);
			// вычисляем временнОй интервал приращения в мс
			CalculateIncrementInterval();
		}
		else IncrementIntervalDiplay(false);

		// конец реентерабельности...
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
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// тип аналогового сигнала на выходах 10 и 50 Ом
	GeneratorPars.AnalogOutputsPars.SignalType = AnalogOutputsTypeLMDRadioGroup->ItemIndex;

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::AnalogOutputsGaindBLMDComboBoxChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// индекс усиления выходного тракта генератора
	GeneratorPars.AnalogOutputsPars.GainIndex = AnalogOutputsGaindBLMDComboBox->ItemIndex;
	// вычисляем усиления на аналоговых выходах 10 и 50 Ом
	CalculateAnalogOutputsGains();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TE310GeneratorFrame::CalculateAnalogOutputsGains(void)
{
	// амплитуда сигнала на выходе 10 Ом в В
	ApmlitudeOutput10OhmInVLMDStaticText->Caption = FormatFloat("0.000", Output10OhmVArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);
	// амплитуда сигнала на выходе 10 Ом в дБ
	ApmlitudeOutput10OhmIndBLMDStaticText->Caption = FormatFloat("0.0", Output10OhmdBArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);
	// амплитуда сигнала на выходе 50 Ом в В
	ApmlitudeOutput50OhmInVLMDStaticText->Caption = FormatFloat("0.000", Output50OhmVArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);
	// амплитуда сигнала на выходе 50 Ом в дБ
	ApmlitudeOutput50OhmIndBLMDStaticText->Caption = FormatFloat("0.0", Output50OhmdBArray[this->GeneratorPars.AnalogOutputsPars.GainIndex]);

}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::OffsetTypeOutput10OhmLMDRadioGroupChange(TObject *Sender, int ButtonIndex)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// запомним тип смещения на выходе 10 Ом
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

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredOffsetValueOutput10OhmLMDEditKeyPress(TObject *Sender, char &Key)
{
	// реентерабельность...
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

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TE310GeneratorFrame::DesiredOffsetValueOutput10OhmLMDEditChange(TObject *Sender)
{
	LONG OffsetValueCode;
	double DesiredOffsetValue;

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// проверим строку с задаваемой величиной внутреннего смещения на выходе 10 Ом в В
	if(!DesiredOffsetValueOutput10OhmLMDEdit->Text.IsEmpty() && (DesiredOffsetValueOutput10OhmLMDEdit->Text != AnsiString("-")))
	{
		DesiredOffsetValueDiplay(true);
		try
		{
			// попробуем узнать величину задаваемого внутреннего смещения
			DesiredOffsetValue = DesiredOffsetValueOutput10OhmLMDEdit->Text.ToDouble();
			// узнаем код задаваемого внутреннего смещения
			OffsetValueCode = 255.0*(DesiredOffsetValue + 4.0)/8.0 + 0.5;
			// если нужно - подрихтуем
			if(OffsetValueCode < 0x0 ) OffsetValueCode = 0x0;
			else if(OffsetValueCode > 255) OffsetValueCode = 255;
		}
		catch(EConvertError&)
		{
			if(!DesiredOffsetValueOutput10OhmLMDEdit->Text.IsEmpty())
				Application->MessageBox("Обнаружена ошибка преобразования строки в число.", "Ошибка DesiredStartFrequencyValueLMDEditChange()!!!", MB_OK + MB_ICONINFORMATION);
		}

		// вычислим актуальную величину внутреннего смещения
		ActualOffsetValue = 8.0*OffsetValueCode/255.0 - 4.0;
		ActualOffsetValueOutput10OhmLMDStaticText->Caption = FormatFloat("0.000", ActualOffsetValue);
	}
	else
	{
		DesiredOffsetValueDiplay(false);
	}

	// конец реентерабельности...
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
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	do
	{
		if((GeneratorState ^ 0x1) == GENERATOR_IS_ON)
		{
			// попробуем собрать все параметры работы генератора
			GetGeneratorPars();
			//
			if(!pModule->SET_GENERATOR_PARS(&GeneratorPars)) { Application->MessageBox("Не могу выполнить функцию SET_GENERATOR_PARS()","Ошибка ControlGeneratorLMDButtonClick()!!!", MB_OK + MB_ICONINFORMATION); break; }
			// запускаем генератор
			if(!pModule->START_GENERATOR()) { Application->MessageBox("Не могу выполнить функцию START_GENERATOR()","Ошибка ControlGeneratorLMDButtonClick()!!!", MB_OK + MB_ICONINFORMATION); break; }

			GetControlElementState();
			ControlElements(false);
			ControlGeneratorLMDButton->Font->Color = clMaroon;
			ControlGeneratorLMDButton->Caption = "Останов генератора";
		}
		else
		{
			ControlElements(true);
			ControlGeneratorLMDButton->Font->Color = clGreen;
			ControlGeneratorLMDButton->Caption = "Запуск генератора";
			// останов генератора
			if(!pModule->STOP_GENERATOR()) { Application->MessageBox("Не могу выполнить функцию STOP_GENERATOR()","Ошибка ControlGeneratorLMDButtonClick()!!!", MB_OK + MB_ICONINFORMATION); break; }
		}

		// изменим состояние генератора
		GeneratorState ^= 0x1;
	}
	while(false);

	// конец реентерабельности...
	IsReenteringInProgress = false;
}
//---------------------------------------------------------------------------


