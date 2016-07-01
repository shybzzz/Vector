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

// ��������� �� ��������� ������
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
	// ������������� �������� ������
	Synchronize(InitThread);
	// ��������� ������ ��� � ������������ ������� USB-����� ������ ������
	if(!ThreadError) { if(!pModule->STOP_ADC()) ThreadError = true; }
	// �������� ���� ����� � ����������� ������
	while(!Terminated && !ThreadError)
	{
		// ������ ��������������� ������ �� ���� ������
		if(!pModule->ReadData(&IoReq)) { Mes = "�� ���� ������� ������ �� ���� ������ � ������!"; Synchronize(ShowErrorMessageBox); break; }
		// ��������� ���� ������
		if(!pModule->START_ADC()) { Mes = "�� ���� ���������� ���� ������!"; Synchronize(ShowErrorMessageBox); break; }
		// �������� ���������� ����� ������
		if(!WaitingForIoRequestCompleted(&IoReq)) break;
		// ��������� ��� � ������������ ������� USB-����� ������ ������
		if(!pModule->STOP_ADC()) { Mes = "�� ���� ��������� ���� ������!"; Synchronize(ShowErrorMessageBox); break; }
		// ������������ ���������� ������
		if(Terminated) break;
		// ��� ������ ������� 'A' ������ ������������� ��������� ������
		if(ModuleRevision == REVISIONS_E2010[REVISION_A_E2010]) DataCorrection();
		// ��������� ���������� ������
		if(Terminated) break;
		Synchronize(RedrawViewer);
		// ����������
		Sleep(300);
	}

	// ��������� ��� � ������������ ������� USB-����� ������ ������
	if(!pModule->STOP_ADC()) { Mes = "�� ���� ��������� ���� ������!"; Synchronize(ShowErrorMessageBox); }
	// ������ ��� ����������� �������
	if(!CancelIo(ModuleHandle)) { Mes = "�� ���� �������� ����������� ����!"; Synchronize(ShowErrorMessageBox); }
	// ��������� ������������ �������
	FreeResource();
	// ���-�
	while(!Terminated) { Sleep(50); }
}

//---------------------------------------------------------------------------
// ������������� �������� ������
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::InitThread(void)
{
	WORD i;

	// ������� ������ ������� ������
	ThreadError = false;
	// �� � ������
	MainForm->IsSynchroThreadRunning = true;
	// ���������� ����������
	ModuleHandle = MainForm->ModuleHandle;
	// ���-�� ������� �������
	ChannelsQuantity = MainForm->ChannelsQuantity;
	// ���-�� �������� �� �����
	ChannelPoint = MainForm->ChannelPoint;
	// ����� ���-�� ���������� ������
	PointsToRead = ChannelPoint * ChannelsQuantity;
	// ��������� �������� ����� ��� ���������� � ����� ������
	ReadBuffer	 = new SHORT[PointsToRead];
	if(!ReadBuffer) { Mes = "�� ���� �������� ������ ��� ���������� ������!"; ShowErrorMessageBox(); return; }
	// ������ �������� ���������
	InputRangeIndex = MainForm->AdcInputRangeIndex;
	// ��������� ���������� ������ ��� ������
	ap = MainForm->ap;
	// ������� ������
	ModuleRevision = MainForm->ModuleDescription.Module.Revision;

	// �������� ���������������� ������������
	CopyMemory(OffsetArray, MainForm->OffsetArray, sizeof(OffsetArray));
	CopyMemory(ScaleArray, MainForm->ScaleArray, sizeof(ScaleArray));

	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		// ������� ������ ���������� �������
		PlusOverload[i] = MinusOverload[i] = FALSE;
		// ������� ������������ ���������� �������
		MainForm->PlusOverloadLedsArray[i]->Brush->Color = clBtnFace;
		MainForm->MinusOverloadLedsArray[i]->Brush->Color = clBtnFace;
	}

	// ��������� ����������� ��������� ��� ����� ������
	ZeroMemory(&ReadOv, sizeof(OVERLAPPED));
	// ������ ������� ��� ������������ �������
	ReadOv.hEvent = CreateEvent(NULL, FALSE , FALSE, NULL);
	if(!ReadOv.hEvent) { Mes = "�� ���� ������� ������� ReadEvent!"; ShowErrorMessageBox(); return; }
	// ��������� ��������� IoReq
	IoReq.Buffer = ReadBuffer;
	IoReq.NumberOfWordsToPass = PointsToRead;
	IoReq.NumberOfWordsPassed = 0x0;
	IoReq.Overlapped = &ReadOv;
	IoReq.TimeOut = PointsToRead/ap.KadrRate + 1000;
}

//---------------------------------------------------------------------------
// ������������ ���������� ������
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::DataCorrection(void)
{
	DWORD i, j;

	// ������� ������ ���������� �������
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
		PlusOverload[i] = MinusOverload[i] = FALSE;

	// �������� ������������� ������
	for(i = 0x0; i < PointsToRead; i += ChannelsQuantity)
		for(j = 0x0; j < ChannelsQuantity; j++)
		{
			// ��������� ���������� ������� � �������� ������
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
			// ������������ ������ � ���
			ReadBuffer[i + j] = (ReadBuffer[i + j] + OffsetArray[InputRangeIndex][j]) * ScaleArray[InputRangeIndex][j];
		}
}

//---------------------------------------------------------------------------
// ��������� ��� �������
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::RedrawViewer(void)
{
	DWORD i;

	// �������� ������ � ����� ��������
	for(i = 0x0; i < PointsToRead; i++) MainForm->Buffer[i] = ReadBuffer[i];

	// �������� ������ ���������� �������
	for(i = 0x0; i < ADC_CHANNELS_QUANTITY_E2010; i++)
	{
		// �������� ���� �� '����' ���������� ���������������� ������
		if(PlusOverload[i]) MainForm->PlusOverloadLedsArray[i]->Brush->Color = clRed;
		// ������� ������������ '����' ���������� ���������������� ������
		else MainForm->PlusOverloadLedsArray[i]->Brush->Color = clLime;

		// �������� ���� �� '�����' ���������� ���������������� ������
		if(MinusOverload[i]) MainForm->MinusOverloadLedsArray[i]->Brush->Color = clRed;
		// ������� ������������ '�����' ���������� ���������������� ������
		else MainForm->MinusOverloadLedsArray[i]->Brush->Color = clLime;
	}
         
	// �������������� ����� ��������
	MainForm->ViewerPaintBox->Invalidate();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall TSynchroThread::WaitingForIoRequestCompleted(IO_REQUEST_LUSBAPI * const IoReq)
{
	// ��� ���������� ���������� �������
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
		// ������� ������ ���������� �������
		PlusOverload[i] = MinusOverload[i] = FALSE;
		// ������� ������������ ���������� �������
		MainForm->PlusOverloadLedsArray[i]->Brush->Color = clBtnFace;
		MainForm->MinusOverloadLedsArray[i]->Brush->Color = clBtnFace;
	}
	// ��������� ������������ �������
	if(ReadOv.hEvent) { if(!CloseHandle(ReadOv.hEvent)) { Mes = "�� ���� ������� ������� ReadEvent!"; Synchronize(ShowErrorMessageBox); } }
	// ������� ������ ������� ������
	if(ReadBuffer) { delete[] ReadBuffer; ReadBuffer = NULL; }
}

//---------------------------------------------------------------------------
// ����������� ��������� � �������
//---------------------------------------------------------------------------
void __fastcall TSynchroThread::ShowErrorMessageBox(void)
{
	if(ThreadError) return;
	Application->MessageBox(Mes.c_str(),"��������� TSynchroThread::Execute()!", MB_OK + MB_ICONINFORMATION);
	ThreadError = true;
}


