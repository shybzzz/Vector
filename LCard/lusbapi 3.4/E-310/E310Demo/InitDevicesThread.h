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

	// работа с файлом настроек
	bool OpenIniFile(void);
	void GetIniData(void);
//	bool CreateDefaultIniFile	(AnsiString IniFileName);
//	bool CreateModuleIniFile(TIniFile *IniFile);
//	bool CreateApplicationIniFile(TIniFile *IniFile);

	// управляющие элементы главной панели программы
//	void EnableMainFormControlElements(bool EnabledFlag);
//	void EnableMainFormIniControlElements(bool EnabledFlag);

	void __fastcall InitMainForm(void);

	// управляющие элементы главной панели программы
	void MainFormControlElements(bool EnabledFlag);
	void ControlElements(TWinControl *WinControlElement, bool EnabledFlag);

	// работа с модулем E20-10
	void InitModuleControlElements(void);
	void EnableModuleControlElements(bool EnabledFlag);
	void __fastcall AccessToModule(void);
	void __fastcall InitModule(void);
	void __fastcall ShowModule(void);
	bool InitModulePars(void);

	// работа с калибратором TE-01
	void InitTe01ControlElements(void);
	void EnableTe01ControlElements(WORD SubModuleIndex, bool EnabledFlag);
	void __fastcall AccessToTe01(void);
	void __fastcall InitTe01(void);
	void __fastcall ShowTe01(void);

	// параметры приложения
	void __fastcall InitApplication(void);

	// признак ошибки выполнения потока
	bool ThreadError;
	// признак автосохранения настроек
	bool IsAutoSaveSettings;
	// файл настроек
	TIniFile *IniFile;

	// статус загрузки модуля
	bool IsModuleLoaded;
	// виртуальный слот и режим поиска модуля E20-10
	WORD VirtualSlot, ModuleSearchMode;
	// скорость работы шины USB
	BYTE UsbSpeed;
	// название модуля
	BYTE ModuleName[0x10];
	// ревизия и исполнение модуля
	BYTE ModuleRevision, ModuleModification;
	// строчки для списка обнаруженных модулей
	AnsiString ModuleListBoxStrings[MAX_VIRTUAL_SLOTS_QUANTITY];

	// режим обнаружения устройств
	WORD InitDevicesMode;
	// COM-порт для калибратора TE-01
//	WORD Te01ComPort;
	// статус субмодулей калибратора TE-01
//	bool Te01SubmodulesStatusArray[TE01_SUBMODULES_QUANTITY];
	// отображения надписей с строкам статуса субмодулей калибратора TE-01
//	TStaticText *Te01SubmodulesLabelStaticTextArray[TE01_SUBMODULES_QUANTITY];
	// отображения статуса субмодулей калибратора TE-01 с помощью строки
//	TStaticText *Te01SubmodulesStatusStaticTextArray[TE01_SUBMODULES_QUANTITY];
	// отображения статуса субмодулей калибратора TE-01 с помощью светодиодиков
//	ThhALed *Te01SubmodulesLedArray[TE01_SUBMODULES_QUANTITY];

	//
	TCursor SaveCursor;
	// признак автосохранения настроек
//	bool IsAutoSaveSettings;
	// строка с ошибкой выполнения потока
	AnsiString Mes;

public:
	__fastcall TInitDevicesThread(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
