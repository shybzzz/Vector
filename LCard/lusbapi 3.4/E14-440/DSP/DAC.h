{ ---------------------------------------------------------------------	}
{	Функции для работы с ЦАП															}
{ ---------------------------------------------------------------------	}

{ *********************************************************************	}
{ Команда запуска ЦАП																	}
{ *********************************************************************	}
Start_Dac_cmd:
{ SPORT0 - disable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1}
	AR = 0x0;
	DM(Sys_Ctrl_Reg) = AR;			{ 0x3FFF - System Control Register 		}

{ проинициализируем необходимые указатели ЦАП									}
	CALL InitDacPointers;

{ установим требуемую частоту вывода отсчетов на ЦАП							}
	GetPm(AR, DacRateAddr);
	DM(Sport0_Rfsdiv) = AR; { 0x3FF4 - Receive Frame Sync Divide Modulus }

{ разрешаем автобуфферизацию для передачи данных по SPORT0					}
{  - буфер данных для ЦАП: I1, M1, L1								 				}
{ отключаем CLOCKOUT																		}
	AR = 0x4282;					{ 0100 0010 1000 0010							}
	DM(Sport0_Autobuf_Ctrl) = AR;

{ установим флаг работы ЦАП															}
	PutValPm(AR, 0x1, DacEnableAddr);

{ SPORT0 - enable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1	}
	AR = 0x1000;					 	{ 0001 0000 0000 0000 						}
	DM(Sys_Ctrl_Reg) = AR;      	{ 0x3FFF - System Control Register 		}

{ штатное завершение выполнения команды											}
	JUMP EndOfCommand;

{ *********************************************************************	}
{ Команда останова ЦАП																	}
{ *********************************************************************	}
Stop_Dac_cmd:
{ SPORT0 - disable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1}
	AR = 0x0;
	DM(Sys_Ctrl_Reg)=AR;				{ 0x3FFF - System Control Register 		}

{ запрещаем автобуфферизацию SPORT0 и отключаем CLOCKOUT						}
	AR = 0x4000; DM(Sport0_Autobuf_Ctrl) = AR;

{ установим требуемую частоту вывода отсчетов на ЦАП							}
	GetPm(AR, DacRateAddr);
	DM(Sport0_Rfsdiv) = AR; { 0x3FF4 - Receive Frame Sync Divide Modulus }

{ SPORT0 - enable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1	}
	AR = 0x1000;					 	{ 0001 0000 0000 0000 						}
	DM(Sys_Ctrl_Reg) = AR;      	{ 0x3FFF - System Control Register 		}

{ сбросим флаг работы ЦАП																}
	PutValPm(AR, 0x0, DacEnableAddr);

{ штатное завершение выполнения команды											}
	JUMP EndOfCommand;

{ *********************************************************************	}
{ Команда выдачи однократного отсчета на ЦАП (если возможно)				}
{ *********************************************************************	}
Dac_Sample_cmd:
{ а работа ЦАП разрешена?																}
	GetPm(AR, DacEnableAddr); NONE = PASS AR;
	IF NE JUMP EndOfCommand; 	{ никакой выдачи на ЦАП - просто выйдем	}

{ SPORT0 - disable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1}
	AR = 0x0;
	DM(Sys_Ctrl_Reg) = AR;			{ 0x3FFF - System Control Register 		}

{ запрещаем автобуфферизацию SPORT0 и отключаем CLOCKOUT						}
	AR = 0x4000; DM(Sport0_Autobuf_Ctrl) = AR;

{ установим максимальную частоту вывода отсчетов на ЦАП (125 кГц)			}
	AR = 31;
	DM(Sport0_Rfsdiv) = AR; { 0x3FF4 - Receive Frame Sync Divide Modulus }

{ SPORT0 - enable, SPORT1 - disable, SPORT1 - FI, FO, IRQ0, IRQ1, SCLK1	}
	AR = 0x1000;					 	{ 0001 0000 0000 0000 						}
	DM(Sys_Ctrl_Reg) = AR;      	{ 0x3FFF - System Control Register 		}

{ сбросим флажок завершения однократной выдачи отсчета на ЦАП				}
	SB = 0x0;
{ очистим запросы, кроме IRQE(команды) и IRQ2 (АЦП)							}
	IFC = 0x6F; NOP;

{ получим однократный отсчет для вывода на ЦАП									}
	GetPm(AR, DacSampleAddr); TX0 = AR;

{ временно запретим все прерывания													}
	DIS INTS;							
{ дополнительно размаскируем прерывания SPORT0 Transmit(ЦАП)				}
	GetPm(AR, ImaskValueAddr);
	AR = SETBIT 6 OF AR;				{ бит прерывания SPORT0 Transmit			}
	PutPm(AR, ImaskValueAddr);
	IMASK = AR; NOP;
{ разрешим все размаскированные прерывания										}
	ENA INTS;							

{ спокойно выходим из функции															}
	RTS;
