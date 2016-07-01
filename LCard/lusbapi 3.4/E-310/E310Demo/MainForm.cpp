//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "GeneratorFrame.h"
#include "MainForm.h"
#include "InitDevicesThread.h"
#include "About.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "LMDCustomBevelPanel"
#pragma link "LMDCustomControl"
#pragma link "LMDCustomGroupBox"
#pragma link "LMDCustomPanel"
#pragma link "LMDCustomParentPanel"
#pragma link "LMDGroupBox"
#pragma link "LMDBaseControl"
#pragma link "LMDBaseGraphicControl"
#pragma link "LMDBaseShape"
#pragma link "LMDButton"
#pragma link "LMDButtonControl"
#pragma link "LMDCheckBox"
#pragma link "LMDControl"
#pragma link "LMDCustomButton"
#pragma link "LMDCustomCheckBox"
#pragma link "LMDCustomPanelFill"
#pragma link "LMDShapeControl"
#pragma link "LMDStaticText"
#pragma link "LMDCustomButtonGroup"
#pragma link "LMDCustomRadioGroup"
#pragma link "LMDRadioGroup"
#pragma link "Pages"
#pragma link "BMPage"
#pragma resource "*.dfm"

// основная форма программы
TMainForm *MainForm;
// указатель на поток обнаружения модуля E-310
TInitDevicesThread *InitDevicesThread;
// указатель на интерфейс модуля E-310
ILE310 *pModule;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner)
{
	E310GeneratorFrame = new TE310GeneratorFrame(GeneratorParsLMDGroupBox);
	// фрейм создан?
	if(!E310GeneratorFrame) return;
	// попробуем определить фрейму достойного родителя
	E310GeneratorFrame->Parent = GeneratorParsLMDGroupBox;
	E310GeneratorFrame->Top		= 25;
	E310GeneratorFrame->Left	= 2;
	GeneratorParsLMDGroupBox->Width = E310GeneratorFrame->Left + E310GeneratorFrame->Width + 0x2;
	GeneratorParsLMDGroupBox->Height = E310GeneratorFrame->Top + E310GeneratorFrame->Height + 0x2;
	Width = GeneratorParsLMDGroupBox->Left + GeneratorParsLMDGroupBox->Width + 0x7;
	Height = GeneratorParsLMDGroupBox->Top + GeneratorParsLMDGroupBox->Height + 31;
	// делаем фрейм видимым
	E310GeneratorFrame->Visible = true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
	Application->OnShortCut = &ApplicationShortCut;

	// разделитель
	DecimalSeparator = '.';

	// сбросим признак завершения приложения
	IsAppTerminated = false;
	// служебные флажки программы
	IsReenteringInProgress = false;
	// установим признак запуска приложения
	IsProgramLaunching = true;
	// инициализация флажков для потока обнаружения оборудования
	IsInitDevicesThreadRunning = false;
	IsInitDevicesThreadDone = true;

	// реентерабельность...
//	IsReenteringInProgress = true;

}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormActivate(TObject *Sender)
{
	DecimalSeparator = '.';
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void __fastcall TMainForm::ApplicationShortCut(TWMKey &Msg, bool &Handled)
{
	if(Msg.CharCode == VK_ESCAPE)
	{
		// реентерабельность...
		if(IsReenteringInProgress) { Handled = true; return; }
		else IsReenteringInProgress = true;

		if(Application->MessageBox("Действительно хотите завершить работу?", "Подтвердите завершение!", MB_YESNO + MB_ICONQUESTION) == IDYES)
		{
			Handled = true;
			Application->Terminate();
		}

		// конец реентерабельности...
		IsReenteringInProgress = false;
	}
	else if(Msg.CharCode == VK_F1)
	{
//		SetModuleParsLMDButtonClick(SetModuleParsLMDButton);
	}
}

//---------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
	// запустим поток по обнаружению модуля E-310
//	StartInitModuleThread();
}

//---------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject *Sender)
{
	// проверим нужно ли запускать поток обнаружения железа
	if(IsProgramLaunching)
	{
		// запустим поток по обнаружению модуля E-310
		StartInitModuleThread();
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TMainForm::ControlElements(TWinControl *WinControlElement, bool EnabledFlag)
{
	DWORD i;

	for(i = 0x0; i < (DWORD)WinControlElement->ControlCount; i++)
	{
		TWinControl *WinControl;
		WinControl = dynamic_cast<TWinControl *>(WinControlElement->Controls[i]);
		if(WinControl) ControlElements(dynamic_cast<TWinControl *>(WinControlElement->Controls[i]), EnabledFlag);

		TControl *Control;
		Control = dynamic_cast<TControl *>(WinControlElement->Controls[i]);
		if(Control) { Control->Enabled = EnabledFlag;  }
	}
}

//---------------------------------------------------------------------------
// запуск потока поиска и инициализации модуля
//---------------------------------------------------------------------------
void TMainForm::StartInitModuleThread(void)
{
	if(!IsInitDevicesThreadRunning && IsInitDevicesThreadDone)
	{
		IsInitDevicesThreadDone = false;
		PostMessage(Handle, WM_START_INIT_MODULE_THREAD, 0x0, 0x0);
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnStartInitModuleThread(TMessage& Message)
{
	// пробуем запустить поток обнаружения калибратора TE-01
	IsInitDevicesThreadRunning = false;
	InitDevicesThread = new TInitDevicesThread(false);
	if(!InitDevicesThread) { Application->MessageBox("Не могу открыть поток 'InitDevicesThread'!","Ошибка OnStartInitDevicesThreadThread()!", MB_OK); return; }
	InitDevicesThread->OnTerminate = InitDevicesThreadDone;
	IsInitDevicesThreadDone = true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::InitDevicesThreadDone(TObject * /*Sender*/)
{
	IsInitDevicesThreadRunning = false;
}

//---------------------------------------------------------------------------
// если нужно - ждём завершения потока обнаружения модуля
//---------------------------------------------------------------------------
BOOL TMainForm::WaitFortInitDevicesThreadDone(void)
{
	while(IsInitDevicesThreadRunning)
	{
		if(Application->Terminated) return FALSE;
		Application->ProcessMessages();
		Sleep(50);
	};
	return TRUE;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	// это событие не генерится при выполнении Application->Terminate();
	// взведём флажок завершения приложения
	IsAppTerminated = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
	// взведём флажок завершения приложения
	IsAppTerminated = true;

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// убъём фрейм
	if(E310GeneratorFrame) { delete E310GeneratorFrame; E310GeneratorFrame = NULL; }

	// предварительно если нужно - сохраним текущие настройки
	if(AutoSaveSettingsLMDCheckBox->Checked) SaveSettingsButtonClick(Sender);
	// зачистим файл настроек
	if(IniFile) { delete IniFile; IniFile = NULL; }
	// полностью освободим указатель на модуль
	if(pModule) { pModule->ReleaseLInstance(); pModule = NULL; }

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
// инициализация программы и железа
//---------------------------------------------------------------------------
bool TMainForm::InitApplication(TIniFile *IniFile)
{
	E310GeneratorFrame->InitGeneratorFramePars(pModule, &GeneratorPars);

	// реентерабельность...
	IsReenteringInProgress = true;

	// что там у нас с автосохранением настроек
	AutoSaveSettingsLMDCheckBox->Checked = IniFile ? IniFile->ReadInteger("Приложение. Общие", "Сохранение настроек", false) : false;

	// всё хорошо
	return true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::AutoSaveSettingsLMDCheckBoxChange(TObject *Sender)
{
	if(AutoSaveSettingsLMDCheckBox->Checked) SaveSettingsButton->Enabled = false;
	else SaveSettingsButton->Enabled = true;
}

//---------------------------------------------------------------------------
// создание файла штатных настроек
//---------------------------------------------------------------------------
bool TMainForm::CreateDefaultIniFile(AnsiString IniFileName)
{
	AnsiString Str;

	// на всякий случай зачистим файл настроек
	if(!IniFile) { delete IniFile; IniFile = NULL; };
	// проверим название файла
	if(IniFileName == "") { Application->MessageBox("Неправильное название файла штатных настроек!", "ОШИБКА CreateDefaultIniFile()!", MB_OK + MB_ICONINFORMATION); return false; }
	// теперь спокойно попробуем открыть файл настроек
	IniFile = new TIniFile(IniFileName);
	// проверим
	if(!IniFile) { Str = "Не могу создать файл штатных настроек '" + IniFileName + "'!"; Application->MessageBox(Str.c_str(), "ОШИБКА CreateDefaultIniFile()!", MB_OK + MB_ICONINFORMATION); return false; }

	// название приложения
	IniFile->WriteString("Файлы", "Головная программа", ParamStr(0x0));

	// автосохранение настроек
	IniFile->WriteBool("Приложение. Общие", "Сохранение настроек", false);
	// координаты основной панели программы
	IniFile->WriteInteger("Приложение. Общие", "X координата основного панели", 20);
	IniFile->WriteInteger("Приложение. Общие", "Y координата основного панели", 20);

	// параметры генератора
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Начальная частота в кГц", 10.0);
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Частота приращений в кГц", 50.0);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Кол-во приращений", 10);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип базового интервала приращения", CLOCK_PERIOD_INCREMENT_INTERVAL_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Индекс умножителя для базового интервала приращения", INCREMENT_INTERVAL_MULTIPLIERS_001_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Кол-во базовых интервалов в интервале приращения", 100);
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Частота тактирующего сигнала", 50000.0);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Источник тактирующего сигнала", INTERNAL_MASTER_CLOCK_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип циклического автосканирования", NO_CYCLIC_AUTOSCAN_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип инкрементации частоты", CTRL_LINE_INCREMENT_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип линии 'CTRL'", INTERNAL_CTRL_LINE_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип линии 'INTERRUPT'", INTERNAL_INTERRUPT_LINE_E310);
	IniFile->WriteBool("Модуль E-310",		"Генератор. Разрешение цифрового выхода генератора", false);
	IniFile->WriteBool("Модуль E-310",		"Генератор. Разрешение синхросигнала", false);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип формирования синхросигнала", SYNCOUT_AT_END_OF_SCAN_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип аналогового сигнала", SINUSOIDAL_ANALOG_OUTPUT_E310);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Индекс усиления выходного тракта", ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310);
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Величина внутреннего смещение на выходе 10 Ом", 0.0);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип смещения на выходе 10 Ом", INTERNAL_OUTPUT_10_OHM_OFFSET_E310);

	// освободим используемый файл настроек
	if(IniFile) { delete IniFile; IniFile = NULL; }
	// всё хорошо
	return true;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SaveSettingsButtonClick(TObject *Sender)
{
	AnsiString IniFileName = "E310Config.ini";

	// узнаем текущие параметры работы генератора
	E310GeneratorFrame->GetGeneratorFramePars(&MainForm->GeneratorPars);

	// формируем полное название файла
	IniFileName = ExtractFileDir(ParamStr(0)) + "\\" + IniFileName;
	// освободим используемый файл настроек
	if(IniFile) { delete IniFile; IniFile = NULL; }

	// откроем файл настроек
	IniFile = new TIniFile(IniFileName);
	IniFile->WriteString("Файлы", "Головная программа", ParamStr(0));

	// что там у нас с автосохранением настроек
	IniFile->WriteBool("Приложение. Общие", "Сохранение настроек", AutoSaveSettingsLMDCheckBox->Checked);
	IniFile->WriteInteger("Приложение. Общие", "X координата основного панели", MainForm->Left);
	IniFile->WriteInteger("Приложение. Общие", "Y координата основного панели", MainForm->Top);

	// параметры генератора
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Начальная частота в кГц", GeneratorPars.StartFrequency);
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Частота приращений в кГц", GeneratorPars.FrequencyIncrements);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Кол-во приращений", GeneratorPars.NumberOfIncrements);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип базового интервала приращения", GeneratorPars.IncrementIntervalPars.BaseIntervalType);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Индекс умножителя для базового интервала приращения", GeneratorPars.IncrementIntervalPars.MultiplierIndex);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Кол-во базовых интервалов в интервале приращения", GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber);
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Частота тактирующего сигнала", GeneratorPars.MasterClock);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Источник тактирующего сигнала", GeneratorPars.MasterClockSource);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип циклического автосканирования", GeneratorPars.CyclicAutoScanType);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип инкрементации частоты", GeneratorPars.IncrementType);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип линии 'CTRL'", GeneratorPars.CtrlLineType);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип линии 'INTERRUPT'", GeneratorPars.InterrupLineType);
	IniFile->WriteBool("Модуль E-310",		"Генератор. Разрешение цифрового выхода генератора", GeneratorPars.SquareWaveOutputEna);
	IniFile->WriteBool("Модуль E-310",		"Генератор. Разрешение синхросигнала", GeneratorPars.SynchroOutEna);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип формирования синхросигнала", GeneratorPars.SynchroOutType);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип аналогового сигнала", GeneratorPars.AnalogOutputsPars.SignalType);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Индекс усиления выходного тракта", GeneratorPars.AnalogOutputsPars.GainIndex);
	IniFile->WriteFloat("Модуль E-310",		"Генератор. Величина внутреннего смещение на выходе 10 Ом", GeneratorPars.AnalogOutputsPars.Output10OhmOffset);
	IniFile->WriteInteger("Модуль E-310",	"Генератор. Тип смещения на выходе 10 Ом", GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FindModuleLMDButtonClick(TObject *Sender)
{
	// запустим поток по обнаружению модуля E-310
	StartInitModuleThread();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::ApplicationExitClick(TObject *Sender)
{
	// выходим из приложения
	Application->Terminate();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SaveDefaultSettingsClick(TObject *Sender)
{
	AnsiString IniFileName = "E310Config.ini";

	// формируем полное название файла
	IniFileName = ExtractFileDir(ParamStr(0)) + "\\" + IniFileName;
	// сохраним настройки работы программы по умолчанию
	CreateDefaultIniFile(IniFileName);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SaveCurrentSettingsClick(TObject *Sender)
{
	SaveSettingsButtonClick(SaveSettingsButton);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::AboutProgramClick(TObject *Sender)
{
	// покажем панель About
	AboutProgramPanel->ShowModal();
}
//---------------------------------------------------------------------------


