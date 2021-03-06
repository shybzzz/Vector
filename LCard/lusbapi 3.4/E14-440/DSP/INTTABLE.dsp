{ *********************************************************************	}
{  ��� � ⠡��楩 ���뢠��� ��� ����� E-440   									}
{ *********************************************************************	}
#include "const.h"

.MODULE/ABS=0x0/SEG=int_pm_user       Intr_Table;
.EXTERNAL StartBios, CommandProcessing, Irq0Handler, Irq1Handler;

{ ********************************************************************* }
{ !!!!!!!!!!!!!!!!!! ������� ��������� ���������� !!!!!!!!!!!!!!!!!!!!! }
{ *********************************************************************	}
{ 	RESET 																					}
	JUMP StartBios; NOP; NOP; NOP;

{ *********************************************************************	}
{ 	��ࠡ��稪 ���뢠��� IRQ2 (���)		                             		}
{ *********************************************************************	}
	JUMP(I7); NOP; NOP; NOP;

{ *********************************************************************	}
{ 	��ࠡ��稪 ���뢠��� IRQL1 														}
{ *********************************************************************	}
	RTI; NOP; NOP; NOP;

{ *********************************************************************	}
{ 	��ࠡ��稪 ���뢠��� IRQL0                                				}
{ *********************************************************************	}
	RTI; NOP; NOP; NOP;

{ *********************************************************************	}
{ 	��ࠡ��稪 ���뢠��� SPORT0 Transmit 											}
{ *********************************************************************	}
	SB = 0x1; RTI; NOP; NOP;

{ *********************************************************************	}
{ 	��ࠡ��稪 ���뢠��� SPORT0 Receive											}
{ *********************************************************************	}
	RTI; NOP; NOP; NOP;

{ *********************************************************************	}
{ ��ࠡ��稪 ���뢠��� IRQE �� �� �१ AVR (�������)						}
{ ॣ���� SE - �ਧ��� ������ ����ࠡ�⠭��� �������							}
{ *********************************************************************	}
	SE = 0x1; RTI; NOP; NOP;

{ *********************************************************************	}
{ 	��ࠡ��稪 ���뢠��� BDMA															}
{ *********************************************************************	}
	RTI; NOP; NOP; NOP;

{ *********************************************************************	}
{	��ࠡ��稪 ���뢠��� SPORT1 Transmit/IRQ1 (����. ᨭ�஭�����)		}
{ *********************************************************************	}
	JUMP Irq1Handler; NOP; NOP; NOP;

{ *********************************************************************	}
{ ��ࠡ��稪 ���뢠��� SPORT1 Receive/IRQ0 (��⭮ �� �ᯮ������)		}
{ *********************************************************************	}
	JUMP Irq0Handler; NOP; NOP; NOP;

{ *********************************************************************	}
{	��ࠡ��稪 ���뢠��� Timer														}
{ *********************************************************************	}
	RTI; NOP; NOP; NOP;

{ *********************************************************************	}
{	��ࠡ��稪 ���뢠��� Powerdown													}
{ *********************************************************************	}
	RTI; NOP; NOP; NOP;

.ENDMOD;
