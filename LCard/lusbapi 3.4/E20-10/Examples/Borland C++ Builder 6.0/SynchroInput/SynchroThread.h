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
	ADC_PARS_E2010 ap;
	// ревизия модуля
	BYTE ModuleRevision;
	// индекс входного диапазона
	WORD InputRangeIndex;
	// флажки перегрузки каналов в 'плюс' и 'минус'
	BOOL PlusOverload[ADC_CHANNELS_QUANTITY_E2010], MinusOverload[ADC_CHANNELS_QUANTITY_E2010];
	// массивы корректировочных коэффициентов
   double OffsetArray[ADC_INPUT_RANGES_QUANTITY_E2010][ADC_CHANNELS_QUANTITY_E2010];
	double ScaleArray[ADC_INPUT_RANGES_QUANTITY_E2010][ADC_CHANNELS_QUANTITY_E2010];

	AnsiString Mes;

public:
	__fastcall TSynchroThread(bool CreateSuspended);

};
//---------------------------------------------------------------------------
#endif
