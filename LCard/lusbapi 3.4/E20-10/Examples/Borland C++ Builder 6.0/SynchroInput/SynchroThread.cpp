//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SynchroMain.h"
#include "SynchroThread.h"
#pragma package(smart_init)

//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TSynchroThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

// указатель на интерфейс модуля
extern ILE2010 *pModule;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
__fastcall TSynchroThread::TSynchroThread(bool CreateSuspended) : TThread(CreateSuspended)
{
	FreeOnTerminate = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::Execute()
{
	//---- Place thread code here ----
	// инициализация текущего потока
	Synchronize(InitThread);
	// остановим работу АЦП и одновременно сбросим USB-канал чтения данных
	if(!ThreadError) { if(!pModule->STOP_ADC()) ThreadError = true; }
	// основной цикл сбора и отображения данных
	while(!Terminated && !ThreadError)
	{
		// делаем предварительный запрос на ввод данных
		if(!pModule->ReadData(&IoReq)) { Mes = "Не могу послать запрос на сбор данных с модуля!"; Synchronize(ShowErrorMessageBox); break; }
		// стартанем сбор данных
		if(!pModule->START_ADC()) { Mes = "Не могу стартануть сбор данных!"; Synchronize(ShowErrorMessageBox); break; }
		// ожидание завершения сбора данных
		if(!WaitingForIoRequestCompleted(&IoReq)) break;
		// остановим АЦП и одновременно сбросим USB-канал чтения данных
		if(!pModule->STOP_ADC()) { Mes = "Не могу завершить сбор данных!"; Synchronize(ShowErrorMessageBox); break; }
		// корректируем полученные данные
		if(Terminated) break;
		// для модуля ревизии 'A' делаем корректировку собранных данных
		if(ModuleRevision == REVISIONS_E2010[REVISION_A_E2010]) DataCorrection();
		// отобразим полученные данные
		if(Terminated) break;
		Synchronize(RedrawViewer);
		// задержечка
		Sleep(300);
	}

	// остановим АЦП и одновременно сбросим USB-канал чтения данных
	if(!pModule->STOP_ADC()) { Mes = "Не могу завершить сбор данных!"; Synchronize(ShowErrorMessageBox); }
	// прервём все асинхронные запросы
	if(!CancelIo(ModuleHandle)) { Mes = "Не могу прервать асинхронный сбор!"; Synchronize(ShowErrorMessageBox); }
	// освободим используемые ресурсы
	FreeResource();
	// ждём-с
	while(!Terminated) { Sleep(50); }
}

//---------------------------------------------------------------------------
// инициализация текущего потока
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::InitThread(void)
{
	WORD i;

	// сбросим флажок наличия ошибки
	ThreadError = false;
	// мы в потоке
	MainForm->IsSynchroThreadRunning = true;
	// дескриптор устройства
	ModuleHandle = MainForm->ModuleHandle;
	// кол-во акивных каналов
	ChannelsQuantity = MainForm->ChannelsQuantity;
	// кол-во отсчётов на канал
	ChannelPoint = MainForm->ChannelPoint;
	// общее кол-во собираемых данных
	PointsToRead = ChannelPoint * ChannelsQuantity;
	// попробуем выделить буфер под получаемые с платы данные
	ReadBuffer	 = new SHORT[PointsToRead];
	if(!ReadBuffer) { Mes = "Не могу выделить память под получаемые данные!"; ShowErrorMessageBox(); return; }
	// индекс входного диапазона
	InputRangeIndex = MainForm->AdcInputRangeIndex;
	// структура параметров работы АЦП модуля
	ap = MainForm->ap;
	// ревизия модуля
	ModuleRevision = MainForm->ModuleDescription.Module.Revision;

	// копируем корректировочные коэффициенты
	CopyMemory(OffsetArray, MainForm->OffsetArray, sizeof(OffsetArray));
	CopyMemory(ScaleArray, MainForm->ScaleArray, sizeof(ScaleArray));

	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		// сбросим флажки перегрузки каналов
		PlusOverload[i] = MinusOverload[i] = FALSE;
		// сбросим светодиодики перегрузки каналов
		MainForm->PlusOverloadLedsArray[i]->Brush->Color = clBtnFace;
		MainForm->MinusOverloadLedsArray[i]->Brush->Color = clBtnFace;
	}

	// формируем необходимую структуру для сбора данных
	ZeroMemory(&ReadOv, sizeof(OVERLAPPED));
	// создаём событие для асинхронного запроса
	ReadOv.hEvent = CreateEvent(NULL, FALSE , FALSE, NULL);
	if(!ReadOv.hEvent) { Mes = "Не могу создать событие ReadEvent!"; ShowErrorMessageBox(); return; }
	// формируем структуру IoReq
	IoReq.Buffer = ReadBuffer;
	IoReq.NumberOfWordsToPass = PointsToRead;
	IoReq.NumberOfWordsPassed = 0x0;
	IoReq.Overlapped = &ReadOv;
	IoReq.TimeOut = PointsToRead/ap.KadrRate + 1000;
}

//---------------------------------------------------------------------------
// корректируем полученные данные
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::DataCorrection(void)
{
	DWORD i, j;

	// сбросим флажки перегрузки каналов
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
		PlusOverload[i] = MinusOverload[i] = FALSE;

	// выполним корректировку данных
	for(i = 0x0; i < PointsToRead; i += ChannelsQuantity)
		for(j = 0x0; j < ChannelsQuantity; j++)
		{
			// посмотрим перегрузку отсчёта с текущего канала
			if(ap.OverloadMode == CLIPPING_OVERLOAD_E2010)
			{
				if(ReadBuffer[i + j] == 8191) PlusOverload[j] = TRUE;
				else if(ReadBuffer[i + j] == -8192) MinusOverload[j] = TRUE;
			}
			else
			{
				if(ReadBuffer[i + j] == (SHORT)ADC_PLUS_OVERLOAD_MARKER_E2010) PlusOverload[j] = TRUE;
				else if(ReadBuffer[i + j] == (SHORT)ADC_MINUS_OVERLOAD_MARKER_E2010) MinusOverload[j] = TRUE;
			}
			// корректируем отсчёт с АЦП
			ReadBuffer[i + j] = (ReadBuffer[i + j] + OffsetArray[InputRangeIndex][j]) * ScaleArray[InputRangeIndex][j];
		}
}

//---------------------------------------------------------------------------
// отобразим все графики
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::RedrawViewer(void)
{
	DWORD i;

	// копируем данные в буфер графиков
	for(i = 0x0; i < PointsToRead; i++) MainForm->Buffer[i] = ReadBuffer[i];

	// проверим флажки перегрузки каналов
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		// проверим была ли 'плюс' перегрузка соответствующего канала
		if(PlusOverload[i]) MainForm->PlusOverloadLedsArray[i]->Brush->Color = clRed;
		// сбросим светодиодики 'плюс' перегрузки соответствующего канала
		else MainForm->PlusOverloadLedsArray[i]->Brush->Color = clLime;

		// проверим была ли 'минус' перегрузка соответствующего канала
		if(MinusOverload[i]) MainForm->MinusOverloadLedsArray[i]->Brush->Color = clRed;
		// сбросим светодиодики 'минус' перегрузки соответствующего канала
		else MainForm->MinusOverloadLedsArray[i]->Brush->Color = clLime;
	}
         
	// перерисовываем буфер графиков
	MainForm->ViewerPaintBox->Invalidate();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall TSynchroThread::WaitingForIoRequestCompleted(IO_REQUEST_LUSBAPI * const IoReq)
{
	// ждём завершения очередного запроса
	while(true)
	{
		if(HasOverlappedIoCompleted(IoReq->Overlapped)) return TRUE;
		else if(Terminated) return FALSE;
		else Sleep(20);
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::FreeResource(void)
{
	WORD i;

	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		// сбросим флажки перегрузки каналов
		PlusOverload[i] = MinusOverload[i] = FALSE;
		// сбросим светодиодики перегрузки каналов
		MainForm->PlusOverloadLedsArray[i]->Brush->Color = clBtnFace;
		MainForm->MinusOverloadLedsArray[i]->Brush->Color = clBtnFace;
	}
	// освободим используемое событие
	if(ReadOv.hEvent) { if(!CloseHandle(ReadOv.hEvent)) { Mes = "Не могу закрыть событие ReadEvent!"; Synchronize(ShowErrorMessageBox); } }
	// очистим память буферов данных
	if(ReadBuffer) { delete[] ReadBuffer; ReadBuffer = NULL; }
}

//---------------------------------------------------------------------------
// отображение сообщение с ошибкой
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::ShowErrorMessageBox(void)
{
	if(ThreadError) return;
	Application->MessageBox(Mes.c_str(),"Сообщение TSynchroThread::Execute()!", MB_OK + MB_ICONINFORMATION);
	ThreadError = true;
}


