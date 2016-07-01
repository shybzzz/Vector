//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SynchroMain.h"
#include "SynchroThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

// основная форма программы
TMainForm *MainForm;
// поток сбора и отображения данных
TSynchroThread *SynchroThread;
// указатель на интерфейс модуля E14-140
ILE140 *pModule;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// событие Create
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
	DecimalSeparator = '.';

	// реакция на клавишу ESCAPE
	Application->OnShortCut = &ApplicationShortCut;

	// изменим горизонтальный размер формы
	Width = GetSystemMetrics(SM_CXFULLSCREEN) - GetSystemMetrics(SM_CXFULLSCREEN)/20;

	ChannelsQuantity = 0x4;			// будем отображать четыре активных канала
	ChannelPoint = 8192;				// кол-во точек на канал

	// выделим память для Viewer
	Buffer = new int[ChannelPoint * ChannelsQuantity];
	if(!Buffer) { PostMessage(Handle, WM_LOAD_USB_DEVICE, 0x0, 0x0); return; }
	memset(Buffer, 0x0, sizeof(int)*ChannelPoint * ChannelsQuantity);

	// получим Viewer и установим его параметры функционирования
	Viewer = CreateOSC();
                                                                                                       // RGB(192,192, 192)
	// установим параметры Viewer
	ZS.Set(0, ChannelPoint, -8192, 8192, -8192, 8192, 6, "%-6.0f");
	ZP.Set(8, 8, (COLORREF)RGB(192,192, 192), (COLORREF)RGB(0, 0, 0), (COLORREF)RGB(0, 255, 0), (COLORREF)RGB(100, 100, 100));
	Viewer->SetParameters(30, &ZP);
	Viewer->MakeFont(Handle, 8);

	// цвета для активных каналов
	rgb[0] = RGB(255, 255, 255);	// первый канал - белый
	rgb[1] = RGB(0, 255, 255);		// второй канал - бирюзовый
	rgb[2] = RGB(255, 255, 0);   	// третий канал - желтый
	rgb[3] = RGB(255, 0, 0);		//	четвертый канал - синий
	rgb[4] = RGB(0, 255, 0);		//	четвертый канал - синий

	Shape1->Brush->Color = (TColor)rgb[0];
	Shape2->Brush->Color = (TColor)rgb[1];
	Shape3->Brush->Color = (TColor)rgb[2];
	Shape4->Brush->Color = (TColor)rgb[3];

	ActiveChannelComboBox->ItemIndex = ActiveChannel = 0x0;
	Viewer->SetDataI(Buffer, ChannelPoint * ChannelsQuantity, ChannelsQuantity, ActiveChannel, &ZS, rgb);

	IsReenteringInProgress = false;

	// проинициализируем параметры аналоговой синхронизации
	InputModeComboBox->ItemIndex = InputMode;
	if(InputMode == ANALOG_SYNC_E140) DisableAdPars(false);
	else DisableAdPars(true);
	SynchroAdTypeComboBox->ItemIndex = (SynchroAdType = true);
	SynchroAdModeComboBox->ItemIndex = SynchroAdMode;
	SynchroAdcChannelComboBox->ItemIndex = SynchroAdChannel;
	SynchroGainComboBox->ItemIndex = SynchroAdGain;
	SynchroPorogEdit->Text = (SynchroAdPorog = 0x0);

	// компонент все перерисует сам
	ViewerPaintBox->ControlStyle = ViewerPaintBox->ControlStyle << csOpaque;
}

//---------------------------------------------------------------------------
// обработка нажатия клавиши ESCAPE на клавиатуре
//---------------------------------------------------------------------------
void __fastcall TMainForm::ApplicationShortCut(TWMKey &Msg, bool &Handled)
{
	if(Msg.CharCode == VK_ESCAPE)
   	if(Application->MessageBox("Действительно хотите завершить работу?", "Подтвердите завершение!", MB_YESNO + MB_ICONQUESTION) == IDYES)
      	Application->Terminate();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
	// сообщение загрузки модуля
	PostMessage(Handle, WM_LOAD_USB_DEVICE, 0x0, 0x0);
}

//---------------------------------------------------------------------------
// попробуем зарузить модуль
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnLoadUsbDevice(TMessage& Message)
{
	char ModuleName[7];
	WORD i;

	if(!Buffer) { Application->MessageBox("Не могу выделить память под буфер данных!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	Application->ProcessMessages();

	// проверим версию используемой DLL библиотеки
	if(GetDllVersion() != CURRENT_VERSION_LUSBAPI)
	{
		AnsiString ErrorMessage = "Неправильная версия библиотеки Lusbapi.dll!\n";
		ErrorMessage += "Текущая: " + IntToStr(GetDllVersion() >> 0x10) + "." + IntToStr(GetDllVersion() & 0xFFFF) + "      ";
		ErrorMessage += "Требуется: " + IntToStr(CURRENT_VERSION_LUSBAPI >> 0x10) + "." + IntToStr(CURRENT_VERSION_LUSBAPI & 0xFFFF);
		Application->MessageBox(ErrorMessage.c_str(), "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION);
		Close(); return;
	}

	// попытаемся получить указатель на интерфейс для модуля E14-140
	pModule = static_cast<ILE140 *>(CreateLInstance("e140"));
	if(!pModule) { Application->MessageBox("Не могу получить интерфейс на модуль E14-140!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	// попробуем обнаружить модуль E14-140 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) { Application->MessageBox("Не могу обнаружить модуль 'E14-140' в первых 127 виртуальных слотах!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) { Application->MessageBox("Не могу получить дескриптор устройства!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// прочитаем название модуля в нулевом виртуальном слоте
	else if(!pModule->GetModuleName(ModuleName)) { Application->MessageBox("Не могу прочитать название модуля в нулевом виртуальном слоте!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// проверим, что это 'E14-140'
	else if(StrComp(ModuleName, "E140")) { Application->MessageBox("В нулевом виртуальном слоте не модуль 'E14-140'!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// прочитаем ППЗУ модуля
	else if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { Application->MessageBox("Не могу прочитать ППЗУ в модуле E14-140!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// в ППЗУ прописано название модуля?
	else if(strcmp(ModuleDescription.Module.DeviceName, "E14-140"))  { Application->MessageBox("В ППЗУ не прописано название модуля!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// проверим прошивку MCU модуля
	else if((ModuleDescription.Module.Revision == REVISIONS_E140[REVISION_B_E140]) &&
		(strtod((char *)ModuleDescription.Mcu.Version.Version, NULL) < 3.05)) { Application->MessageBox("Для модуля E14-140(Rev.'B') версия прошивки ARM должна быть 3.05 или выше!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	UsbLoadingLed->Brush->Color = clLime;

	// прочитаем текущие параметры работы АЦП
	if(!pModule->GET_ADC_PARS(&ap)) { Application->MessageBox("Не могу выполнить функцию GET_ADC_PARS()!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// установим желаемые параметры АЦП
	ap.ClkSource = INT_ADC_CLOCK_E140;							// внутренний запуск АЦП
	ap.EnableClkOutput = ADC_CLOCK_TRANS_DISABLED_E140; 	// без трансляции тактовых импульсо АЦП
	ap.InputMode = InputMode;								// режим ввода даных с АЦП
	ap.SynchroAdType = SynchroAdType;					// тип аналоговой синхронизации
	ap.SynchroAdMode = SynchroAdMode;					// режим аналоговой сихронизации
	ap.SynchroAdChannel = SynchroAdChannel;			// канал АЦП при аналоговой синхронизации
	ap.SynchroAdPorog = SynchroAdPorog;					// порог срабатывания АЦП при аналоговой синхронизации
	ap.ChannelsQuantity = ChannelsQuantity;			// четыре активных канала
	// какое используем подключение входных каналов: диф. или с общей землей?
	IsCommonGnd = false;
	// формируем управляющую таблицу логических каналов
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i |
   																				(IsCommonGnd << 0x5)/*общая земля*/ |
                                                               (0x1 << 0x6)/*усиление 4*/);
	ap.AdcRate = 100.0;										// частота работы АЦП в кГц
	ap.InterKadrDelay = 0.0;								// межкадровая задержка в млс
	// передача в модуль параметров работы АЦП
	if(!pModule->SET_ADC_PARS(&ap)) { Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	AdcRateStaticText->Caption = FormatFloat("0.0", ap.AdcRate);;
	InterChannelDelayStaticText->Caption = FormatFloat("0.0000", ap.InterKadrDelay);
	ChannelRateStaticText->Caption = FormatFloat("0.00", ap.KadrRate);

	// теперь можно запустить поток сбора и отображения данных
	IsStartSynchroThreadDone = true;
	StartThread();
}

//---------------------------------------------------------------------------
// cобытие MouseMove
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
// cобытие Paint
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewerPaintBoxPaint(TObject *Sender)
{
	if(Viewer) Viewer->OnPaint(ViewerPaintBox->Canvas->Handle, ViewerPaintBox->ClientRect, 1);
}

//---------------------------------------------------------------------------
//	cобытие MouseDown
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
// cобытие DblClick
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
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	if(InputModeComboBox->ItemIndex == ANALOG_SYNC_E140) DisableAdPars(false);
	else DisableAdPars(true);

	// передача параметров синхронизации работы АЦП
	InputMode = (WORD)( InputModeComboBox->ItemIndex);
	ap.InputMode = InputMode;
	ap.SynchroAdType = SynchroAdType;
	ap.SynchroAdMode = SynchroAdMode;
	ap.SynchroAdChannel = (WORD)(SynchroAdChannel | (SynchroAdGain << 6));
	ap.SynchroAdPorog = (SHORT)SynchroAdPorog;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка InputModeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// запустим поток сбора заново
	else StartThread();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroAdcChannelComboBoxChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	// передача параметров синхронизации работы АЦП
   SynchroAdChannel = (WORD)(SynchroAdcChannelComboBox->ItemIndex);
   ap.SynchroAdChannel = (WORD)(SynchroAdChannel | (SynchroAdGain << 6));
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// запустим поток сбора заново
	else StartThread();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroGainComboBoxChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	// передача параметров синхронизации работы АЦП
   SynchroAdGain = (WORD)(SynchroGainComboBox->ItemIndex);
   ap.SynchroAdChannel = (WORD)(SynchroAdChannel | (SynchroAdGain << 6));
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// запустим поток сбора заново
	else StartThread();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroAdPorogTrackBarChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	SynchroAdPorog = SynchroAdPorogTrackBar->Position;
   if(SynchroAdPorog > 8191) SynchroAdPorog = 8191;
   else if(SynchroAdPorog < -8191) SynchroAdPorog = -8192;
	SynchroPorogEdit->Text = " " + FormatFloat("0.", SynchroAdPorogTrackBar->Position);

	// передача параметров синхронизации работы АЦП
   ap.SynchroAdPorog = (SHORT)SynchroAdPorog;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// запустим поток сбора заново
	else StartThread();

	// конец реентерабельности...
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
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

   if(SynchroAdTypeComboBox->ItemIndex) { SynchroAdModeComboBox->Items->Strings[0] = "  Снизу-вверх"; SynchroAdModeComboBox->Items->Strings[1] = "  Сверху-вниз"; }
   else { SynchroAdModeComboBox->Items->Strings[0] = "  Выше"; SynchroAdModeComboBox->Items->Strings[1] = "  Ниже"; }
	SynchroAdType = SynchroAdTypeComboBox->ItemIndex;
   SynchroAdModeComboBox->ItemIndex = SynchroAdMode;

	// передача параметров синхронизации работы АЦП
   ap.SynchroAdType = SynchroAdType;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// запустим поток сбора заново
	else StartThread();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::SynchroAdModeComboBoxChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	// передача параметров синхронизации работы АЦП
   SynchroAdMode = SynchroAdModeComboBox->ItemIndex;
   ap.SynchroAdMode = SynchroAdMode;
	if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка SynchroAdTypeComboBoxChange()!!!", MB_OK + MB_ICONINFORMATION);
	// запустим поток сбора заново
	else StartThread();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	// освободим интерфейс модуля
	if(pModule) { pModule->ReleaseLInstance(); pModule = NULL; }
	// освободим ресурсы Viewer
	if(Viewer) { Viewer->Release(); Viewer = NULL; }
	// освободим память
	if(Buffer) { delete[] Buffer; Buffer = NULL; }
}

//---------------------------------------------------------------------------
// запуск потока сбора данных
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
// собственно сам запуск потока
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnStartSynchroThread(TMessage& Message)
{
	IsSynchroThreadRunning = false;
	SynchroThread = new TSynchroThread(false);
	if(!SynchroThread) { Application->MessageBox("Не могу открыть поток 'SynchroThread'!","Ошибка OnStartSynchroThread()!", MB_OK); Close(); return; }
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
// останов потока сбора данных
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
// изменение активного канала
//---------------------------------------------------------------------------
void __fastcall TMainForm::ActiveChannelComboBoxChange(TObject *Sender)
{
	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	ActiveChannel = (WORD)ActiveChannelComboBox->ItemIndex;
   Viewer->SetDataI(Buffer, ChannelPoint * ChannelsQuantity, ChannelsQuantity, ActiveChannel, &ZS, rgb);

	// запустим поток сбора заново
	StartThread();

	// конец реентерабельности...
	IsReenteringInProgress = false;
}
//---------------------------------------------------------------------------

