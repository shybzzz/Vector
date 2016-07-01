//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "InitDevicesThread.h"
#include "GeneratorFrame.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateC	aption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Unit1::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

// указатель на интерфейс модуля E-310
extern ILE310 *pModule;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TInitDevicesThread::TInitDevicesThread(bool CreateSuspended) : TThread(CreateSuspended)
{
	FreeOnTerminate = true;		// не требуется явного уничтожения потока
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::Execute()
{
	// инициализация текущего потока
	Synchronize(InitThread); Sleep(500);
	// были ошибки при инициализация потока?
	if(ThreadError) { Synchronize(FinishThread); return; }
	//---- Place thread code here ----

	try
	{
		// инициализвция основной панели программы
		Synchronize(InitMainForm); Sleep(100);
		// если нужно - попробуем найти модуль E-310
		Synchronize(AccessToModule); Sleep(100);
		// теперь можно поробовать проинициализировать модуль E-310
		if(!ThreadError) { Synchronize(InitModule); Sleep(100); }
		Synchronize(ShowModule); Sleep(100);
		// инициализвция всех параметров работы программы
		Synchronize(InitApplication);
	}
	catch(...)
	{
		// handler for any C++ exception
		Mes = "Опаньки!!! Поймали неустранимое исключение!!!\nСрочно сообщите разработчику!"; ShowErrorMessageBox();
	}

	// завершения потока
	Synchronize(FinishThread);
}

//---------------------------------------------------------------------------
// инициализация текущего потока
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitThread(void)
{
	// мы в потоке
	MainForm->IsInitDevicesThreadRunning = true;
	// управляющие элементы главной панели программы
	MainFormControlElements(false);
	// сбросим флажок наличия ошибки выполнения потока
	ThreadError = false;
	// режим обнаружения устройств
	InitDevicesMode = MainForm->InitDevicesMode;
	// сделаем доступным главную страницу
	MainForm->Enabled = false;
	// временно исключим Popup Menu
	MainForm->PopupMenu->AutoPopup = false;
	// прочистим очередь сообщений
	Application->ProcessMessages();

	// если нужно - инициализируем управляющие элементы для работы с модулем E-310
	InitModuleControlElements();

	// если нужно - откроем файл настроек
	if(MainForm->IsProgramLaunching) { OpenIniFile(); GetIniData(); }
	else E310GeneratorFrame->GetGeneratorFramePars(&MainForm->GeneratorPars);
	// флажок автосохранения настроек
	IsAutoSaveSettings = IniFile ? IniFile->ReadBool("Приложение. Общие", "Сохранение настроек", false) : false;
}

//---------------------------------------------------------------------------
// управляющие элементы главной панели программы
//---------------------------------------------------------------------------
void TInitDevicesThread::MainFormControlElements(bool EnabledFlag)
{
	// управление текущим Popup меню
	MainForm->PopupMenu->AutoPopup = EnabledFlag ? true : false;
	// управление видом курсора: часики или стрелка
	if(!EnabledFlag) { SaveCursor = Screen->Cursor; Screen->Cursor = crHourGlass; }
	else Screen->Cursor = SaveCursor;

	// вход в реентерабельность...
	MainForm->IsReenteringInProgress = true;

	ControlElements((TWinControl *)MainForm->LoadingTestsLMDGroupBox, EnabledFlag);
	ControlElements((TWinControl *)MainForm->SaveIniSettingLMDGroupBox, EnabledFlag ? IsModuleLoaded : false);
	if(!EnabledFlag)
		ControlElements((TWinControl *)MainForm->GeneratorParsLMDGroupBox, EnabledFlag ? IsModuleLoaded : false);

	// конец реентерабельности...
	MainForm->IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TInitDevicesThread::ControlElements(TWinControl *WinControlElement, bool EnabledFlag)
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
// инициализация управляющих элементов для работы с модулем E-310
//---------------------------------------------------------------------------
void TInitDevicesThread::InitModuleControlElements(void)
{
	// сбросим статус загрузки модуля E-310
	IsModuleLoaded = MainForm->IsModuleLoaded = false;
	// сбросим скорость работы модуля E-310
	MainForm->UsbSpeed = UsbSpeed = INVALID_USB_SPEED_LUSBAPI;
	// режим поиска модуля E-310: сканирование всех виртуальных слотов или только выбранный
	ModuleSearchMode = MainForm->VirtualSlot;
	// виртуальнеый слот
	VirtualSlot = MainForm->VirtualSlot;
	// сделаем недоступыми все возможные отображаемые управляющие элементы модуля E-310
	EnableModuleControlElements(ALL_MODULE_ELEMENTS_ENABLED);
}

//---------------------------------------------------------------------------
// инициализация координат основной панели программы
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitMainForm(void)
{
	int Temp;

	// координаты основной панели программы
	if(IniFile)
	{
		Temp = IniFile->ReadInteger("Приложение. Общие", "X координата основного панели", 20);
		if(Temp != -1) MainForm->Left = Temp;
		Temp = IniFile->ReadInteger("Приложение. Общие", "Y координата основного панели", 20);
		if(Temp != -1) MainForm->Top = Temp;
	}
}

//---------------------------------------------------------------------------
// попытка обнаружить модуль E-310
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::AccessToModule(void)
{
	WORD i, LocVirtualSlot;
	DWORD DllVersion;

	// сбросим флажок наличия ошибки выполнения потока
	ThreadError = false;

	// если уже есть интерфейс, то просто закроем текущий модуль E-310
	if(pModule) pModule->CloseLDevice();
	// иначе попробуем получить интерфейс модуля E-310
	else
	{
		DllVersion = GetDllVersion();
		if(DllVersion != CURRENT_VERSION_LUSBAPI)
		{
			Mes = "Неправильная версия библиотеки Lusbapi.dll!\n";
			Mes += "Текущая: " + IntToStr(DllVersion >> 0x10) + "." + IntToStr(DllVersion & 0xFFFF);
			Mes += " Требуется: " + IntToStr(VERSION_MAJOR_LUSBAPI) + "." + IntToStr(VERSION_MINOR_LUSBAPI);
			ShowErrorMessageBox(); return;
		}
		// попробуем получить указатель на интерфейс модуля E-310
		pModule = static_cast<ILE310 *>(CreateLInstance("e310"));
		if(!pModule) { Mes = "Не могу получить интерфейс на модуль E-310!"; ShowErrorMessageBox(); return; }
	}

	// попробуем обнаружить модуль E-310
	for(i = 0x0, VirtualSlot = 0xFFFF; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++)
	{
		if(!pModule->OpenLDevice(i)) continue;
		else break;
	}
	// попробуем запомнить номер виртуального слота с модулем
	if(i != MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) VirtualSlot = i;
	// сбросим флажок наличия ошибки выполнения потока
	else ThreadError = true;
}

//---------------------------------------------------------------------------
// попытка проинициализировать модуль E-310
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitModule(void)
{
	// прочитаем название модуля в обнаруженном виртуальном слоте
	if(!pModule->GetModuleName(ModuleName)) { Mes = "Не могу выполнить функцию GetModuleName()!"; ShowErrorMessageBox(); return; }
	// проверим, что это 'E-310'
	if(strcmp(ModuleName, "E-310")) { Mes = "Обнаруженное устройство не является модулем 'E-310'!"; ShowErrorMessageBox(); return; }
	// узнаем текущую скорость работы шины USB20
	if(!pModule->GetUsbSpeed(&UsbSpeed)) { Mes = "Не могу выполнить функцию GetUsbSpeed()!"; ShowErrorMessageBox(); return; }
	if(UsbSpeed >= INVALID_USB_SPEED_LUSBAPI) { Mes = "Считалась неправильная скорость работы шины USB!"; ShowErrorMessageBox(); return; }
	else MainForm->UsbSpeed = UsbSpeed;
	// получим информацию из ППЗУ модуля
	if(!pModule->GET_MODULE_DESCRIPTION(&MainForm->ModuleDescription)) { Mes = "Не могу выполнить функцию GET_MODULE_DESCRIPTION()!"; ShowErrorMessageBox(); return; }
	// попробуем проинициализировать параметры работы АЦП
	if(!InitModulePars()) return;

	// работоспособный модуль E-310 успешно найден
	IsModuleLoaded		= MainForm->IsModuleLoaded		= true;
//	ModuleRevision		= MainForm->ModuleRevision		= MainForm->ModuleDescription.Module.Revision;
//	ModuleModification= MainForm->ModuleModification= MainForm->ModuleDescription.Module.Modification;

	// виртуальнеый слот
	MainForm->VirtualSlot = VirtualSlot;
}

//---------------------------------------------------------------------------
// попробуем проинициализировать параметры работы АЦП
//---------------------------------------------------------------------------
bool TInitDevicesThread::InitModulePars(void)
{
//	WORD i, j;

/*	if(MainForm->IsProgramLaunching) { }*/

	// всё хорошо
	return true;
}

//---------------------------------------------------------------------------
// отобразим всё, что надыбали про модуль E-310
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::ShowModule(void)
{
	WORD i;

	// сделаем доступыми все возможные отображаемые управляющие элементы модуля E-310
	EnableModuleControlElements(ALL_MODULE_ELEMENTS_DISABLED);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TInitDevicesThread::EnableModuleControlElements(bool EnabledFlag)
{
	AnsiString Str;

	Str = "Модуль  " + (AnsiString)((char *)ModuleName);
	Str = IsModuleLoaded ? Str : (AnsiString)"Модуль ??????";
	MainForm->LoadingTestsLMDGroupBox->Caption = EnabledFlag ? Str : (AnsiString)"Модуль ??????";
	// лампочка статуса обнаружения и загнрузки модуля
	MainForm->ModuleDetectionLMDShapeControl->Brush->Color = EnabledFlag ? (IsModuleLoaded ? clLime : clRed) : clBtnShadow;
	// надпись около лампочки обнаружения модуля
	MainForm->ModuleDetectionLMDStaticText->Enabled = EnabledFlag ? true : false;
	// statictext c номером виртуального слота
	MainForm->VirtualSlotLMDStaticText->Enabled = EnabledFlag ? true : false;
	Str = IsModuleLoaded ? (AnsiString)(VirtualSlot) : (AnsiString)"";
	MainForm->VirtualSlotLMDStaticText->Caption = EnabledFlag ? Str.c_str() : "";
	// надпись номера виртуального слота
	MainForm->VirtualSlotLabelLMDStaticText->Enabled = EnabledFlag ? true : false;
}

//---------------------------------------------------------------------------
// параметры приложения
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitApplication(void)
{
	if(IsModuleLoaded)
	{
		ControlElements((TWinControl *)MainForm->GeneratorParsLMDGroupBox, IsModuleLoaded);
		if(!MainForm->InitApplication(IniFile))  { Mes = "Не могу проинициализировать параметры работы приложения!"; ShowErrorMessageBox(); return; }
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::FinishThread(void)
{
	// сделаем доступным главную страницу
	MainForm->Enabled = true;
	// кнопки загрузки модуля
	MainForm->FindModuleLMDButton->Enabled = true;

	// управляющие элементы главной панели программы
	MainFormControlElements(true);
	//
/*	if(MainForm->INT_OR_EXT_INCR_LMDCheckBox->Checked) ControlElements(MainForm->IncrementIntervalLMDGroupBox, false);
	else ControlElements(MainForm->IncrementIntervalLMDGroupBox, IsModuleLoaded);*/

	// кнопка сохранения настроек
	if(MainForm->AutoSaveSettingsLMDCheckBox->Checked) MainForm->SaveSettingsButton->Enabled = false;
	else MainForm->SaveSettingsButton->Enabled = IsModuleLoaded;

	// панель автозапуска сканирования DDS
/*	if(MainForm->GeneratorPars.DdsPars.ControlReg.BitFields.SYNCOUTEN	&&
		MainForm->GeneratorPars.DdsPars.ControlReg.BitFields.SYNCSEL		&&
		(!MainForm->GeneratorPars.ControlReg.BitFields.DDS_CTRL_SOURCE))
	{
		MainForm->AutoScanLMDRadioGroup->Enabled = true;
	}
	else
	{
		MainForm->AutoScanLMDRadioGroup->Enabled = false;
	}*/

	// освободим используемый файл настроек
	if(IniFile) { delete IniFile; IniFile = NULL; }

	// восстановим вид курсора
	Screen->Cursor = SaveCursor;
	// восстановим Popup Menu
	MainForm->PopupMenu->AutoPopup = true;
	// сбросим признак запуска приложения
	MainForm->IsProgramLaunching = false;

	// если нужно - установим фокус на кнопке 'Обнаружить TE-01'
//	if(MainForm->FindTe01LMDButton->CanFocus()) MainForm->FindTe01LMDButton->SetFocus();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool TInitDevicesThread::OpenIniFile(void)
{
	AnsiString IniFileName = "E310Config.ini";

	// сбросим указатель на файл настроек
	IniFile = NULL;
	// формируем полное название файла
	IniFileName = ExtractFileDir(ParamStr(0)) + "\\" + IniFileName;
	// а он сущуствует?
	if(!FileExists(IniFileName))
	{
		// если нет, то создаем его
		int iIniFileHandle;
		if((iIniFileHandle = FileCreate(IniFileName)) == -1)
		{
			Mes = "Не могу создать файл штатных настроек '" + IniFileName + "'!";
			ShowErrorMessageBox();
			return false;
		}
		FileClose(iIniFileHandle);
		// и инициализируем
		if(!MainForm->CreateDefaultIniFile(IniFileName)) return false;
	}
	IniFile = new TIniFile(IniFileName);

	return true;
}

//---------------------------------------------------------------------------
// берем инициализационные данные из ini файла
//---------------------------------------------------------------------------
void TInitDevicesThread::GetIniData(void)
{
	MainForm->GeneratorPars.StartFrequency = IniFile ? IniFile->ReadFloat("Модуль E-310", "Генератор. Начальная частота в кГц", 10.0) : 10.0;
	MainForm->GeneratorPars.FrequencyIncrements = IniFile ? IniFile->ReadFloat("Модуль E-310", "Генератор. Частота приращений в кГц", 50.0) : 50.0;
	MainForm->GeneratorPars.NumberOfIncrements = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Кол-во приращений", 10) : 10;
	MainForm->GeneratorPars.IncrementIntervalPars.BaseIntervalType = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип базового интервала приращения", CLOCK_PERIOD_INCREMENT_INTERVAL_E310) : CLOCK_PERIOD_INCREMENT_INTERVAL_E310;
	MainForm->GeneratorPars.IncrementIntervalPars.MultiplierIndex = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Индекс умножителя для базового интервала приращения", INCREMENT_INTERVAL_MULTIPLIERS_001_E310) : INCREMENT_INTERVAL_MULTIPLIERS_001_E310;
	MainForm->GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Кол-во базовых интервалов в интервале приращения", 100) : 100;
	MainForm->GeneratorPars.MasterClock = IniFile ? IniFile->ReadFloat("Модуль E-310", "Генератор. Частота тактирующего сигнала", 50000.0) : 50000.0;
	MainForm->GeneratorPars.MasterClockSource = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Источник тактирующего сигнала", INTERNAL_MASTER_CLOCK_E310) : INTERNAL_MASTER_CLOCK_E310;
	MainForm->GeneratorPars.CyclicAutoScanType = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип циклического автосканирования", NO_CYCLIC_AUTOSCAN_E310) : NO_CYCLIC_AUTOSCAN_E310;
	MainForm->GeneratorPars.IncrementType = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип инкрементации частоты", AUTO_INCREMENT_E310) : AUTO_INCREMENT_E310;
	MainForm->GeneratorPars.CtrlLineType = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип линии 'CTRL'", INTERNAL_CTRL_LINE_E310) : INTERNAL_CTRL_LINE_E310;
	MainForm->GeneratorPars.InterrupLineType = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип линии 'INTERRUPT'", INTERNAL_INTERRUPT_LINE_E310) : INTERNAL_INTERRUPT_LINE_E310;
	MainForm->GeneratorPars.SquareWaveOutputEna = IniFile ? IniFile->ReadBool("Модуль E-310", "Генератор. Разрешение цифрового выхода генератора", false) : false;
	MainForm->GeneratorPars.SynchroOutEna = IniFile ? IniFile->ReadBool("Модуль E-310", "Генератор. Разрешение синхросигнала", false) : false;
	MainForm->GeneratorPars.SynchroOutType = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип формирования синхросигнала", SYNCOUT_AT_END_OF_SCAN_E310) : SYNCOUT_AT_END_OF_SCAN_E310;
	MainForm->GeneratorPars.AnalogOutputsPars.SignalType = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип аналогового сигнала", SINUSOIDAL_ANALOG_OUTPUT_E310) : SINUSOIDAL_ANALOG_OUTPUT_E310;
	MainForm->GeneratorPars.AnalogOutputsPars.GainIndex = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Индекс усиления выходного тракта", ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310) : ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310;
	MainForm->GeneratorPars.AnalogOutputsPars.Output10OhmOffset = IniFile ? IniFile->ReadFloat("Модуль E-310", "Генератор. Величина внутреннего смещение на выходе 10 Ом", 0.0) : 0.0;
	MainForm->GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource = IniFile ? IniFile->ReadInteger("Модуль E-310", "Генератор. Тип смещения на выходе 10 Ом", INTERNAL_OUTPUT_10_OHM_OFFSET_E310) : INTERNAL_OUTPUT_10_OHM_OFFSET_E310;
}

//---------------------------------------------------------------------------
// отображение сообщение с ошибкой
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::ShowErrorMessageBox(void)
{
	// проверим была ли ранее ошибка
	if(ThreadError) return;
	// выведем на экран сообщение с ошибкой
	Application->MessageBox(Mes.c_str(),"Сообщение TInitDevicesThread::Execute()!", MB_OK + MB_ICONINFORMATION);
	// установим флаг наличия ошибки
	ThreadError = true;
}

