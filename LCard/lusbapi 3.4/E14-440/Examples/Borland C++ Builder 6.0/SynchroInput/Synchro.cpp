//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("Synchro.res");
USEFORM("SynchroMain.cpp", MainForm);
USELIB("Lusbapi.lib");
USEUNIT("SynchroThread.cpp");
USELIB("Zoomer.lib");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "������ ��� E14-440";
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
