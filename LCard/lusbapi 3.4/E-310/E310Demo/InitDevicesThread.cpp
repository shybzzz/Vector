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

// ��������� �� ��������� ������ E-310
extern ILE310 *pModule;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TInitDevicesThread::TInitDevicesThread(bool CreateSuspended) : TThread(CreateSuspended)
{
	FreeOnTerminate = true;		// �� ��������� ������ ����������� ������
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::Execute()
{
	// ������������� �������� ������
	Synchronize(InitThread); Sleep(500);
	// ���� ������ ��� ������������� ������?
	if(ThreadError) { Synchronize(FinishThread); return; }
	//---- Place thread code here ----

	try
	{
		// ������������� �������� ������ ���������
		Synchronize(InitMainForm); Sleep(100);
		// ���� ����� - ��������� ����� ������ E-310
		Synchronize(AccessToModule); Sleep(100);
		// ������ ����� ���������� ������������������� ������ E-310
		if(!ThreadError) { Synchronize(InitModule); Sleep(100); }
		Synchronize(ShowModule); Sleep(100);
		// ������������� ���� ���������� ������ ���������
		Synchronize(InitApplication);
	}
	catch(...)
	{
		// handler for any C++ exception
		Mes = "�������!!! ������� ������������ ����������!!!\n������ �������� ������������!"; ShowErrorMessageBox();
	}

	// ���������� ������
	Synchronize(FinishThread);
}

//---------------------------------------------------------------------------
// ������������� �������� ������
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitThread(void)
{
	// �� � ������
	MainForm->IsInitDevicesThreadRunning = true;
	// ����������� �������� ������� ������ ���������
	MainFormControlElements(false);
	// ������� ������ ������� ������ ���������� ������
	ThreadError = false;
	// ����� ����������� ���������
	InitDevicesMode = MainForm->InitDevicesMode;
	// ������� ��������� ������� ��������
	MainForm->Enabled = false;
	// �������� �������� Popup Menu
	MainForm->PopupMenu->AutoPopup = false;
	// ��������� ������� ���������
	Application->ProcessMessages();

	// ���� ����� - �������������� ����������� �������� ��� ������ � ������� E-310
	InitModuleControlElements();

	// ���� ����� - ������� ���� ��������
	if(MainForm->IsProgramLaunching) { OpenIniFile(); GetIniData(); }
	else E310GeneratorFrame->GetGeneratorFramePars(&MainForm->GeneratorPars);
	// ������ �������������� ��������
	IsAutoSaveSettings = IniFile ? IniFile->ReadBool("����������. �����", "���������� ��������", false) : false;
}

//---------------------------------------------------------------------------
// ����������� �������� ������� ������ ���������
//---------------------------------------------------------------------------
void TInitDevicesThread::MainFormControlElements(bool EnabledFlag)
{
	// ���������� ������� Popup ����
	MainForm->PopupMenu->AutoPopup = EnabledFlag ? true : false;
	// ���������� ����� �������: ������ ��� �������
	if(!EnabledFlag) { SaveCursor = Screen->Cursor; Screen->Cursor = crHourGlass; }
	else Screen->Cursor = SaveCursor;

	// ���� � �����������������...
	MainForm->IsReenteringInProgress = true;

	ControlElements((TWinControl *)MainForm->LoadingTestsLMDGroupBox, EnabledFlag);
	ControlElements((TWinControl *)MainForm->SaveIniSettingLMDGroupBox, EnabledFlag ? IsModuleLoaded : false);
	if(!EnabledFlag)
		ControlElements((TWinControl *)MainForm->GeneratorParsLMDGroupBox, EnabledFlag ? IsModuleLoaded : false);

	// ����� �����������������...
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
// ������������� ����������� ��������� ��� ������ � ������� E-310
//---------------------------------------------------------------------------
void TInitDevicesThread::InitModuleControlElements(void)
{
	// ������� ������ �������� ������ E-310
	IsModuleLoaded = MainForm->IsModuleLoaded = false;
	// ������� �������� ������ ������ E-310
	MainForm->UsbSpeed = UsbSpeed = INVALID_USB_SPEED_LUSBAPI;
	// ����� ������ ������ E-310: ������������ ���� ����������� ������ ��� ������ ���������
	ModuleSearchMode = MainForm->VirtualSlot;
	// ������������ ����
	VirtualSlot = MainForm->VirtualSlot;
	// ������� ����������� ��� ��������� ������������ ����������� �������� ������ E-310
	EnableModuleControlElements(ALL_MODULE_ELEMENTS_ENABLED);
}

//---------------------------------------------------------------------------
// ������������� ��������� �������� ������ ���������
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitMainForm(void)
{
	int Temp;

	// ���������� �������� ������ ���������
	if(IniFile)
	{
		Temp = IniFile->ReadInteger("����������. �����", "X ���������� ��������� ������", 20);
		if(Temp != -1) MainForm->Left = Temp;
		Temp = IniFile->ReadInteger("����������. �����", "Y ���������� ��������� ������", 20);
		if(Temp != -1) MainForm->Top = Temp;
	}
}

//---------------------------------------------------------------------------
// ������� ���������� ������ E-310
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::AccessToModule(void)
{
	WORD i, LocVirtualSlot;
	DWORD DllVersion;

	// ������� ������ ������� ������ ���������� ������
	ThreadError = false;

	// ���� ��� ���� ���������, �� ������ ������� ������� ������ E-310
	if(pModule) pModule->CloseLDevice();
	// ����� ��������� �������� ��������� ������ E-310
	else
	{
		DllVersion = GetDllVersion();
		if(DllVersion != CURRENT_VERSION_LUSBAPI)
		{
			Mes = "������������ ������ ���������� Lusbapi.dll!\n";
			Mes += "�������: " + IntToStr(DllVersion >> 0x10) + "." + IntToStr(DllVersion & 0xFFFF);
			Mes += " ���������: " + IntToStr(VERSION_MAJOR_LUSBAPI) + "." + IntToStr(VERSION_MINOR_LUSBAPI);
			ShowErrorMessageBox(); return;
		}
		// ��������� �������� ��������� �� ��������� ������ E-310
		pModule = static_cast<ILE310 *>(CreateLInstance("e310"));
		if(!pModule) { Mes = "�� ���� �������� ��������� �� ������ E-310!"; ShowErrorMessageBox(); return; }
	}

	// ��������� ���������� ������ E-310
	for(i = 0x0, VirtualSlot = 0xFFFF; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++)
	{
		if(!pModule->OpenLDevice(i)) continue;
		else break;
	}
	// ��������� ��������� ����� ������������ ����� � �������
	if(i != MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) VirtualSlot = i;
	// ������� ������ ������� ������ ���������� ������
	else ThreadError = true;
}

//---------------------------------------------------------------------------
// ������� ������������������� ������ E-310
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitModule(void)
{
	// ��������� �������� ������ � ������������ ����������� �����
	if(!pModule->GetModuleName(ModuleName)) { Mes = "�� ���� ��������� ������� GetModuleName()!"; ShowErrorMessageBox(); return; }
	// ��������, ��� ��� 'E-310'
	if(strcmp(ModuleName, "E-310")) { Mes = "������������ ���������� �� �������� ������� 'E-310'!"; ShowErrorMessageBox(); return; }
	// ������ ������� �������� ������ ���� USB20
	if(!pModule->GetUsbSpeed(&UsbSpeed)) { Mes = "�� ���� ��������� ������� GetUsbSpeed()!"; ShowErrorMessageBox(); return; }
	if(UsbSpeed >= INVALID_USB_SPEED_LUSBAPI) { Mes = "��������� ������������ �������� ������ ���� USB!"; ShowErrorMessageBox(); return; }
	else MainForm->UsbSpeed = UsbSpeed;
	// ������� ���������� �� ���� ������
	if(!pModule->GET_MODULE_DESCRIPTION(&MainForm->ModuleDescription)) { Mes = "�� ���� ��������� ������� GET_MODULE_DESCRIPTION()!"; ShowErrorMessageBox(); return; }
	// ��������� ������������������� ��������� ������ ���
	if(!InitModulePars()) return;

	// ��������������� ������ E-310 ������� ������
	IsModuleLoaded		= MainForm->IsModuleLoaded		= true;
//	ModuleRevision		= MainForm->ModuleRevision		= MainForm->ModuleDescription.Module.Revision;
//	ModuleModification= MainForm->ModuleModification= MainForm->ModuleDescription.Module.Modification;

	// ������������ ����
	MainForm->VirtualSlot = VirtualSlot;
}

//---------------------------------------------------------------------------
// ��������� ������������������� ��������� ������ ���
//---------------------------------------------------------------------------
bool TInitDevicesThread::InitModulePars(void)
{
//	WORD i, j;

/*	if(MainForm->IsProgramLaunching) { }*/

	// �� ������
	return true;
}

//---------------------------------------------------------------------------
// ��������� ��, ��� �������� ��� ������ E-310
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::ShowModule(void)
{
	WORD i;

	// ������� ��������� ��� ��������� ������������ ����������� �������� ������ E-310
	EnableModuleControlElements(ALL_MODULE_ELEMENTS_DISABLED);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TInitDevicesThread::EnableModuleControlElements(bool EnabledFlag)
{
	AnsiString Str;

	Str = "������  " + (AnsiString)((char *)ModuleName);
	Str = IsModuleLoaded ? Str : (AnsiString)"������ ??????";
	MainForm->LoadingTestsLMDGroupBox->Caption = EnabledFlag ? Str : (AnsiString)"������ ??????";
	// �������� ������� ����������� � ��������� ������
	MainForm->ModuleDetectionLMDShapeControl->Brush->Color = EnabledFlag ? (IsModuleLoaded ? clLime : clRed) : clBtnShadow;
	// ������� ����� �������� ����������� ������
	MainForm->ModuleDetectionLMDStaticText->Enabled = EnabledFlag ? true : false;
	// statictext c ������� ������������ �����
	MainForm->VirtualSlotLMDStaticText->Enabled = EnabledFlag ? true : false;
	Str = IsModuleLoaded ? (AnsiString)(VirtualSlot) : (AnsiString)"";
	MainForm->VirtualSlotLMDStaticText->Caption = EnabledFlag ? Str.c_str() : "";
	// ������� ������ ������������ �����
	MainForm->VirtualSlotLabelLMDStaticText->Enabled = EnabledFlag ? true : false;
}

//---------------------------------------------------------------------------
// ��������� ����������
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::InitApplication(void)
{
	if(IsModuleLoaded)
	{
		ControlElements((TWinControl *)MainForm->GeneratorParsLMDGroupBox, IsModuleLoaded);
		if(!MainForm->InitApplication(IniFile))  { Mes = "�� ���� ������������������� ��������� ������ ����������!"; ShowErrorMessageBox(); return; }
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::FinishThread(void)
{
	// ������� ��������� ������� ��������
	MainForm->Enabled = true;
	// ������ �������� ������
	MainForm->FindModuleLMDButton->Enabled = true;

	// ����������� �������� ������� ������ ���������
	MainFormControlElements(true);
	//
/*	if(MainForm->INT_OR_EXT_INCR_LMDCheckBox->Checked) ControlElements(MainForm->IncrementIntervalLMDGroupBox, false);
	else ControlElements(MainForm->IncrementIntervalLMDGroupBox, IsModuleLoaded);*/

	// ������ ���������� ��������
	if(MainForm->AutoSaveSettingsLMDCheckBox->Checked) MainForm->SaveSettingsButton->Enabled = false;
	else MainForm->SaveSettingsButton->Enabled = IsModuleLoaded;

	// ������ ����������� ������������ DDS
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

	// ��������� ������������ ���� ��������
	if(IniFile) { delete IniFile; IniFile = NULL; }

	// ����������� ��� �������
	Screen->Cursor = SaveCursor;
	// ����������� Popup Menu
	MainForm->PopupMenu->AutoPopup = true;
	// ������� ������� ������� ����������
	MainForm->IsProgramLaunching = false;

	// ���� ����� - ��������� ����� �� ������ '���������� TE-01'
//	if(MainForm->FindTe01LMDButton->CanFocus()) MainForm->FindTe01LMDButton->SetFocus();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool TInitDevicesThread::OpenIniFile(void)
{
	AnsiString IniFileName = "E310Config.ini";

	// ������� ��������� �� ���� ��������
	IniFile = NULL;
	// ��������� ������ �������� �����
	IniFileName = ExtractFileDir(ParamStr(0)) + "\\" + IniFileName;
	// � �� ����������?
	if(!FileExists(IniFileName))
	{
		// ���� ���, �� ������� ���
		int iIniFileHandle;
		if((iIniFileHandle = FileCreate(IniFileName)) == -1)
		{
			Mes = "�� ���� ������� ���� ������� �������� '" + IniFileName + "'!";
			ShowErrorMessageBox();
			return false;
		}
		FileClose(iIniFileHandle);
		// � ��������������
		if(!MainForm->CreateDefaultIniFile(IniFileName)) return false;
	}
	IniFile = new TIniFile(IniFileName);

	return true;
}

//---------------------------------------------------------------------------
// ����� ����������������� ������ �� ini �����
//---------------------------------------------------------------------------
void TInitDevicesThread::GetIniData(void)
{
	MainForm->GeneratorPars.StartFrequency = IniFile ? IniFile->ReadFloat("������ E-310", "���������. ��������� ������� � ���", 10.0) : 10.0;
	MainForm->GeneratorPars.FrequencyIncrements = IniFile ? IniFile->ReadFloat("������ E-310", "���������. ������� ���������� � ���", 50.0) : 50.0;
	MainForm->GeneratorPars.NumberOfIncrements = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ���-�� ����������", 10) : 10;
	MainForm->GeneratorPars.IncrementIntervalPars.BaseIntervalType = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� �������� ��������� ����������", CLOCK_PERIOD_INCREMENT_INTERVAL_E310) : CLOCK_PERIOD_INCREMENT_INTERVAL_E310;
	MainForm->GeneratorPars.IncrementIntervalPars.MultiplierIndex = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ������ ���������� ��� �������� ��������� ����������", INCREMENT_INTERVAL_MULTIPLIERS_001_E310) : INCREMENT_INTERVAL_MULTIPLIERS_001_E310;
	MainForm->GeneratorPars.IncrementIntervalPars.BaseIntervalsNumber = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ���-�� ������� ���������� � ��������� ����������", 100) : 100;
	MainForm->GeneratorPars.MasterClock = IniFile ? IniFile->ReadFloat("������ E-310", "���������. ������� ������������ �������", 50000.0) : 50000.0;
	MainForm->GeneratorPars.MasterClockSource = IniFile ? IniFile->ReadInteger("������ E-310", "���������. �������� ������������ �������", INTERNAL_MASTER_CLOCK_E310) : INTERNAL_MASTER_CLOCK_E310;
	MainForm->GeneratorPars.CyclicAutoScanType = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� ������������ ����������������", NO_CYCLIC_AUTOSCAN_E310) : NO_CYCLIC_AUTOSCAN_E310;
	MainForm->GeneratorPars.IncrementType = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� ������������� �������", AUTO_INCREMENT_E310) : AUTO_INCREMENT_E310;
	MainForm->GeneratorPars.CtrlLineType = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� ����� 'CTRL'", INTERNAL_CTRL_LINE_E310) : INTERNAL_CTRL_LINE_E310;
	MainForm->GeneratorPars.InterrupLineType = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� ����� 'INTERRUPT'", INTERNAL_INTERRUPT_LINE_E310) : INTERNAL_INTERRUPT_LINE_E310;
	MainForm->GeneratorPars.SquareWaveOutputEna = IniFile ? IniFile->ReadBool("������ E-310", "���������. ���������� ��������� ������ ����������", false) : false;
	MainForm->GeneratorPars.SynchroOutEna = IniFile ? IniFile->ReadBool("������ E-310", "���������. ���������� �������������", false) : false;
	MainForm->GeneratorPars.SynchroOutType = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� ������������ �������������", SYNCOUT_AT_END_OF_SCAN_E310) : SYNCOUT_AT_END_OF_SCAN_E310;
	MainForm->GeneratorPars.AnalogOutputsPars.SignalType = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� ����������� �������", SINUSOIDAL_ANALOG_OUTPUT_E310) : SINUSOIDAL_ANALOG_OUTPUT_E310;
	MainForm->GeneratorPars.AnalogOutputsPars.GainIndex = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ������ �������� ��������� ������", ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310) : ANALOG_OUTPUT_GAIN_PLUS_10_DB_E310;
	MainForm->GeneratorPars.AnalogOutputsPars.Output10OhmOffset = IniFile ? IniFile->ReadFloat("������ E-310", "���������. �������� ����������� �������� �� ������ 10 ��", 0.0) : 0.0;
	MainForm->GeneratorPars.AnalogOutputsPars.Output10OhmOffsetSource = IniFile ? IniFile->ReadInteger("������ E-310", "���������. ��� �������� �� ������ 10 ��", INTERNAL_OUTPUT_10_OHM_OFFSET_E310) : INTERNAL_OUTPUT_10_OHM_OFFSET_E310;
}

//---------------------------------------------------------------------------
// ����������� ��������� � �������
//---------------------------------------------------------------------------
void __fastcall TInitDevicesThread::ShowErrorMessageBox(void)
{
	// �������� ���� �� ����� ������
	if(ThreadError) return;
	// ������� �� ����� ��������� � �������
	Application->MessageBox(Mes.c_str(),"��������� TInitDevicesThread::Execute()!", MB_OK + MB_ICONINFORMATION);
	// ��������� ���� ������� ������
	ThreadError = true;
}

