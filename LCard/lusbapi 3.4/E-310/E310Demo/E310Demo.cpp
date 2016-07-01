//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MainForm.cpp", MainForm);
USEFORM("GeneratorFrame.cpp", E310GeneratorFrame); /* TFrame: File Type */
USEFORM("About.cpp", AboutProgramPanel);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "Генератор E-310";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TAboutProgramPanel), &AboutProgramPanel);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
