{ ---------------------------------------------------------------------	}
{	Функции для работы с пользовательским ППЗУ									}
{ ---------------------------------------------------------------------	}

{ *********************************************************************	}
{ Разрешение/запрещение процедуры записи в пользовательское ППЗУ			}
{ *********************************************************************	}
Enable_Flash_Write_cmd:
	GetPm(SR0, EnableFlashAddr);		{ считаем флаг	разрешения ППЗУ		}

	SET FL0, RESET FL1;					{ установим chip select ППЗУ			}
												{ клоки ППЗУ в ноль						}
	CNTR = 0x9;
	DO EnableFlashLoop UNTIL CE;
		AR = TSTBIT 15 OF SR0;
		IF NE SET FL2;    				{ данные в ППЗУ							}
		IF EQ RESET FL2;

		CALL Delay3mks;
		TOGGLE FL1;
		CALL Delay3mks;
		TOGGLE FL1;
EnableFlashLoop:	SR = LSHIFT SR0 BY 1 (LO);

	RESET FL0, RESET FL1;		{ сбросим chip select  клоки ППЗУ в ноль	}

{ завершим команду																		}	
	JUMP EndOfCommand;

{ *********************************************************************	}
{ Команда чтения слова из пользовательского ППЗУ								}
{ *********************************************************************	}
Read_Flash_Word_cmd:
{ читаем слово из пользовательского ППЗУ											}
	CALL Read_Flash_Word;
{ завершим команду																		}	
	JUMP EndOfCommand;

{ *********************************************************************	}
{ Процедура записи слова в пользовательское ППЗУ								}
{ *********************************************************************	}
Write_Flash_Word_cmd:
	GetPm(SR0, FlashAddressAddr);		{ считаем адрес ячейки ППЗУ			}

	SET FL0, RESET FL1;					{ установим chip select ППЗУ			}
												{ клоки ППЗУ в ноль						}
{ передадим адрес ячейки ППЗУ															}
	CNTR = 0x9;
	DO WriteFlashLoop1 UNTIL CE;
		AR = TSTBIT 15 OF SR0;
		IF NE SET FL2;    				{ данные в ППЗУ							}
		IF EQ RESET FL2;

		CALL Delay3mks;
		TOGGLE FL1;
		CALL Delay3mks;
		TOGGLE FL1;
WriteFlashLoop1: 	SR = LSHIFT SR0 BY 1 (LO);

	GetPm(SR0, FlashDataAddr);			{ считаем данные для записи в ППЗУ	}

{ запишем значение в заданную ячейку ППЗУ											}
	CNTR = 0x10;
	DO WriteFlashLoop2 UNTIL CE;
		AR = TSTBIT 15 OF SR0;
		IF NE SET FL2;    				{ данные в ППЗУ							}
		IF EQ RESET FL2;

		CALL Delay3mks;
		TOGGLE FL1;
		CALL Delay3mks;
		TOGGLE FL1;
WriteFlashLoop2:	SR = LSHIFT SR0 BY 1 (LO);

	RESET FL0, RESET FL1;		{ сбросим chip select  клоки ППЗУ в ноль	}

{ завершим команду																		}	
	JUMP EndOfCommand;

{ *********************************************************************	}
{ Процедура чтения слова из пользовательского ППЗУ								}
{ *********************************************************************	}
Read_Flash_Word:
	GetPm(SR0, FlashAddressAddr);		{ считаем адрес ячейки ППЗУ			}

	SET FL0, RESET FL1;					{ установим chip select ППЗУ			}
												{ клоки ППЗУ в ноль						}
{ передадим адрес ячейки ППЗУ															}
	CNTR = 0x9;
	DO ReadFlashLoop1 UNTIL CE;
		AR = TSTBIT 15 OF SR0;
		IF NE SET FL2;    				{ данные в ППЗУ							}
		IF EQ RESET FL2;

		CALL Delay3mks;
		TOGGLE FL1;
		CALL Delay3mks;
		TOGGLE FL1;
ReadFlashLoop1:	SR = LSHIFT SR0 BY 1 (LO);

	CALL Delay3mks;

{ считаем значение из заданной ячейки ППЗУ										}
	SR0 = 0x0;
	CNTR = 0x10;
	DO ReadFlashLoop2 UNTIL CE;
		SR = LSHIFT SR0 BY 1 (LO);

		CALL Delay3mks;
		TOGGLE FL1;
		CALL Delay3mks;
		TOGGLE FL1;

		AR = 0x0;
		IF NOT FLAG_IN JUMP ReadFlashLoop2;
		AR = 0x1;
ReadFlashLoop2:	SR = SR OR LSHIFT AR BY 0 (LO);

	PutPm(SR0, FlashDataAddr);	{ сохраним полученное слово					}

	RESET FL0, RESET FL1;		{ сбросим chip select  клоки ППЗУ в ноль	}

{ спокойно выходим из функции															}
	RTS;

{ *********************************************************************	}
{ Задержка на 3.0 мкс																	}
{ *********************************************************************	}
Delay3mks:
	CNTR = 145;
	DO Delay3mksLoop UNTIL CE;
Delay3mksLoop: NOP;
	RTS;
