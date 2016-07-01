{ значения адресов управляющих регистров DSP									}
.const  	Sys_Ctrl_Reg					=	0x3FFF;
.const  	Dm_Wait_Reg						=	0x3FFE;
.const  	Tperiod_Reg						=	0x3FFD;
.const  	Tcount_Reg						=	0x3FFC;
.const  	Tscale_Reg						=	0x3FFB;
.const  	Sport1_Ctrl_Reg				= 	0x3FF2;
.const  	Sport1_Sclkdiv					= 	0x3FF1;
.const  	Sport1_Rfsdiv					=	0x3FF0;
.const  	Sport1_Autobuf_Ctrl			= 	0x3FEF;
.const  	Sport0_Ctrl_Reg				= 	0x3FF6;
.const  	Sport0_Sclkdiv					= 	0x3FF5;
.const  	Sport0_Rfsdiv					=	0x3FF4;
.const  	Sport0_Autobuf_Ctrl			=	0x3FF3;
.const 	Prog_Flag_Comp_Sel_Ctrl    =	0x3FE6;
.const 	Prog_Flag_Data             =	0x3FE5;
.const 	BDMA_Word_Count            =	0x3FE4;
.const 	BDMA_Control               =	0x3FE3;
.const 	BDMA_External_Address      =	0x3FE2;
.const 	BDMA_Internal_Address      =	0x3FE1;

{ константы расположения LBIOS в памяти программ							}
.const 	ProgaramBaseAddress      	=	0x400;
.const 	VariableBaseAddress    		=	0x30;
.const	ProgaramBaseAddressValAddr	= 	VariableBaseAddress + 0x0;
.const	ReadyAddr						= 	VariableBaseAddress + 0x1;
.const	TMode1Addr						= 	VariableBaseAddress + 0x2;
.const	TMode2Addr                 = 	VariableBaseAddress + 0x3;
.const	TestLoadVarAddr            = 	VariableBaseAddress + 0x4;
.const	CommandAddr						=	VariableBaseAddress + 0x5;
.const	ImaskValueAddr					=	VariableBaseAddress + 0x6;
.const	Sclk0DivAddr					=	VariableBaseAddress + 0x7;
.const	DacRateAddr						=	VariableBaseAddress + 0x8;
.const	AdcRateAddr						=	VariableBaseAddress + 0x9;
.const	AdcEnableAddr					=	VariableBaseAddress + 0xA;
.const	AdcFifoBaseAddressAddr		=	VariableBaseAddress + 0xB;
.const	CurAdcFifoLengthAddr			=	VariableBaseAddress + 0xC;
.const	AdcFifoLengthOver2Addr		=	VariableBaseAddress + 0xD;
.const	AdcFifoLengthAddr				=	VariableBaseAddress + 0xE;
.const	IsCorrectionEnabledAddr		=	VariableBaseAddress + 0xF;
/*.const	LbiosVersionAddr				=	VariableBaseAddress + 0x10;*/
.const	AdcSampleAddr					=	VariableBaseAddress + 0x11;
.const	AdcSampleChannelAddr			=	VariableBaseAddress + 0x12;
.const	InputModeAddr					=	VariableBaseAddress + 0x13;
.const	IsTtlKadrSynchroAddr			=	VariableBaseAddress + 0x14;
.const	SynchroAdFlagAddr				=	VariableBaseAddress + 0x15;
.const	SynchroAdChannelAddr			=	VariableBaseAddress + 0x16;
.const	SynchroAdPorogAddr			=	VariableBaseAddress + 0x17;
.const	SynchroAdModeAddr				=	VariableBaseAddress + 0x18;
.const	SynchroAdTypeAddr				=	VariableBaseAddress + 0x19;
.const	SynchroAdStepAddr				=	VariableBaseAddress + 0x1A;
.const	ControlTableLenghtAddr		=	VariableBaseAddress + 0x1B;
.const	FirstSampleDelayAddr			=	VariableBaseAddress + 0x1C;
.const	InterKadrDelayAddr			=	VariableBaseAddress + 0x1D;
.const	AdcChannelAddr					=	VariableBaseAddress + 0x1E;
.const	CorrectableAdcChannelAddr	=	VariableBaseAddress + 0x1F;

.const	DacSampleAddr					=	VariableBaseAddress + 0x20;
.const	DacEnableAddr					=	VariableBaseAddress + 0x21;
.const	DacFifoBaseAddressAddr		=	VariableBaseAddress + 0x22;
.const	DacFifoLengthOver2Addr		=	VariableBaseAddress + 0x23;
.const	CurDacFifoLengthAddr			=	VariableBaseAddress + 0x24;
.const	DacFifoLengthAddr				=	VariableBaseAddress + 0x25;
.const	EnableFlashAddr				=	VariableBaseAddress + 0x26;
.const	FlashAddressAddr				=	VariableBaseAddress + 0x27;
.const	FlashDataAddr					=	VariableBaseAddress + 0x28;

.const	EnableTtlOutAddr				=	VariableBaseAddress + 0x29;
.const	TtlOutAddr						=	VariableBaseAddress + 0x2A;
.const	TtlInAddr						=	VariableBaseAddress + 0x2B;

.const	AdcClockSourceAddr			=	VariableBaseAddress + 0x2F;

.const	ScaleFactorAddr				=	VariableBaseAddress + 0x30;
.const	ZeroOffsetAddr					=	VariableBaseAddress + 0x34;

.const	NaladkaTestNumberAddr		=	VariableBaseAddress + 0x38;
.const	NaladkaIrq0CounterAddr		=	VariableBaseAddress + 0x39;

{ диапазон адресов от 0x3A до 0x80 - свободен								}

.const	ControlTableAddr				=	0x80;		{ массив на 128 слов	}
.const	CalibrationArrayAddr			=	0x100;	{ массив на 256 слов	}
.const	DspInfoStructAddr				=	0x200;	{ массив на 256 слов	}


{ текущая версия LBIOS																}
/*.const	LBIOS_VERMAJOR 				=	3;*/   	{ старший байт версии	}
/*.const	LBIOS_VERMINOR 				=	0;*/		{ младший байт версии	}

{ регистры со стороны DSP															}
.const TTL_IN								=	0x0;
.const TTL_OUT								=	0x0;
.const READ_ADC							=	0x1;
.const SET_ADC_CHANNEL					=	0x1;
