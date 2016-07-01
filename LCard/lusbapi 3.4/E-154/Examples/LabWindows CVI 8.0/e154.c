#include <windows.h>
#include "toolbox.h"
#include <analysis.h>
#include <utility.h>
#include <rs232.h>
#include <ansi_c.h>
#include <cvirte.h>		/* Needed if linking in external compiler; harmless otherwise */
#include <userint.h>
#include <winioctl.h>
#include "test.h"
#include "bulkioct.h"


	#pragma pack(1)
	// структура с информацией об модуле E-154
#include "LusbapiTypes.h"   

	struct MODULE_DESCRIPTION_E154
	{
		struct MODULE_INFO_LUSBAPI     Module;		// общая информация о модуле
		struct INTERFACE_INFO_LUSBAPI  Interface;	// информация об используемом интерфейсе
		struct MCU_INFO_LUSBAPI		Mcu;	// информация о микроконтроллере
		struct ADC_INFO_LUSBAPI        Adc;			// информация о АЦП
		struct DAC_INFO_LUSBAPI        Dac;			// информация о ЦАП
		struct DIGITAL_IO_INFO_LUSBAPI DigitalIo;	// информация о цифровом вводе-выводе
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
	
	

#include "wlusbapi.h"

#define N_POINTS 5000
#define  LUSB_RESULT_OK  (10000)

int E154InitOk;	
extern LPVOID pModule;
extern HANDLE ModuleHandle;
extern char ModuleName[32];
int CloseE154(void);
int ShowSerialNumber(void);

struct MODULE_DESCRIPTION_E154 md;

int InitE154()
{
int i;

E154InitOk=FALSE;

CloseE154();
pModule=WE154CreateInstance();

	if(!pModule) return 1;  
		{
		for(i = 0x0; i < 128; i++) if(WE154OpenLDevice(pModule, i)) break;
		// что-нибудь обнаружили?
		if(i == 128)  return 1;
		  else
		  {
		  ModuleHandle=WE154GetModuleHandle(pModule);
	  	  if(ModuleHandle == INVALID_HANDLE_VALUE) return 1;
		    else
				{
				if(!WE154GetModuleName(pModule, ModuleName)) return 1;
				  else 
				  	{
					if(strcmp(ModuleName, "E154")) return 1;
						else  
							{ 
								E154InitOk=TRUE; 
								return 0;  
							}
				  	}
				}
		  }
	
		}
		
return 0;
}

int CloseE154(void)
{
E154InitOk=FALSE;

		if(pModule)
			{
			// освободим интерфейс модуля
			if(!WE154ReleaseLDevice(pModule));
			pModule = NULL;
			}
return 0;		
}


int TestLinkE154(void)
{
int i;

if(E154InitOk==FALSE) return 1;

if(!WE154GetModuleName(pModule, ModuleName)) { CloseE154(); return 1;}
  else 
	  	{
		if(strcmp(ModuleName, "E154"))  { CloseE154(); return 1;}
			else  return 0;  
	  	}
		
return 0;
}

