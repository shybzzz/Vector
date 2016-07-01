//---------------------------------------------------------------------------
#ifndef SynchroThreadH
#define SynchroThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class TSynchroThread : public TThread
{
private:
	void __fastcall InitThread(void);
	void __fastcall DataCorrection(void);
	void __fastcall RedrawViewer(void);
	BOOL __fastcall WaitingForIoRequestCompleted(IO_REQUEST_LUSBAPI * const IoReq);
	void __fastcall FreeResource(void);
	void __fastcall ShowErrorMessageBox(void);

protected:
	void __fastcall Execute();

	// ������ ���������� ������ ����� ������
	bool ThreadError;
	// ���������� ����������
	HANDLE ModuleHandle;
	// ���-�� ������� ������� � �������� �� �����
	DWORD ChannelsQuantity, ChannelPoint;
	// ����� ������
   SHORT *ReadBuffer;
	// ����� ���-�� ���������� ������
   DWORD PointsToRead;
	// OVERLAPPED ���������
	OVERLAPPED ReadOv;
	// ��������� � ����������� ������� �� ���� ������
	IO_REQUEST_LUSBAPI IoReq;
	// ��������� ���������� ������ ��� ������
	ADC_PARS_E2010 ap;
	// ������� ������
	BYTE ModuleRevision;
	// ������ �������� ���������
	WORD InputRangeIndex;
	// ������ ���������� ������� � '����' � '�����'
	BOOL PlusOverload[ADC_CHANNELS_QUANTITY_E2010], MinusOverload[ADC_CHANNELS_QUANTITY_E2010];
	// ������� ���������������� �������������
   double OffsetArray[ADC_INPUT_RANGES_QUANTITY_E2010][ADC_CHANNELS_QUANTITY_E2010];
	double ScaleArray[ADC_INPUT_RANGES_QUANTITY_E2010][ADC_CHANNELS_QUANTITY_E2010];

	AnsiString Mes;

public:
	__fastcall TSynchroThread(bool CreateSuspended);

};
//---------------------------------------------------------------------------
#endif
