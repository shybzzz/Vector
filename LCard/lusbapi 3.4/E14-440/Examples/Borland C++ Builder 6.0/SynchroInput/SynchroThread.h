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
	void __fastcall RedrawViewer(void);
	BOOL __fastcall WaitingForIoRequestCompleted(IO_REQUEST_LUSBAPI * const IoReq);
	void __fastcall FreeResource(void);
	void __fastcall ShowErrorMessageBox(void);

protected:
	void __fastcall Execute();

public:
	__fastcall TSynchroThread(bool CreateSuspended);

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
	ADC_PARS_E440 ap;

	AnsiString Mes;
};
//---------------------------------------------------------------------------
#endif
