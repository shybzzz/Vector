{ ********************************************************************* }
{  Основная программа для работы c модулем E14-440               			}
{ ********************************************************************* }
#include "const.h"

.MODULE/ABS=ProgaramBaseAddress/SEG=int_pm_user             main_program;
.ENTRY StartBios, Irq1Handler, CommandProcessing;

#include "var.h"

{ After DSP RESET we appear to be here                                  }
StartBios:
	DIS SEC_REG;  						{ перейдем на первый банк регистров		}

{ I4, M4, L4 используются для доступа к переменным LBIOS						}
	M4 = 0x0; L4 = 0x0;
	M7 = 0x0; L7 = 0x0;

{ запишем в соответствующие переменные необходимые значения					}
	PutValPm(AR, 0x5555, TMode1Addr);
	PutValPm(AR, 0xAAAA, TMode2Addr);

{ сформируем текущую версию LBIOS													}
/*	AR = LBIOS_VERMAJOR; SR = LSHIFT AR BY 8 (LO);
	AR = LBIOS_VERMINOR; SR = SR OR LSHIFT AR BY 0 (LO);
	PutPm(SR0,LbiosVersionAddr);*/

{ Set Wait State for ALL External Devices                   				}
	AR = 0x0001;						{ 0000 0000 0000 0001 						}
	DM(Dm_Wait_Reg) = AR;			{ 0x3FFE - Waitstate Control Register 	}

{ SPORT0 - disable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1}
	AR = 0x0000;
	DM(Sys_Ctrl_Reg)=AR;				{ 0x3FFF - System Control Register 		}

{ *********************************************************************	}
{ 	Set SPORT0 for transmit digital codes in DAC                    	 	}
{ 	SCLK and Transmit Frame - internal, word = 16 bits                	}
{ 	Serial Clock Divide Modulus 														}
	GetPm(AR, Sclk0DivAddr);		{ SCLK0 period = 250 ns						}
	DM(Sport0_Sclkdiv) = AR;		{ 0x3FF5 - Serial Clock Divide Modulus	}

{ Receive Frame Sync Divide Modulus 												}
	GetPm(AR, DacRateAddr); { Определяет частоту вывода отсчетов с ЦАП	}
	DM(Sport0_Rfsdiv) = AR; { 0x3FF4 - Receive Frame Sync Divide Modulus }

{ запрещаем автобуфферизацию SPORT0 и отключаем CLOCKOUT						}
	AR = 0x4000; DM(Sport0_Autobuf_Ctrl) = AR;

{ Control word for SPORT0 : SCLK - internal 										}
{ low level, alternate internal receive frame on each word(16 bit) 		}
{ low level, alternate external transmit frame on each word(16 bit)		}
	AR = 0x7DCF;						{ 0111 1101 1100 1111						}
	DM(Sport0_Ctrl_Reg) = AR;		{ 0x3FF6 - SPORT0 Control Register		}
{ *********************************************************************	}

{ *********************************************************************	}
{ * Set SPORT1 for start of ADC chip	                           	 *	}
{ Serial Clock Divide Modulus 														}
	GetPm(AR, AdcRateAddr);		{ частота запуска АЦП							}
	DM(Sport1_Sclkdiv) = AR;	{ 0x3FF1 - Serial Clock Divide Modulus		}

{ Receive Frame Sync Divide Modulus 												}
	AR = 0xF;					{ RFS period = 16 SCLK	 							}
	DM(Sport1_Rfsdiv) = AR;	{ 0x3FF0 - Receive Frame Sync Divide Modulus	}

{ Control word for SPORT1 : SCLK - external 										}
{ high level, alternate internal receive frame on each word(16 bit)		}
{ high level, alternate internal transmit frame on each word(16 bit)		}
	AR = 0x3F1F; 						{ 0011 1111 0001 1111 						}
	DM(Sport1_Ctrl_Reg) = AR;		{ 0x3FF2 - SPORT1 Control Register		}
{ *********************************************************************	}

{ IRQ0, IRQ1, IRQ2 - negative front, disable Interrupt Nesting 			}
	ICNTL = 0x07;

{ очистим запросы на все прерывания													}
	IFC = 0xFF; NOP; NOP;

{ установим флаг FO в низкое состояние	- разрешение загрузки ЦАП			}
	RESET FLAG_OUT;

{ установим флаги FLx в исходное состояние										}
	RESET FL0, RESET FL1, RESET FL2;

{ PF0 в '1' для перевода TTL DSP OUT в третье состояние						}
{ PF2 в '1' и PF6 в '0' для внутреннего тактирования АЦП без трансляции	}
{ PF5 в '0' для записи номера канала в промежуточный буфер					}
	AR = 0x05; DM(Prog_Flag_Data) = AR;

{ установим флаги PFx: PF0-PF3,PF5 и PF6 - выходные, PF4 и PF7 - входные}
	AR = 0x6F; DM(Prog_Flag_Comp_Sel_Ctrl) = AR;

{ у нас по жизни всегда только ADSP-2185, т.е. есть внутреняя память 	}
	DMOVLAY = 0x0; PMOVLAY = 0x0;
{ включим дробный режим умножителя													}
	DIS M_MODE;

{ проинициализируем необходимые указатели и переменные						}
	CALL InitAdcPointers;
	CALL InitDacPointers;

{ обнулим регистр TX0 (данные для ЦАП)												}
	TX0 = 0x0;

{ SPORT0 - enable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1	}
	AR = 0x1000;					 	{ 0001 0000 0000 0000 						}
	DM(Sys_Ctrl_Reg) = AR;      	{ 0x3FFF - System Control Register 		}

{ если нужно - проверим наличие вторичного питания модуля					}
	CALL WaitForSecondaryPowerOn;

{ дадим знать основной программе, что плата полностью готова к работе	}
	PutValPm(AR, 0x1, ReadyAddr);

{ если запрещена работа АЦП, то клоки SCLK вкючать не будем					}
	GetPm(AR, AdcEnableAddr); NONE = PASS AR;
	IF EQ JUMP MainLoopEntry;

{ включим клоки, т.е.сделаем SCLK внутренним										}
	AR = 0x7F1F; 						{ 0111 1111 0001 1111 						}
	DM(Sport1_Ctrl_Reg) = AR;		{ 0x3FF2 - SPORT1 Control Register		}

{ =====================================================================	}
{ основной цикл работы АЦП	и/или ЦАП												}
{ =====================================================================	}
MainLoopEntry:
{ разрешим необходимые прерывания 													}
	GetPm(AR,ImaskValueAddr);
	IMASK = AR; NOP;

MainLoop:
	CALL PutAcdData;
	CALL GetDacData;
	CALL CheckDacSample;
	CALL CommandProcessing;

	JUMP MainLoop;

{ *********************************************************************	}
{ Процедура передачи готовых данных из FIFO буфера АЦП в РС					}
{ *********************************************************************	}
PutAcdData:
{ а работа АЦП разрешена?																}
	GetPm(AR, AdcEnableAddr); NONE = PASS AR;
	IF EQ RTS;								{ если нет, то просто выйдем			}

{ а не пора ли посылать данные в РС?												}
	AR = I3;									{ получим 'голову' FIFO АЦП			}
	AY0 = I2;  								{ получим 'хвост' FIFO АЦП				}
	AR = AR-AY0;
	GetPm(AY0, CurAdcFifoLengthAddr);{ текущая длина FIFO буфера АЦП		}
	IF LT AR = AR+AY0;		{ теперь в AR истинная разница между I3 и I2	}
	GetPm(AY0, AdcFifoLengthOver2Addr);	{ половина длины FIFO буфера АЦП	}
	NONE = AR-AY0;   						{ сравним ее с половиной длины		}
	IF LT RTS;								{ если меньше, то выйдем				}

{ если данные готовы, то сгенерим прерывание в AVR от имени АЦП			}
	AR = DM(Prog_Flag_Data); AR = SETBIT 1 OF AR; DM(Prog_Flag_Data) = AR;
	NOP; NOP; NOP; NOP; NOP; NOP;
	NOP; NOP; NOP; NOP; NOP; NOP;
	AR = CLRBIT 1 OF AR; DM(Prog_Flag_Data) = AR;
{ модифицируем указатель на хвост FIFO буфера АЦ								}
	MODIFY(I2, M2);

{ спокойно выходим из функции															}
	RTS;

{ *********************************************************************	}
{ Процедура приема новых данных для буфера ЦАП из РС							}
{ *********************************************************************	}
GetDacData:
{ а работа ЦАП разрешена?																}
	GetPm(AR, DacEnableAddr); NONE = PASS AR;
	IF EQ RTS;								{ если нет, то просто выйдем			}

{ а не пора ли принимать данные из РС?												}
	AR = I1;									{ получим 'голову' FIFO ЦАП			}
	AY0 = I0;  								{ получим 'хвост' FIFO ЦАП				}
	AR = AR - AY0;
	GetPm(AY0, CurDacFifoLengthAddr);{ текущая длина FIFO буфера ЦАП		}
	IF LT AR = AR + AY0;		{ теперь в AR истинная разница между I3 и I2	}
	GetPm(AY0, DacFifoLengthOver2Addr);	{ половина длины FIFO буфера ЦАП	}
	NONE = AR - AY0; 						{ сравним ее с половиной длины		}
	IF LT RTS;								{ если меньше, то выйдем				}

{ если данные готовы, то сгенерим прерывание в AVR от имени ЦАП			}
	AR = DM(Prog_Flag_Data); AR = SETBIT 3 OF AR; DM(Prog_Flag_Data) = AR;
	NOP; NOP; NOP; NOP; NOP; NOP;
	NOP; NOP; NOP; NOP; NOP; NOP;
	AR= CLRBIT 3 OF AR; DM(Prog_Flag_Data) = AR;
{ модифицируем указатель на хвост FIFO буфера ЦАП								}
	MODIFY(I0, M0);

{ спокойно выходим из функции															}
	RTS;

{ *********************************************************************	}
{ Проверка окончания однократного вывода на ЦАП									}
{ *********************************************************************	}
CheckDacSample:
{ флажок завершения выдачи отсчета на ЦАП											}
	AR = SB; NONE = PASS AR; IF EQ RTS;

{ запретим все прерывания																}
	DIS INTS;
{ разрешим прерывания IRQE(команды)													}
	GetPm(AR, ImaskValueAddr);
	AR = CLRBIT 6 OF AR;			{ запретим прерывания SPORT0 Transmit(ЦАП)}
	PutPm(AR, ImaskValueAddr);
	IMASK = AR; NOP;
{ разрешим все прерывания																}
	ENA INTS;

{ сбросим флажок завершения однократной выдачи отсчета на ЦАП				}
	SB = 0x0;
{ задержечка на 4 мкс																	}
	CALL Delay2mks; CALL Delay2mks;

{ теперь выключим SPORT0 (ЦАП)														}
{ SPORT0 - disable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1}
	AR = 0x0;
	DM(Sys_Ctrl_Reg)=AR;		  		{ 0x3FFF - System Control Register 		}
{ штатное завершение выполнения команды											}
	JUMP EndOfCommand;

{ *********************************************************************	}
{ Обработчик прерываний IRQ1 (внеш. синхронизация)								}
{ *********************************************************************	}
Irq1Handler:

/*
!!!!!!!!!
сделать разные обработчики для внутренних и внешних клоков АЦП
!!!!!!!!!
*/

	ENA SEC_REG;			{ перейдем на второй банк регистров					}

{ разрешим прерывания IRQE(команды) и IRQ2(АЦП)									}
	I7 = ImaskValueAddr; AR = PM(I7, M7);
	AR = CLRBIT 2 OF AR;				{ запретим прерывания IRQ1(внеш.синхр.)}
	AR = SETBIT 9 OF AR;				{ разрешим прерывания IRQ2(АЦП)			}
	PM(I7, M7) = AR; DM(TempImask) = AR;

{ положим в стэк регистр IMASK с новым значением								}
	DIS INTS;				{ запретим все прерывания								}
	POP STS;
	IMASK = DM(TempImask);
	PUSH STS;
	IMASK = 0x0;
	ENA SEC_REG;			{ перейдем на второй банк регистров					}
	ENA INTS;				{ разрешим все прерывания								}

{ установим флажок, что разрешена работа АЦП										}
	AR = 0x1; I7 = AdcEnableAddr; PM(I7, M7) = AR;
{ восстановим стандартный обработчик прерываний IRQ2(АЦП)					}
	I7 = ^GetAdData;
{ очистим запросы, кроме IRQE(команды) и SPORT0 Transmit (ЦАП)				}
	IFC = 0xAF; /*NOP;*/

{ отладка																					}
/*ar=dm(Test0); ar=setbit 0 of ar; dm(Test0)=ar; io(ttl_out) = ar;*/

{ запустим АЦП, включив клоки SCLK1, т.е. сделаем их внутренним			}
	AR = 0x7F1F; 						{ 0111 1111 0001 1111 						}
	DM(Sport1_Ctrl_Reg) = AR; 		{ 0x3FF2 - SPORT1 Control Register		}

{ выходим из обработчика прерываний													}
	RTI;

{ ********************************************************************  }
{ выявим номер команды																	}
{ ********************************************************************  }
CommandProcessing:
{ проверим есть ли необработанная команда?										}
	AR = SE; NONE = PASS AR;
	IF EQ RTS; 							{ если нет, то просто выйдем				}

{ сбросим флажок необработанной команды											}
	SE = 0x0;

	GetPm(AR, CommandAddr);
	AR = PASS AR;
	IF EQ JUMP Test_cmd;       				{ номер команды 0					}
	AR = AR-0x1;
	IF EQ JUMP Enable_Flash_Write_cmd;		{ команда номер 1					}
	AR = AR-0x1;
	IF EQ JUMP Read_Flash_Word_cmd; 			{ команда номер 2					}
	AR = AR-0x1;
	IF EQ JUMP Write_Flash_Word_cmd;			{ команда номер 3					}
	AR = AR-0x1;
	IF EQ JUMP Start_Adc_cmd;			 		{ номер команды 4					}
	AR = AR-0x1;
	IF EQ JUMP Stop_Adc_cmd;			 		{ номер команды 5					}
	AR = AR-0x1;
	IF EQ JUMP Adc_Kadr_cmd;  					{ номер команды 6					}
	AR = AR-0x1;
	IF EQ JUMP Adc_Sample_cmd;  				{ номер команды 7					}
	AR = AR-0x1;
	IF EQ JUMP Start_Dac_cmd;			  		{ номер команды 8					}
	AR = AR-0x1;
	IF EQ JUMP Stop_Dac_cmd;			  		{ номер команды 9					}
	AR = AR-0x1;
	IF EQ JUMP Dac_Sample_cmd;			  		{ номер команды 10				}
	AR = AR-0x1;
	IF EQ JUMP Enable_TTL_Out_cmd; 			{ номер команды 11  				}
	AR = AR-0x1;
	IF EQ JUMP Get_TTL_cmd;   					{ номер команды 12				}
	AR = AR-0x1;
	IF EQ JUMP Put_TTL_cmd;   					{ номер команды 13				}
	AR = AR-0x1;
	IF EQ JUMP Nakadka_Mode_cmd;				{ номер команды 14				}

{ ошибочный номер команды																}
ErrorEndOfCommand:
	PutValPm(AR, 0xFFFF, CommandAddr);

{ спокойно выходим из функции															}
	RTS;			   								{ продолжим работу				}

{ *********************************************************************	}
{ Тестовая команда.                                                   	}
{ После выхода в ячейке	памяти данных TestLoadVar								}
{  													должно быть число 0xAA55		}
{ *********************************************************************	}
Test_cmd:
	PutValPm(AR,0xAA55,TestLoadVarAddr);

{ *********************************************************************	}
{ Выход из обработчика команды IRQE													}
{ После выхода в ячейке	памяти данных Command должно быть число 0x0		}
{ *********************************************************************	}
EndOfCommand:
	PutValPm(AR, 0x0, CommandAddr);

{ спокойно выходим из функции															}
	RTS;			   								{ продолжим работу				}

{ *********************************************************************	}
{	Установка указателей	для АЦП														}
{ *********************************************************************	}
InitAdcPointers:
{ установим параметры для корректировки входных данных						}
	I5 = ^ControlTable; M5 = 0x1; L5 = 0x0;

	I6 = ^CalibrationArray; M6 = 0x1;
	GetPm(AR, ControlTableLenghtAddr); 	{ длина управляющей таблицы		}
	SR = LSHIFT AR BY 1 (LO);				{ умножим на два						}
	L6 = SR0;

	GetPm(AR, IsCorrectionEnabledAddr);
	NONE = PASS AR;			{ разрешена ли корректировка входных данных	}
	IF EQ JUMP NoAdcDataCorrection;{ если да, готовим соответствующий массив}

	GetPm(AR, ControlTableLenghtAddr); 	{ длина управляющей таблицы		}
	CNTR = AR;
	DO CtrlLoop UNTIL CE;
		AR = PM(I5, M5);
		SR = LSHIFT AR BY -6 (LO);
		M4 = SR0;

{	 	Set ZeroOffset																		}
		I4 = ^ZeroOffset;
		MODIFY(I4, M4);
		AR = PM(I4, M4);
		PM(I6, M6) = AR;

{	 	Set ScaleFactor																	}
		I4 = ^ScaleFactor;
		MODIFY(I4, M4);
		AR = PM(I4, M4);
		PM(I6, M6) = AR;
CtrlLoop: NOP;

ContinueAdcPar:
{ установим требуемые размеры FIFO буфера АЦП									}
	CALL CheckAdcFifoLimits;

{ половина длины FIFO буфера АЦП														}
	GetPm(AR, CurAdcFifoLengthAddr);
	SR = LSHIFT AR BY -1 (LO);
	PutPm(SR0, AdcFifoLengthOver2Addr);

{ Set ADC Fifo Parameters (указатель на 'голову' FIFO АЦП)					}
	GetPm(AR, AdcFifoBaseAddressAddr);
	I3 = AR;
	M3 = 0x0;									{ первый отсчет левый				}
	GetPm(AR, CurAdcFifoLengthAddr);
	L3 = AR;

{ указатель на 'хвост' FIFO буфера АЦП												}
	I2 = I3;
{ в M2 должна быть половина длины FIFO буфера АЦП								}
	GetPm(AR, AdcFifoLengthOver2Addr);
	M2 = AR;
	L2 = L3;

{ Set pointer to ControlTable															}
	I5 = ^ControlTable;
	M5 = 0x1;
	GetPm(AR, ControlTableLenghtAddr); 	{ длина управляющей таблицы		}
	L5 = AR;

{ массив коэффициентов для корректировки смещения нуля и масштаба АЦП	}
	I6 = ^CalibrationArray;
	M6 = 0x1;
	GetPm(AR, ControlTableLenghtAddr); 	{ длина управляющей таблицы		}
	SR = LSHIFT AR BY 1 (LO);				{ умножим на два						}
	L6 = SR0;

{ установим стандартный обработчик прерываний IRQ2(АЦП)						}
	I7 = ^GetAdData;
{ по месту метки Irq2Lab0 пропишем код RTI										}
	I4 = ^Irq2Lab0; AR = 0x0A00; PX = 0x1F; PM(I4, M4) = AR;
{ проверим есть ли межкадровая задержка											}
	GetPm(AR, InterKadrDelayAddr); NONE = PASS AR;
	IF EQ RTS;									{ если нет, то тихонечко выйдем	}

{ ТАК! У нас есть межкадровая задержка												}
{ теперь по месту метки Irq2Lab0 надо прописать код AF=AF-0x1				}
	I4 = ^Irq2Lab0; AR = 0x2710; PX = 0x0F; PM(I4, M4) = AR;
{ а по месту метки Irq2Lab1 надо прописать код AF= PASS AX1					}
	I4 = ^Irq2Lab1; AR = 0x2679; PX = 0x0F; PM(I4, M4) = AR;

	ENA SEC_REG;						{ перейдем на второй банк регистров		}
	GetPm(AX0, ControlTableLenghtAddr);	{ AX0 - кол-во активных каналов 	}
	AF = PASS AX0;								{ AF - текущий счетчик прерываний}
	GetPm(AX1, InterKadrDelayAddr);		{ AX1 - межкадровая задержка		}
	DIS SEC_REG;					  	{ перейдем на первый банк регистров		}

{ спокойно выходим из функции															}
	RTS;

{ *********************************************************************	}
{ Если корректировка не нужна, готовим соответствующий массив				}
{ *********************************************************************	}
NoAdcDataCorrection:
	GetPm(AR, ControlTableLenghtAddr); 	{ длина управляющей таблицы		}
	CNTR = AR;
	DO CtrlLoop1 UNTIL CE;
{	 	Set ZeroOffset																		}
		AR = 0x0;
		PM(I6, M6)=AR;

{	 	Set ScaleFactor																	}
		AR = 0x8000;
		PM(I6, M6)=AR;

CtrlLoop1: 	NOP;

	JUMP ContinueAdcPar;

{ *********************************************************************	}
{ Установка первого канала для работы АЦП											}
{ *********************************************************************	}
SetAdcFirstChannel:
{ зададим логические каналы для следующих двух отсчетов						}
{ PF5 в ноль - запись в промежуточный регистр каналов							}
	AR = DM(Prog_Flag_Data); AR = CLRBIT 5 OF AR; DM(Prog_Flag_Data) = AR;

{ запись первого канала																	}
	AR = PM(I5, M5); IO(SET_ADC_CHANNEL) = AR;

{ PF5 в единицу - перезапись в выходной регистр каналов						}
	AR = DM(Prog_Flag_Data); AR = SETBIT 5 OF AR; DM(Prog_Flag_Data) = AR;
{ PF5 в ноль - запись в промежуточный регистр каналов							}
	AR = DM(Prog_Flag_Data); AR = CLRBIT 5 OF AR; DM(Prog_Flag_Data) = AR;

{ запись второго канала																	}
	AR = PM(I5, M5); IO(SET_ADC_CHANNEL) = AR;

{ спокойно выходим из функции															}
	RTS;

{ *********************************************************************	}
{ Установка указателей для ЦАП														}
{ *********************************************************************	}
InitDacPointers:
{ установим требуемые размеры FIFO буфера ЦАП									}
	CALL CheckDacFifoLimits;

{ половина длины FIFO буфера ЦАП														}
	GetPm(AR, CurDacFifoLengthAddr);
	SR = LSHIFT AR BY -1 (LO);
	PutPm(SR0, DacFifoLengthOver2Addr);

{ Set DAC Fifo Parameters (указатель на 'голову' FIFO ЦАП)					}
	GetPm(AR, DacFifoBaseAddressAddr);
	I1 = AR;
	M1 = 0x1;
	GetPm(AR, CurDacFifoLengthAddr);
	L1 = AR;

{ указатель на 'хвост' FIFO буфера ЦАП												}
	I0 = I1;
{ в M0 должна быть половина длины FIFO буфера ЦАП								}
	GetPm(AR, DacFifoLengthOver2Addr);
	M0 = AR;
	L0 = L1;

{ сбросим флажок завершения однократной выдачи отсчета на ЦАП				}
	SB = 0x0;

{ спокойно выходим из функции															}
	RTS;

{ ********************************************************************  }
{ проверим наличие вторичного питания модуля										}
{ ********************************************************************  }
WaitForSecondaryPowerOn:
{ зачитаем ревизию модуля																}
	PutValPm(AR, 0xC400, FlashAddressAddr); CALL Read_Flash_Word;
	AY0 = 0X00FF; AR = SR0 AND AY0;
{ ревизия модуля должна быть 'F' или выше											}
	AY0 = 0x0046;/*Rev.F*/ NONE = AR - AY0; IF LT RTS;
{ ждём окончания установки вторичного питания модуля							}
SecondaryPowerOnLoop:	
	AR = DM(Prog_Flag_Data); AR = TSTBIT 7 OF AR; 
	IF EQ JUMP SecondaryPowerOnLoop;

{ спокойно выходим из функции															}
	RTS;


{ ********************************************************************  }
{ задержка установление аналогового тракта										}
{ ********************************************************************  }
SettlingDelay:
	GetPm(AR, FirstSampleDelayAddr);
	CNTR = AR;
	DO SettlingDelayLoop UNTIL CE;
SettlingDelayLoop: NOP;

{ спокойно выходим из функции															}
	RTS;

{ ********************************************************************  }
{ задержка на 2.0 мкс 																	}
{ ********************************************************************  }
Delay2mks:
	CNTR = 100;
	DO Delay2mksLoop UNTIL CE;
Delay2mksLoop: NOP;

{ спокойно выходим из функции															}
	RTS;

#include "adc.h"
#include "dac.h"
#include "synchro.h"
#include "fifo.h"
#include "flash.h"
#include "ttl.h"
#include "naladka.h"

.ENDMOD;
