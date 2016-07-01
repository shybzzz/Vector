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
// указатель на интерфейс модуля E20-10
ILE2010 *pModule;

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
	ChannelPoint = 4*8192;			// кол-во точек на канал

	// выделим память для Viewer
	Buffer = new int[ChannelPoint * ChannelsQuantity];
	if(!Buffer) { PostMessage(Handle, WM_LOAD_USB_DEVICE, 0x0, 0x0); return; }
	memset(Buffer, 0x0, sizeof(int)*ChannelPoint * ChannelsQuantity);

	// получим Viewer и установим его параметры функционирования
	Viewer = CreateOSC();

	// установим параметры Viewer
//	ZS.Set(0, ChannelPoint, -8192, 8192, -8192, 8192, 6, "%-6.0f");
	ZS.Set(0.0, ChannelPoint, -8500.0, 8500.0, -8500.0, 8500.0, 6.0, "%-6.0f");
	ZP.Set(8, 8, (COLORREF)RGB(192,192, 192), (COLORREF)RGB(0, 0, 0), (COLORREF)RGB(0, 255, 0), (COLORREF)RGB(100, 100, 100));
	Viewer->SetParameters(30, &ZP);
	Viewer->MakeFont(Handle, 8);

	// цвета для активных каналов
	rgb[0] = RGB(255, 255, 255);	// первый канал - белый
	rgb[1] = RGB(0, 255, 255);		// второй канал - бирюзовый
	rgb[2] = RGB(255, 255, 0);   	// третий канал - желтый
	rgb[3] = RGB(255, 0, 0);		//	четвертый канал - синий

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

	// проинициализируем параметры синхронизации ввода данных
	StartSourceIndex = INT_ADC_START_E2010;
	AdcStartPulseSourceArrayRadioButton[StartSourceIndex]->Checked = true;
	SynhroSourceIndex = INT_ADC_CLOCK_E2010;
	AdcConvertPulseSourceArrayRadioButton[SynhroSourceIndex]->Checked = true;

	IsReenteringInProgress = false;

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
	WORD i, j;   

	if(!Buffer) { Application->MessageBox("Не могу выделить память под буфер данных!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	Application->ProcessMessages();

	// проверим версию используемой DLL библиотеки
	if(GetDllVersion() != CURRENT_VERSION_LUSBAPI)
	{
		AnsiString ErrorMessage = "Неправильная версия библиотеки S211api.dll!\n";
		ErrorMessage += "Текущая: " + IntToStr(GetDllVersion() >> 0x10) + "." + IntToStr(GetDllVersion() & 0xFFFF) + "      ";
		ErrorMessage += "Требуется: " + IntToStr(CURRENT_VERSION_LUSBAPI >> 0x10) + "." + IntToStr(CURRENT_VERSION_LUSBAPI & 0xFFFF);
		Application->MessageBox(ErrorMessage.c_str(), "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION);
		Close(); return;
	}

	// попытаемся получить указатель на интерфейс для модуля E20-10
	pModule = static_cast<ILE2010 *>(CreateLInstance("e2010"));
	if(pModule == NULL) { Application->MessageBox("Не могу получить интерфейс на модуль E20-10!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	// попробуем обнаружить модуль E20-10 в первых MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI виртуальных слотах
	for(i = 0x0; i < MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI; i++) if(pModule->OpenLDevice(i)) break;
	if(i == MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI) { Application->MessageBox("Не могу обнаружить модуль 'E20-10' в первых 127 виртуальных слотах!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
//	if(!pModule->InitLDevice(0)) { Application->MessageBox("Не могу обнаружить модуль в нулевом виртуальном слоте", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// попробуем прочитать дескриптор устройства
	ModuleHandle = pModule->GetModuleHandle();
	if(ModuleHandle == INVALID_HANDLE_VALUE) { Application->MessageBox("Не могу получить дескриптор устройства!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// прочитаем название модуля в нулевом виртуальном слоте
	else if(!pModule->GetModuleName(ModuleName)) { Application->MessageBox("Не могу прочитать название модуля в нулевом виртуальном слоте!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// проверим, что это 'E20-10'
	else if(StrComp(ModuleName, "E20-10")) { Application->MessageBox("В нулевом виртуальном слоте не модуль 'E20-10'!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// попробуем получить скорость работы шины USB
	else if(!pModule->GetUsbSpeed(&UsbSpeed)) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("Не могу определить скорость работы USB шины!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// теперь можно попытаться загрузить штатный образ для ПЛИС в модуль E20-10
	// Образ для ПЛИС возьмём из соответствующего ресурса DLL библиотеки
	else if(!pModule->LOAD_MODULE()) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("Не могу загрузить ПЛИС модуля E20-10!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// теперь проверим загрузку модуля
 	else if(!pModule->TEST_MODULE()) { UsbLoadingLed->Brush->Color = clRed; Application->MessageBox("Модуль E20-10 не загрузился!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// прочитаем ППЗУ модуля
	else if(!pModule->GET_MODULE_DESCRIPTION(&ModuleDescription)) { Application->MessageBox("Не могу прочитать ППЗУ в модуле E20-10!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	// в ППЗУ прописано название модуля?
	else if(strcmp(ModuleDescription.Module.DeviceName, "E20-10"))  { Application->MessageBox("В ППЗУ не прописано название модуля!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }

	// зажгём зеленую лампочку успешного обнаружения модуля E20-10
	UsbLoadingLed->Brush->Color = clLime;
	UsbSpeedLed->Brush->Color = UsbSpeed ? clLime : (TColor)RGB(255, 150, 0);
	UsbSpeedModeStaticText->Caption = UsbSpeed ? "HS" : "FS";
	// копируем корректировочные коэффициенты
	CopyMemory(OffsetArray, ModuleDescription.Adc.OffsetCalibration, sizeof(OffsetArray));
	CopyMemory(ScaleArray, ModuleDescription.Adc.ScaleCalibration, sizeof(ScaleArray));

	// прочитаем текущие параметры работы АЦП
	if(!pModule->GET_ADC_PARS(&ap)) { Application->MessageBox("Не могу выполнить функцию GET_ADC_PARS()!", "ОШИБКА!!!", MB_OK + MB_ICONINFORMATION); Close(); return; }
	if(ModuleDescription.Module.Revision == REVISIONS_E2010[REVISION_A_E2010])
	{
		ap.IsAdcCorrectionEnabled = FALSE;		// запретим автоматическую корректировку данных на уровне модуля (для Rev.A)
//		ap.OverloadMode = MARKER_OVERLOAD_E2010;		// фиксация факта перегрузки входных каналов при помощи маркеров в отсчёте АЦП (только для Rev.A)
		ap.OverloadMode = CLIPPING_OVERLOAD_E2010; 	// обычная фиксация факта перегрузки входных каналов путём ограничения путём ограничения отсчёта АЦП (только для Rev.A)
	}
	else
		ap.IsAdcCorrectionEnabled = TRUE;		// разрешим автоматическую корректировку данных на уровне модуля (для Rev.B и выше)
	// установим желаемые параметры АЦП
	ap.SynchroPars.StartSource = StartSourceIndex;					// источник импульса старта АЦП
	ap.SynchroPars.SynhroSource = SynhroSourceIndex;				// источник синхроимпульсов АЦП
	ap.SynchroPars.StartDelay = 0x0;									// задержка начала сбора данных в кадрах отсчётов (для Rev.B и выше)
	ap.SynchroPars.StopAfterNKadrs = 0x0;							// останов сбора данных через заданное кол-во кадров отсчётов (для Rev.B и выше)
	ap.SynchroPars.SynchroAdMode = NO_ANALOG_SYNCHRO_E2010;	// тип аналоговой синхронизации (для Rev.B и выше)
//	ap.SynchroPars.SynchroAdMode = ANALOG_SYNCHRO_ON_HIGH_LEVEL_E2010;
	ap.SynchroPars.SynchroAdChannel = 0x0;							// канал аналоговой синхронизации (для Rev.B и выше)
	ap.SynchroPars.SynchroAdPorog = 0;								// порог аналоговой синхронизации в кодах АЦП (для Rev.B и выше)
	ap.SynchroPars.IsBlockDataMarkerEnabled = 0x0;				// маркирование начала блока данных (удобно, например, при аналоговой синхронизации ввода по уровню) (для Rev.B и выше)
	ap.ChannelsQuantity = ChannelsQuantity;			// кол-во активных каналов
	// формируем управляющую таблицу логических каналов
	for(i = 0x0; i < ap.ChannelsQuantity; i++) ap.ControlTable[i] = (WORD)(i);
	// частоту сбора будем устанавливать в зависимости от скорости USB
	ap.AdcRate = 2500.0; 													// частота работы АЦП в кГц
	if(UsbSpeed == USB11_LUSBAPI) ap.InterKadrDelay = 0.01;		// межкадровая задержка в мс
	else ap.InterKadrDelay = 0.0;
	// выберем одинаковый входной диапазон для всех входных каналов
	AdcInputRangeIndex = ADC_INPUT_RANGE_3000mV_E2010;	// входной диапазон ±3В
	// теперь сконфигурим входные каналы
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		ap.InputRange[i] = AdcInputRangeIndex;  	 		// входной диапазон ±3В
		ap.InputSwitch[i] = ADC_INPUT_SIGNAL_E2010;		// источник входа - сигнал
	}
//	ap.InputSwitch[0x3] = ADC_INPUT_ZERO_E2010;			// источник входа - земля
	// передаём в структуру параметров работы АЦП корректировочные коэффициенты АЦП
	for(i = 0x0; i < ADC_INPUT_RANGES_QUANTITY_E2010; i++)
		for(j = 0x0; j  < ADC_CHANNELS_QUANTITY_E2010; j++)
		{
			// корректировка смещения
			ap.AdcOffsetCoefs[i][j] = ModuleDescription.Adc.OffsetCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
			// корректировка масштаба
			ap.AdcScaleCoefs[i][j] = ModuleDescription.Adc.ScaleCalibration[j + i*ADC_CHANNELS_QUANTITY_E2010];
		}
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

	// конец реентерабельности...
	IsReenteringInProgress = false;
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
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::AdcStartPulseRadioButtonClick(TObject *Sender)
{
	WORD i;

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	for(i = 0x0; i < INVALID_ADC_START_E2010; i++) if(AdcStartPulseSourceArrayRadioButton[i] == Sender) break;
	if(i == INVALID_ADC_START_E2010) Application->MessageBox("Не могу открыть поток источник импульса старта АЦП!","Ошибка AdcStartPulseRadioButtonClick()!", MB_OK);
	else
	{
		// источник импульса старта АЦП
		ap.SynchroPars.StartSource = StartSourceIndex = i;
		// передача в модуль параметров работы АЦП
		if(!pModule->SET_ADC_PARS(&ap))
		{
			pModule->GetLastErrorInfo(&LatsErrorInfo);
			ErrorStr = "Не могу выполнить функцию SET_ADC_PARS().\n";
			ErrorStr += (AnsiString)(char *)LatsErrorInfo.ErrorString;
			Application->MessageBox(ErrorStr.c_str(), "Ошибка AdcStartPulseRadioButtonClick()!", MB_OK + MB_ICONINFORMATION);
//			Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка AdcStartPulseRadioButtonClick()!", MB_OK + MB_ICONINFORMATION);
		}
		// запустим поток сбора заново
		else StartThread();
	}

	// конец реентерабельности...
	IsReenteringInProgress = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::AdcConvertPulseRadioButtonClick(TObject *Sender)
{
	WORD i;

	// реентерабельность...
	if(IsReenteringInProgress) return;
	else IsReenteringInProgress = true;

	// прервем текущий поток сбора
	StopThread();

	for(i = 0x0; i < INVALID_ADC_START_E2010; i++) if(AdcConvertPulseSourceArrayRadioButton[i] == Sender) break;
	if(i == INVALID_ADC_START_E2010) Application->MessageBox("Не могу открыть поток источник синхроимпульсов АЦП!","Ошибка AdcConvertPulseRadioButtonClick()!", MB_OK);
	else
	{
		// источник синхроиимпульсов АЦП
		ap.SynchroPars.SynhroSource = SynhroSourceIndex = i;
		// передача в модуль параметров работы АЦП
		if(!pModule->SET_ADC_PARS(&ap)) Application->MessageBox("Не могу выполнить функцию SET_ADC_PARS()!", "Ошибка AdcConvertPulseRadioButtonClick()!", MB_OK + MB_ICONINFORMATION);
		// запустим поток сбора заново
		else StartThread();
	}

	// конец реентерабельности...
	IsReenteringInProgress = false;
}
//---------------------------------------------------------------------------

