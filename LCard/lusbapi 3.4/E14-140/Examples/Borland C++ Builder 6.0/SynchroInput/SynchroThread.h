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

	// ошибка выполнения потока сбора данных
	bool ThreadError;
	// дескриптор устройства
	HANDLE ModuleHandle;
	// кол-во акивных каналов и отсчётов на канал
	DWORD ChannelsQuantity, ChannelPoint;
	// буфер данных
   SHORT *ReadBuffer;
	// общее кол-во собираемых данных
   DWORD PointsToRead;
	// OVERLAPPED структура
	OVERLAPPED ReadOv;
	// структура с параметрами запроса на ввод данных
	IO_REQUEST_LUSBAPI IoReq;
	// структура параметров работы АЦП модуля
	ADC_PARS_E140 ap;

	AnsiString Mes;
};
//---------------------------------------------------------------------------
#endif
