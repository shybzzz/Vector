//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("Synchro.res");
USEFORM("SynchroMain.cpp", MainForm);
USEUNIT("SynchroThread.cpp");
USELIB("Zoomer.lib");
USELIB("..\..\..\DLL\Lib\Borland\Lusbapi.lib");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "Пример для E14-154";
		Application->CreateForm(__classid(TMainForm), &MainForm);
                 Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
