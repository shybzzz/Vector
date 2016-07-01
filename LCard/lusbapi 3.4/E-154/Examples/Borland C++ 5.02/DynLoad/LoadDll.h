//---------------------------------------------------------------------------
#ifndef __LoadDllH
#define __LoadDllH
//---------------------------------------------------------------------------
#include <windows.h>
#include <winioctl.h>

typedef int (WINAPI *pGetDllVersion)(void);
typedef LPVOID (WINAPI *pCreateInstance)(char *);

class TLoadDll
{
	public:
		// коструктор
		TLoadDll(PCHAR DllName);
		// деструктор
		~TLoadDll();

		// получение адреса функции интерфейса модуля
		LPVOID WINAPI CallCreateLInstance(void);
		// получение адреса функции версии DLL
		LPVOID WINAPI CallGetDllVersion(void);
		// получение идентификатора библиотеки
		HINSTANCE WINAPI GetDllHinstance(void);


	private:
		// идентификатор модуля DLL
		HINSTANCE hDll;
};

#endif

