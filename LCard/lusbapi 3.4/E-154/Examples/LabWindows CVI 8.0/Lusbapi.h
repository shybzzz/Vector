#ifndef __LusbapiH__
#define __LusbapiH__

	// --------------------------------------------------------------------------
	// ---------------------------- COMMON PART ---------------------------------
	// --------------------------------------------------------------------------
	#include <windows.h>
	#include "LusbapiTypes.h"

	// версия библиотеки
	#define 	VERSION_MAJOR_LUSBAPI 			(0x3)   	// только одна цифра
	#define 	VERSION_MINOR_LUSBAPI 			(0x2)		// только одна цифра
	#define 	CURRENT_VERSION_LUSBAPI			((VERSION_MAJOR_LUSBAPI << 0x10) | VERSION_MINOR_LUSBAPI)

	#define InitLDevice(Slot) OpenLDevice(Slot)

	// экспортирукемые функции
	extern "C" DWORD WINAPI GetDllVersion(void);
	extern "C" LPVOID WINAPI CreateLInstance(PCHAR const DeviceName);

	// возможные индексы скорости работы модуля на шине USB
	enum { USB11_LUSBAPI, USB20_LUSBAPI, INVALID_USB_SPEED_LUSBAPI };
	// максимально возможное кол-во опрашиваемых виртуальных слотов
	const WORD MAX_VIRTUAL_SLOTS_QUANTITY_LUSBAPI = 127;


	// ==========================================================================
	// *************************** L-Card USB BASE ******************************
	// ==========================================================================
	struct ILUSBBASE
	{
		// функции общего назначения для работы с USB устройствами
		virtual BOOL WINAPI OpenLDevice(WORD VirtualSlot) = 0;
		virtual BOOL WINAPI CloseLDevice(void) = 0;
		virtual BOOL WINAPI ReleaseLInstance(void) = 0;
		// получение дескриптора устройства USB
		virtual HANDLE WINAPI GetModuleHandle(void) = 0;
		// получение названия используемого модуля
		virtual BOOL WINAPI GetModuleName(PCHAR const ModuleName) = 0;
		// получение текущей скорости работы шины USB
		virtual BOOL WINAPI GetUsbSpeed(BYTE * const UsbSpeed) = 0;
		// функция выдачи строки с последней ошибкой
		virtual BOOL WINAPI GetLastErrorInfo(LAST_ERROR_INFO_LUSBAPI * const LastErrorInfo) = 0;
	};



	// ==========================================================================
	// *************************** Модуль E14-140 *******************************
	// ==========================================================================
	// доступные индексы диапазонов входного напряжения модуля E14-140
	enum {	ADC_INPUT_RANGE_10000mV_E140, ADC_INPUT_RANGE_2500mV_E140, ADC_INPUT_RANGE_625mV_E140, ADC_INPUT_RANGE_156mV_E140, INVALID_ADC_INPUT_RANGE_E140 };
	// доступные индексы источника тактовых импульсов для АЦП
	enum {	INT_ADC_CLOCK_E140, EXT_ADC_CLOCK_E140, INVALID_ADC_CLOCK_E140 };
	// доступные индексы управления трансляцией тактовых импульсов АЦП
	// на линию SYN внешнего цифрового разъёма (только при внутреннем
	// источнике тактовых импульсоц АЦП)
	enum {	ADC_CLOCK_TRANS_DISABLED_E140, ADC_CLOCK_TRANS_ENABLED_E140, INVALID_ADC_CLOCK_TRANS_E140 };
	// возможные типы синхронизации модуля E14-140
	enum { 	NO_SYNC_E140, TTL_START_SYNC_E140, TTL_KADR_SYNC_E140, ANALOG_SYNC_E140, INVALID_SYNC_E140 };
	// возможные опции наличия микросхемы ЦАП
	enum {	DAC_INACCESSIBLED_E140, DAC_ACCESSIBLED_E140, INVALID_DAC_OPTION_E140 };
	// доступные индексы ревизий модуля E14-140
	enum {	REVISION_A_E140, /*REVISION_B_E140, */ INVALID_REVISION_E140 };

	// константы для работы с модулем
	enum 	{
				MAX_CONTROL_TABLE_LENGTH_E140 = 128,
				ADC_INPUT_RANGES_QUANTITY_E140 = INVALID_ADC_INPUT_RANGE_E140,
				ADC_CALIBR_COEFS_QUANTITY_E140 = ADC_INPUT_RANGES_QUANTITY_E140,
				DAC_CHANNELS_QUANTITY_E140 = 0x2, DAC_CALIBR_COEFS_QUANTITY_E140 = DAC_CHANNELS_QUANTITY_E140,
				TTL_LINES_QUANTITY_E140 = 0x10,	  		// кол-во цифровых линий
				USER_FLASH_SIZE_E140 = 0x200,   			// размер области пользовательского ППЗУ в байтах
				REVISIONS_QUANTITY_E140 = INVALID_REVISION_E140,		// кол-во ревизий (модификаций) модуля
			};
	// диапазоны входного напряжения АЦП в В
	const double ADC_INPUT_RANGES_E140[ADC_INPUT_RANGES_QUANTITY_E140] =
	{
		10.0, 10.0/4.0, 10.0/16.0, 10.0/64.0
	};
	// диапазоны выходного напряжения ЦАП в В
	const double DAC_OUTPUT_RANGE_E140 = 5.0;
	// доступные ревизии модуля
	const BYTE REVISIONS_E140[REVISIONS_QUANTITY_E140] = { 'A' };

	#pragma pack(1)
	// структура с информацией об модуле E14-140
	struct MODULE_DESCRIPTION_E140
	{
		MODULE_INFO_LUSBAPI     Module;		// общая информация о модуле
		INTERFACE_INFO_LUSBAPI  Interface;	// информация об используемом интерфейсе
		MCU_INFO_LUSBAPI<VERSION_INFO_LUSBAPI>		Mcu;	// информация о микроконтроллере
		ADC_INFO_LUSBAPI        Adc;			// информация о АЦП
		DAC_INFO_LUSBAPI        Dac;			// информация о ЦАП
		DIGITAL_IO_INFO_LUSBAPI DigitalIo;	// информация о цифровом вводе-выводе
	};
	// структура, задающая режим работы АЦП для модуля E14-140
	struct ADC_PARS_E140
	{
		WORD ClkSource;							// источник тактовых импульсов для запуска АПП
		WORD EnableClkOutput;					// разрешение трансляции тактовых импульсов запуска АЦП
		WORD InputMode;							// режим ввода даных с АЦП
		WORD SynchroAdType;						// тип аналоговой синхронизации
		WORD SynchroAdMode; 						// режим аналоговой сихронизации
		WORD SynchroAdChannel;  				// канал АЦП при аналоговой синхронизации
		SHORT SynchroAdPorog; 					// порог срабатывания АЦП при аналоговой синхронизации
		WORD ChannelsQuantity;					// число активных каналов
		WORD ControlTable[128];					// управляющая таблица с активными каналами
		double AdcRate;	  			  			// частота работы АЦП в кГц
		double InterKadrDelay;		  			// межкадровая задержка в мс
		double KadrRate;							// частота кадра в кГц
	};
	#pragma pack()

	//-----------------------------------------------------------------------------
	// интерфейс для модуля E14-140
	//-----------------------------------------------------------------------------
	struct ILE140 : public ILUSBBASE
	{
		// функции для работы с АЦП
		virtual BOOL WINAPI GET_ADC_PARS(ADC_PARS_E140 * const AdcPars) = 0;
 		virtual BOOL WINAPI SET_ADC_PARS(ADC_PARS_E140 * const AdcPars) = 0;
		virtual BOOL WINAPI START_ADC(void) = 0;
		virtual BOOL WINAPI STOP_ADC(void) = 0;
		virtual BOOL WINAPI ADC_KADR(SHORT * const Data) = 0;
		virtual BOOL WINAPI ADC_SAMPLE(SHORT * const AdcData, WORD AdcChannel) = 0;
		virtual BOOL WINAPI ReadData(IO_REQUEST_LUSBAPI * const ReadRequest) = 0;

		// функции для работы с ЦАП
		virtual BOOL WINAPI DAC_SAMPLE(SHORT * const DacData, WORD DacChannel) = 0;

		// функции для работы с ТТЛ линиями
		virtual BOOL WINAPI ENABLE_TTL_OUT(BOOL EnableTtlOut) = 0;
		virtual BOOL WINAPI TTL_IN(WORD * const TtlIn) = 0;
		virtual BOOL WINAPI TTL_OUT(WORD TtlOut) = 0;

		// функции для работы с пользовательской информацией ППЗУ
		virtual BOOL WINAPI ENABLE_FLASH_WRITE(BOOL IsUserFlashWriteEnabled) = 0;
		virtual BOOL WINAPI READ_FLASH_WORD(WORD FlashAddress, SHORT * const FlashWord) = 0;
		virtual BOOL WINAPI WRITE_FLASH_WORD(WORD FlashAddress, SHORT FlashWord) = 0;

		// функции для работы со служебной информацией ППЗУ
		virtual BOOL WINAPI GET_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E140 * const ModuleDescription) = 0;
		virtual BOOL WINAPI SAVE_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E140 * const ModuleDescription) = 0;
	};



	// ==========================================================================
	// *************************** Модуль E-154 *******************************
	// ==========================================================================
	// доступные индексы диапазонов входного напряжения модуля E-154
	enum {	ADC_INPUT_RANGE_5000mV_E154, ADC_INPUT_RANGE_1600mV_E154, ADC_INPUT_RANGE_500mV_E154, ADC_INPUT_RANGE_160mV_E154, INVALID_ADC_INPUT_RANGE_E154 };
	// доступные индексы источника тактовых импульсов для АЦП
	enum {	INT_ADC_CLOCK_E154, EXT_ADC_CLOCK_E154, INVALID_ADC_CLOCK_E154 };
	// доступные индексы управления трансляцией тактовых импульсов АЦП
	// на линию SYN внешнего цифрового разъёма (только при внутреннем
	// источнике тактовых импульсоц АЦП)
	enum {	ADC_CLOCK_TRANS_DISABLED_E154, ADC_CLOCK_TRANS_ENABLED_E154, INVALID_ADC_CLOCK_TRANS_E154 };
	// возможные типы синхронизации модуля E-154
	enum { 	NO_SYNC_E154, TTL_START_SYNC_E154, TTL_KADR_SYNC_E154, ANALOG_SYNC_E154, INVALID_SYNC_E154 };
	// возможные опции наличия микросхемы ЦАП
	enum {	DAC_INACCESSIBLED_E154, DAC_ACCESSIBLED_E154, INVALID_DAC_OPTION_E154 };
	// доступные индексы ревизий модуля E-154
	enum {	REVISION_A_E154, INVALID_REVISION_E154 };

	// константы для работы с модулем
	enum 	{
				MAX_CONTROL_TABLE_LENGTH_E154 = 16,
				ADC_INPUT_RANGES_QUANTITY_E154 = INVALID_ADC_INPUT_RANGE_E154,
				ADC_CALIBR_COEFS_QUANTITY_E154 = ADC_INPUT_RANGES_QUANTITY_E154,
				DAC_CHANNELS_QUANTITY_E154 = 0x1, DAC_CALIBR_COEFS_QUANTITY_E154 = DAC_CHANNELS_QUANTITY_E154,
				TTL_LINES_QUANTITY_E154 = 0x08,	  		// кол-во цифровых линий
				USER_FLASH_SIZE_E154 = 0x200,   			// размер области пользовательского ППЗУ в байтах
				REVISIONS_QUANTITY_E154 = INVALID_REVISION_E154,		// кол-во ревизий (модификаций) модуля
			};
	// диапазоны входного напряжения АЦП в В
	const double ADC_INPUT_RANGES_E154[ADC_INPUT_RANGES_QUANTITY_E154] =
	{
		5.0, 5.0/4.0, 5.0/16.0, 5.0/64.0
	};
	// диапазоны выходного напряжения ЦАП в В
	const double DAC_OUTPUT_RANGE_E154 = 5.0;
	// доступные ревизии модуля
	const BYTE REVISIONS_E154[REVISIONS_QUANTITY_E154] = { 'A' };

	#pragma pack(1)
	// структура с информацией об модуле E-154
	struct MODULE_DESCRIPTION_E154
	{
		MODULE_INFO_LUSBAPI     Module;		// общая информация о модуле
		INTERFACE_INFO_LUSBAPI  Interface;	// информация об используемом интерфейсе
		MCU_INFO_LUSBAPI<VERSION_INFO_LUSBAPI>		Mcu;	// информация о микроконтроллере
		ADC_INFO_LUSBAPI        Adc;			// информация о АЦП
		DAC_INFO_LUSBAPI        Dac;			// информация о ЦАП
		DIGITAL_IO_INFO_LUSBAPI DigitalIo;	// информация о цифровом вводе-выводе
	};
	// структура, задающая режим работы АЦП для модуля E14-154
	struct ADC_PARS_E154
	{
		WORD ClkSource;							// источник тактовых импульсов для запуска АПП
		WORD EnableClkOutput;					// разрешение трансляции тактовых импульсов запуска АЦП
		WORD InputMode;							// режим ввода даных с АЦП
		WORD SynchroAdType;						// тип аналоговой синхронизации
		WORD SynchroAdMode; 						// режим аналоговой сихронизации
		WORD SynchroAdChannel;  				// канал АЦП при аналоговой синхронизации
		SHORT SynchroAdPorog; 					// порог срабатывания АЦП при аналоговой синхронизации
		WORD ChannelsQuantity;					// число активных каналов
		WORD ControlTable[128];					// управляющая таблица с активными каналами
		double AdcRate;	  			  			// частота работы АЦП в кГц
		double InterKadrDelay;		  			// межкадровая задержка в мс
		double KadrRate;							// частота кадра в кГц
	};
	#pragma pack()

	//-----------------------------------------------------------------------------
	// интерфейс для модуля E14-154
	//-----------------------------------------------------------------------------
	struct ILE154 : public ILUSBBASE
	{
		// функции для работы с АЦП
		virtual BOOL WINAPI GET_ADC_PARS(ADC_PARS_E154 * const AdcPars) = 0;
 		virtual BOOL WINAPI SET_ADC_PARS(ADC_PARS_E154 * const AdcPars) = 0;
		virtual BOOL WINAPI START_ADC(void) = 0;
		virtual BOOL WINAPI STOP_ADC(void) = 0;
		virtual BOOL WINAPI ADC_KADR(SHORT * const Data) = 0;
		virtual BOOL WINAPI ADC_SAMPLE(SHORT * const AdcData, WORD AdcChannel) = 0;
		virtual BOOL WINAPI ReadData(IO_REQUEST_LUSBAPI * const ReadRequest) = 0;

		// функции для работы с ЦАП
		virtual BOOL WINAPI DAC_SAMPLE(SHORT * const DacData, WORD DacChannel) = 0;

		// функции для работы с ТТЛ линиями
		virtual BOOL WINAPI ENABLE_TTL_OUT(BOOL EnableTtlOut) = 0;
		virtual BOOL WINAPI TTL_IN(WORD * const TtlIn) = 0;
		virtual BOOL WINAPI TTL_OUT(WORD TtlOut) = 0;

		// функции для работы с пользовательской информацией ППЗУ
		virtual BOOL WINAPI ENABLE_FLASH_WRITE(BOOL IsUserFlashWriteEnabled) = 0;
		virtual BOOL WINAPI READ_FLASH_WORD(WORD FlashAddress, SHORT * const FlashWord) = 0;
		virtual BOOL WINAPI WRITE_FLASH_WORD(WORD FlashAddress, SHORT FlashWord) = 0;

		// функции для работы со служебной информацией ППЗУ
		virtual BOOL WINAPI GET_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E154 * const ModuleDescription) = 0;
		virtual BOOL WINAPI SAVE_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E154 * const ModuleDescription) = 0;
	};



	// ==========================================================================
	// *************************** Модуль E14-440 *******************************
	// ==========================================================================
	// доступные состояния сброса модуля E14-440
	enum {	INIT_E440, RESET_E440, INVALID_RESET_TYPE_E440 };
	// доступные индексы диапазонов входного напряжения модуля E14-440
	enum {	ADC_INPUT_RANGE_10000mV_E440, ADC_INPUT_RANGE_2500mV_E440, ADC_INPUT_RANGE_625mV_E440, ADC_INPUT_RANGE_156mV_E440, INVALID_ADC_INPUT_RANGE_E440 };
	// возможные типы синхронизации модуля E14-440
	enum {	NO_SYNC_E440, TTL_START_SYNC_E440, TTL_KADR_SYNC_E440, ANALOG_SYNC_E440, INVALID_SYNC_E440 };
	// возможные опции наличия микросхемы ЦАП
	enum {	DAC_INACCESSIBLED_E440, DAC_ACCESSIBLED_E440, INVALID_DAC_OPTION_E440 };
	// возможные типы DSP (сейчас только ADSP-2185)
	enum {	ADSP2184_E440, ADSP2185_E440, ADSP2186_E440, INVALID_DSP_TYPE_E440 };
	// возможные тактовые частоты модудя (сейчас только 24000 кГц)
	enum {	F14745_E440, F16667_E440, F20000_E440, F24000_E440, INVALID_QUARTZ_FREQ_E440 };
	// доступные индексы ревизий модуля E14-440
	enum {	REVISION_A_E440, REVISION_B_E440, REVISION_C_E440, REVISION_D_E440, REVISION_E_E440, INVALID_REVISION_E440 };

	// константы для работы с модулем
	enum 	{
				MAX_CONTROL_TABLE_LENGTH_E440 = 128,
				ADC_INPUT_RANGES_QUANTITY_E440 = INVALID_ADC_INPUT_RANGE_E440,
				ADC_CALIBR_COEFS_QUANTITY_E440 = ADC_INPUT_RANGES_QUANTITY_E440,
				MAX_ADC_FIFO_SIZE_E440 = 0x3000,			// 12288
				DAC_CHANNELS_QUANTITY_E440 = 0x2, DAC_CALIBR_COEFS_QUANTITY_E440 = DAC_CHANNELS_QUANTITY_E440,
				MAX_DAC_FIFO_SIZE_E440 = 0x0FC0,			// 4032
				TTL_LINES_QUANTITY_E440 = 0x10, 			// кол-во цифровых линий
				REVISIONS_QUANTITY_E440 = INVALID_REVISION_E440,		// кол-во ревизий (модификаций) модуля
			};
	// диапазоны входного напряжения АЦП в В
	const double ADC_INPUT_RANGES_E440[ADC_INPUT_RANGES_QUANTITY_E440] =
	{
		10.0, 10.0/4.0, 10.0/16.0, 10.0/64.0
	};
	// диапазоны выходного напряжения ЦАП в В
	const double DAC_OUTPUT_RANGE_E440 = 5.0;
	// доступные ревизии модуля
	const BYTE REVISIONS_E440[REVISIONS_QUANTITY_E440] = { 'A', 'B', 'C', 'D', 'E' };

	#pragma pack(1)
	// структура с информацией об модуле E14-440
	struct MODULE_DESCRIPTION_E440
	{
		MODULE_INFO_LUSBAPI     Module;		// общая информация о модуле
		INTERFACE_INFO_LUSBAPI  Interface;	// информация об используемом интерфейсе
		MCU_INFO_LUSBAPI<VERSION_INFO_LUSBAPI>		Mcu;	// информация о микроконтроллере
		DSP_INFO_LUSBAPI        Dsp;			// информация о DSP
		ADC_INFO_LUSBAPI        Adc;			// информация о АЦП
		DAC_INFO_LUSBAPI        Dac;			// информация о ЦАП
		DIGITAL_IO_INFO_LUSBAPI DigitalIo;	// информация о цифровом вводе-выводе
	};
	// структура, задающая режим работы АЦП для модуля E-440
	struct ADC_PARS_E440
	{
		BOOL IsAdcEnabled;						// статус работы АЦП (только при чтении)
		BOOL IsCorrectionEnabled;				// управление разрешением корректировкой данных на уровне драйвера DSP
		WORD InputMode;						// режим ввода даных с АЦП
		WORD SynchroAdType;					// тип аналоговой синхронизации
		WORD SynchroAdMode; 					// режим аналоговой сихронизации
		WORD SynchroAdChannel;  			// канал АЦП при аналоговой синхронизации
		SHORT SynchroAdPorog; 				// порог срабатывания АЦП при аналоговой синхронизации
		WORD ChannelsQuantity;				// число активных каналов
		WORD ControlTable[MAX_CONTROL_TABLE_LENGTH_E440];		// управляющая таблица с активными каналами
		double AdcRate;	  			  		// частота работы АЦП в кГц
		double InterKadrDelay;		  		// Межкадровая задержка в мс
		double KadrRate;					// частота кадра в кГц
		WORD AdcFifoBaseAddress;			// базовый адрес FIFO буфера АЦП
		WORD AdcFifoLength;					// длина FIFO буфера АЦП
		double AdcOffsetCoefs[ADC_CALIBR_COEFS_QUANTITY_E440];	// смещение	АЦП: 4диапазона
		double AdcScaleCoefs[ADC_CALIBR_COEFS_QUANTITY_E440];		// масштаб АЦП	: 4диапазона
	};

	// структура, задающая режим работы ЦАП для модуля E-440
	struct DAC_PARS_E440
	{
		BOOL DacEnabled;						// разрешение/запрещение работы ЦАП
		double DacRate;	  			  		// частота работы ЦАП в кГц
		WORD DacFifoBaseAddress;			// базовый адрес FIFO буфера ЦАП
		WORD DacFifoLength;					// длина FIFO буфера ЦАП
	};
	#pragma pack()

	// адрес начала сегмента блока данных в памяти программ
	const WORD DataBaseAddress_E440 = 0x30;
	// переменные штатного LBIOS для модуля E14-440 (раполагаются в памяти программ DSP)
	#define 	  	L_PROGRAM_BASE_ADDRESS_E440				(DataBaseAddress_E440 + 0x0)
	#define 	  	L_READY_E440 									(DataBaseAddress_E440 + 0x1)
	#define	  	L_TMODE1_E440 									(DataBaseAddress_E440 + 0x2)
	#define	  	L_TMODE2_E440 									(DataBaseAddress_E440 + 0x3)
	#define	  	L_TEST_LOAD_E440	 							(DataBaseAddress_E440 + 0x4)
	#define	  	L_COMMAND_E440	 			      			(DataBaseAddress_E440 + 0x5)

	#define		L_DAC_SCLK_DIV_E440							(DataBaseAddress_E440 + 0x7)
	#define		L_DAC_RATE_E440								(DataBaseAddress_E440 + 0x8)
	#define	  	L_ADC_RATE_E440  			      			(DataBaseAddress_E440 + 0x9)
	#define		L_ADC_ENABLED_E440	 						(DataBaseAddress_E440 + 0xA)
	#define		L_ADC_FIFO_BASE_ADDRESS_E440				(DataBaseAddress_E440 + 0xB)
	#define		L_CUR_ADC_FIFO_LENGTH_E440					(DataBaseAddress_E440 + 0xC)
	#define		L_ADC_FIFO_LENGTH_E440						(DataBaseAddress_E440 + 0xE)
	#define		L_CORRECTION_ENABLED_E440					(DataBaseAddress_E440 + 0xF)
	#define		L_LBIOS_VERSION_E440							(DataBaseAddress_E440 + 0x10)
	#define		L_ADC_SAMPLE_E440								(DataBaseAddress_E440 + 0x11)
	#define		L_ADC_CHANNEL_E440	 						(DataBaseAddress_E440 + 0x12)
	#define		L_INPUT_MODE_E440								(DataBaseAddress_E440 + 0x13)
	#define		L_SYNCHRO_AD_CHANNEL_E440					(DataBaseAddress_E440 + 0x16)
	#define		L_SYNCHRO_AD_POROG_E440						(DataBaseAddress_E440 + 0x17)
	#define		L_SYNCHRO_AD_MODE_E440 						(DataBaseAddress_E440 + 0x18)
	#define		L_SYNCHRO_AD_TYPE_E440 						(DataBaseAddress_E440 + 0x19)

	#define		L_CONTROL_TABLE_LENGHT_E440				(DataBaseAddress_E440 + 0x1B)
	#define		L_FIRST_SAMPLE_DELAY_E440					(DataBaseAddress_E440 + 0x1C)
	#define		L_INTER_KADR_DELAY_E440						(DataBaseAddress_E440 + 0x1D)

	#define		L_DAC_SAMPLE_E440								(DataBaseAddress_E440 + 0x20)
	#define		L_DAC_ENABLED_E440					 		(DataBaseAddress_E440 + 0x21)
	#define		L_DAC_FIFO_BASE_ADDRESS_E440				(DataBaseAddress_E440 + 0x22)
	#define		L_CUR_DAC_FIFO_LENGTH_E440					(DataBaseAddress_E440 + 0x24)
	#define		L_DAC_FIFO_LENGTH_E440						(DataBaseAddress_E440 + 0x25)

	#define		L_FLASH_ENABLED_E440 						(DataBaseAddress_E440 + 0x26)
	#define		L_FLASH_ADDRESS_E440 						(DataBaseAddress_E440 + 0x27)
	#define		L_FLASH_DATA_E440 							(DataBaseAddress_E440 + 0x28)

	#define		L_ENABLE_TTL_OUT_E440						(DataBaseAddress_E440 + 0x29)
	#define		L_TTL_OUT_E440									(DataBaseAddress_E440 + 0x2A)
	#define		L_TTL_IN_E440									(DataBaseAddress_E440 + 0x2B)

	#define		L_SCALE_E440									(DataBaseAddress_E440 + 0x30)
	#define		L_ZERO_E440										(DataBaseAddress_E440 + 0x34)

	#define		L_CONTROL_TABLE_E440							(0x80)

	#define		L_DSP_INFO_STUCTURE_E440					(0x200)

	//-----------------------------------------------------------------------------
	// интерфейс модуля E14-440
	//-----------------------------------------------------------------------------
	struct ILE440 : public ILUSBBASE
	{
		// функции работы с DSP
		virtual BOOL WINAPI RESET_MODULE(BYTE ResetFlag = INIT_E440) = 0;
		virtual BOOL WINAPI LOAD_MODULE(PCHAR const FileName = NULL) = 0;
		virtual BOOL WINAPI TEST_MODULE(void) = 0;
		virtual BOOL WINAPI SEND_COMMAND(WORD Command) = 0;

		// функции для работы с АЦП
		virtual BOOL WINAPI GET_ADC_PARS(ADC_PARS_E440 * const AdcPars) = 0;
		virtual BOOL WINAPI SET_ADC_PARS(ADC_PARS_E440 * const AdcPars) = 0;
		virtual BOOL WINAPI START_ADC(void) = 0;
		virtual BOOL WINAPI STOP_ADC(void) = 0;
		virtual BOOL WINAPI ADC_KADR(SHORT * const Data) = 0;
		virtual BOOL WINAPI ADC_SAMPLE(SHORT * const AdcData, WORD AdcChannel) = 0;
		virtual BOOL WINAPI ReadData(IO_REQUEST_LUSBAPI * const ReadRequest) = 0;

		// функции для работы с ЦАП
		virtual BOOL WINAPI GET_DAC_PARS(DAC_PARS_E440 * const DacPars) = 0;
		virtual BOOL WINAPI SET_DAC_PARS(DAC_PARS_E440 * const DacPars) = 0;
		virtual BOOL WINAPI START_DAC(void) = 0;
		virtual BOOL WINAPI STOP_DAC(void) = 0;
		virtual BOOL WINAPI WriteData(IO_REQUEST_LUSBAPI * const WriteRequest) = 0;
		virtual BOOL WINAPI DAC_SAMPLE(SHORT * const DacData, WORD DacChannel) = 0;

		// функции для работы с цифровыми линиями
		virtual BOOL WINAPI ENABLE_TTL_OUT(BOOL EnableTtlOut) = 0;
		virtual BOOL WINAPI TTL_IN(WORD * const TtlIn) = 0;
		virtual BOOL WINAPI TTL_OUT(WORD TtlOut) = 0;

		// функции для работы ППЗУ
		virtual BOOL WINAPI ENABLE_FLASH_WRITE(BOOL EnableFlashWrite) = 0;
		virtual BOOL WINAPI READ_FLASH_WORD(WORD FlashAddress, SHORT * const FlashWord) = 0;
		virtual BOOL WINAPI WRITE_FLASH_WORD(WORD FlashAddress, SHORT FlashWord) = 0;

		// функции для работы со служебной информацией из ППЗУ
		virtual BOOL WINAPI GET_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E440 * const ModuleDescription) = 0;
		virtual BOOL WINAPI SAVE_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E440 * const ModuleDescription) = 0;

		// функции для работы с памятью DSP
		virtual BOOL WINAPI PUT_LBIOS_WORD(WORD Address, SHORT Data) = 0;
		virtual BOOL WINAPI GET_LBIOS_WORD(WORD Address, SHORT * const Data) = 0;
		virtual BOOL WINAPI PUT_DM_WORD(WORD Address, SHORT Data) = 0;
		virtual BOOL WINAPI GET_DM_WORD(WORD Address, SHORT * const Data) = 0;
		virtual BOOL WINAPI PUT_PM_WORD(WORD Address, long Data) = 0;
		virtual BOOL WINAPI GET_PM_WORD(WORD Address, long * const Data) = 0;
		virtual BOOL WINAPI PUT_DM_ARRAY(WORD BaseAddress, WORD NPoints, SHORT * const Data) = 0;
		virtual BOOL WINAPI GET_DM_ARRAY(WORD BaseAddress, WORD NPoints, SHORT * const Data) = 0;
		virtual BOOL WINAPI PUT_PM_ARRAY(WORD BaseAddress, WORD NPoints, long * const Data) = 0;
		virtual BOOL WINAPI GET_PM_ARRAY(WORD BaseAddress, WORD NPoints, long * const Data) = 0;
	};




	// ==========================================================================
	// *************************** Модуль E20-10 ********************************
	// ==========================================================================
	// возможные режимы фиксации факта перегрузки входных каналов при сборе данных
	enum {	CLIPPING_OVERLOAD_E2010, MARKER_OVERLOAD_E2010, INVALID_OVERLOAD_E2010 };
	// доступные индексы источника импульса старта сбора данных
	enum {	INT_ADC_START_E2010, INT_ADC_START_WITH_TRANS_E2010, EXT_ADC_START_ON_RISING_EDGE_E2010, EXT_ADC_START_ON_FALLING_EDGE_E2010, INVALID_ADC_START_E2010 };
	// доступные индексы источника тактовых импульсов для АЦП
	enum {	INT_ADC_CLOCK_E2010, INT_ADC_CLOCK_WITH_TRANS_E2010, EXT_ADC_CLOCK_ON_RISING_EDGE_E2010, EXT_ADC_CLOCK_ON_FALLING_EDGE_E2010, INVALID_ADC_CLOCK_E2010 };
	// доступные индексы диапазонов входного напряжения модуля E20-10
	enum {	ADC_INPUT_RANGE_3000mV_E2010, ADC_INPUT_RANGE_1000mV_E2010, ADC_INPUT_RANGE_300mV_E2010, INVALID_ADC_INPUT_RANGE_E2010 };
	// возможные типы подключения входного тракта модуля E20-10
	enum {	ADC_INPUT_ZERO_E2010, ADC_INPUT_SIGNAL_E2010, INVALID_ADC_INPUT_E2010 };
	// доступные виды целостности собираемых с АЦП данных
	enum {	DATA_INTEGRITY_OK_E2010, DATA_INTEGRITY_BAD_E2010, INVALID_DATA_INTEGRITY_E2010 };
	// возможные опции наличия микросхемы ЦАП для модуля E20-10
	enum {	DAC_INACCESSIBLED_E2010, DAC_ACCESSIBLED_E2010, INVALID_DAC_OPTION_E2010 };
	// доступные индексы ревизий модуля E20-10
	enum {	REVISION_A_E2010, /*REVISION_B_E2010, */ INVALID_REVISION_E2010 };
	
	// константы для работы с модулем
	enum 	{
				ADC_CHANNELS_QUANTITY_E2010 = 0x4, MAX_CONTROL_TABLE_LENGTH_E2010 = 256,
				ADC_INPUT_RANGES_QUANTITY_E2010 = INVALID_ADC_INPUT_RANGE_E2010,
				ADC_INPUT_TYPES_QUANTITY_E2010 = INVALID_ADC_INPUT_E2010,
				ADC_CALIBR_COEFS_QUANTITY_E2010 = ADC_CHANNELS_QUANTITY_E2010 * ADC_INPUT_RANGES_QUANTITY_E2010,
				DAC_CHANNELS_QUANTITY_E2010 = 0x2, DAC_CALIBR_COEFS_QUANTITY_E2010 = DAC_CHANNELS_QUANTITY_E2010,
				TTL_LINES_QUANTITY_E2010 = 0x10,		// кол-во входных и выходных цифровых линий
				USER_FLASH_SIZE_E2010 = 0x200,  		// размер области пользовательского ППЗУ в байтах
				REVISIONS_QUANTITY_E2010 = INVALID_REVISION_E2010,		// кол-во ревизий (модификаций) модуля
				ADC_PLUS_OVERLOAD_MARKER = 0x5FFF,	// признак 'плюс' перегрузки отсчёта с АЦП
				ADC_MINUS_OVERLOAD_MARKER = 0xA000	// признак 'минус' перегрузки отсчёта с АЦП
			};

	// диапазоны входного напряжения АЦП в В
	const double ADC_INPUT_RANGES_E2010[ADC_INPUT_RANGES_QUANTITY_E2010] =
	{
		3.0, 1.0, 0.3
	};
	// диапазон выходного напряжения ЦАП в В
	const double DAC_OUTPUT_RANGE_E2010 = 5.0;
	// доступные ревизии модуля
	const BYTE REVISIONS_E2010[REVISIONS_QUANTITY_E2010] = { 'A' };

	#pragma pack(1)
	// структура с общей информацией об модуле E20-10
	struct MODULE_DESCRIPTION_E2010
	{
		MODULE_INFO_LUSBAPI     Module;		// общая информация о модуле
		INTERFACE_INFO_LUSBAPI  Interface;	// информация об интерфейсе
		MCU_INFO_LUSBAPI<MCU_VERSION_INFO_LUSBAPI>	Mcu;	// информация о микроконтроллере
		PLD_INFO_LUSBAPI        Pld;			// информация о ПЛИС
		ADC_INFO_LUSBAPI        Adc;			// информация о АЦП
		DAC_INFO_LUSBAPI        Dac;			// информация о ЦАП
		DIGITAL_IO_INFO_LUSBAPI DigitalIo;	// информация о цифровом вводе-выводе
	};
	// структура пользовательского ППЗУ
	struct USER_FLASH_E2010
	{
		BYTE Buffer[USER_FLASH_SIZE_E2010];
	};
	// структура с параметрами работы АЦП
	struct ADC_PARS_E2010
	{
		WORD StartSource;						// источник импульса начала сбора данных с АЦП (внутренний или внешний)
		WORD SynhroSource;					// источник тактовых импульсов запуска АЦП (внутренние или внешние)
		WORD OverloadMode;					// режим фиксации факта перегрузки входных каналов модуля
		WORD ChannelsQuantity;				// кол-во активных каналов
		WORD ControlTable[MAX_CONTROL_TABLE_LENGTH_E2010];			// управляющая таблица с активными логическими каналами
		double AdcRate;						// частота работы АЦП в кГц
		double InterKadrDelay;				// межкадровая задержка в мс
		double KadrRate;						// частота кадра в кГц
		WORD InputRange[ADC_CHANNELS_QUANTITY_E2010]; 	// индекс диапазона входного напряжения: 3.0В, 1.0В или 0.3В
		WORD InputSwitch[ADC_CHANNELS_QUANTITY_E2010];	// индекс типа подключения канала: земля или сигнал
	};
	#pragma pack()

	//-----------------------------------------------------------------------------
	// интерфейс модуля E20-10
	//-----------------------------------------------------------------------------
	struct ILE2010 : public ILUSBBASE
	{
		// загрузка ПЛИС модуля
		virtual BOOL WINAPI LOAD_MODULE(PCHAR const FileName = NULL) = 0;
		virtual BOOL WINAPI TEST_MODULE(void) = 0;
		// работа с АЦП
		virtual BOOL WINAPI GET_ADC_PARS(ADC_PARS_E2010 * const AdcPars) = 0;
		virtual BOOL WINAPI SET_ADC_PARS(ADC_PARS_E2010 * const AdcPars) = 0;
		virtual BOOL WINAPI START_ADC(void) = 0;
		virtual BOOL WINAPI STOP_ADC(void) = 0;
		virtual BOOL WINAPI CHECK_DATA_INTERGRITY(BYTE * const DataIntegrity) = 0;
		// функции потокового асинхронного чтения данных из модуля
		virtual BOOL WINAPI ReadData(IO_REQUEST_LUSBAPI * const ReadRequest) = 0;
		// однократная синхронная работа с ЦАП
		virtual BOOL WINAPI DAC_SAMPLE(SHORT * const DacData, WORD DacChannel) = 0;
		// работа с цифровыми линиями
		virtual BOOL WINAPI ENABLE_TTL_OUT(BOOL EnableTtlOut) = 0;
		virtual BOOL WINAPI TTL_IN (WORD * const TtlIn) = 0;
		virtual BOOL WINAPI TTL_OUT(WORD TtlOut) = 0;
		// функции для работы с пользовательской информацией ППЗУ
		virtual BOOL WINAPI ENABLE_FLASH_WRITE(BOOL IsUserFlashWriteEnabled) = 0;
		virtual BOOL WINAPI READ_FLASH_ARRAY(USER_FLASH_E2010 * const UserFlash) = 0;
		virtual BOOL WINAPI WRITE_FLASH_ARRAY(USER_FLASH_E2010 * const UserFlash) = 0;
		// информация о модуле
		virtual BOOL WINAPI GET_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E2010 * const ModuleDescription) = 0;
		virtual BOOL WINAPI SAVE_MODULE_DESCRIPTION(MODULE_DESCRIPTION_E2010 * const ModuleDescription) = 0;
	};

#endif

