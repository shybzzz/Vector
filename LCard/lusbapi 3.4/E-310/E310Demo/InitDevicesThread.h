//---------------------------------------------------------------------------
#ifndef InitDevicesThreadH
#define InitDevicesThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "MainForm.h"
#include <inifiles.hpp>

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class TInitDevicesThread : public TThread
{
private:
	void __fastcall InitThread(void);
	void __fastcall FinishThread(void);
	void __fastcall ShowErrorMessageBox(void);

protected:
	void __fastcall Execute();

	// ������ � ������ ��������
	bool OpenIniFile(void);
	void GetIniData(void);
//	bool CreateDefaultIniFile	(AnsiString IniFileName);
//	bool CreateModuleIniFile(TIniFile *IniFile);
//	bool CreateApplicationIniFile(TIniFile *IniFile);

	// ����������� �������� ������� ������ ���������
//	void EnableMainFormControlElements(bool EnabledFlag);
//	void EnableMainFormIniControlElements(bool EnabledFlag);

	void __fastcall InitMainForm(void);

	// ����������� �������� ������� ������ ���������
	void MainFormControlElements(bool EnabledFlag);
	void ControlElements(TWinControl *WinControlElement, bool EnabledFlag);

	// ������ � ������� E20-10
	void InitModuleControlElements(void);
	void EnableModuleControlElements(bool EnabledFlag);
	void __fastcall AccessToModule(void);
	void __fastcall InitModule(void);
	void __fastcall ShowModule(void);
	bool InitModulePars(void);

	// ������ � ������������ TE-01
	void InitTe01ControlElements(void);
	void EnableTe01ControlElements(WORD SubModuleIndex, bool EnabledFlag);
	void __fastcall AccessToTe01(void);
	void __fastcall InitTe01(void);
	void __fastcall ShowTe01(void);

	// ��������� ����������
	void __fastcall InitApplication(void);

	// ������� ������ ���������� ������
	bool ThreadError;
	// ������� �������������� ��������
	bool IsAutoSaveSettings;
	// ���� ��������
	TIniFile *IniFile;

	// ������ �������� ������
	bool IsModuleLoaded;
	// ����������� ���� � ����� ������ ������ E20-10
	WORD VirtualSlot, ModuleSearchMode;
	// �������� ������ ���� USB
	BYTE UsbSpeed;
	// �������� ������
	BYTE ModuleName[0x10];
	// ������� � ���������� ������
	BYTE ModuleRevision, ModuleModification;
	// ������� ��� ������ ������������ �������
	AnsiString ModuleListBoxStrings[MAX_VIRTUAL_SLOTS_QUANTITY];

	// ����� ����������� ���������
	WORD InitDevicesMode;
	// COM-���� ��� ����������� TE-01
//	WORD Te01ComPort;
	// ������ ���������� ����������� TE-01
//	bool Te01SubmodulesStatusArray[TE01_SUBMODULES_QUANTITY];
	// ����������� �������� � ������� ������� ���������� ����������� TE-01
//	TStaticText *Te01SubmodulesLabelStaticTextArray[TE01_SUBMODULES_QUANTITY];
	// ����������� ������� ���������� ����������� TE-01 � ������� ������
//	TStaticText *Te01SubmodulesStatusStaticTextArray[TE01_SUBMODULES_QUANTITY];
	// ����������� ������� ���������� ����������� TE-01 � ������� �������������
//	ThhALed *Te01SubmodulesLedArray[TE01_SUBMODULES_QUANTITY];

	//
	TCursor SaveCursor;
	// ������� �������������� ��������
//	bool IsAutoSaveSettings;
	// ������ � ������� ���������� ������
	AnsiString Mes;

public:
	__fastcall TInitDevicesThread(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
