//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("unit1.cpp", Form1);
USEFORM("Unit2.cpp", Form2);
USEFORM("unit4.cpp", Form4);
//---------------------------------------------------------------------------
#include "unit1.h"
#include "..\\dll\\lusbapi.h"
extern TForm1 *Form1;
ILE140 *m1;
ADC_PARS_E140 adc_pars;
MODULE_DESCR_E140 md;
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
 try{
    m1=static_cast<ILE140*>(CreateInstance("e140"));
    if(m1==NULL) throw Exception("Не могу создать экземпляр интерфейса ILE140");
    //
    try{
        Application->Initialize();
        Application->Title="E140DEMO";
        Application->CreateForm(__classid(TForm1), &Form1);
         Application->Run();
    } catch(Exception &exception) {
        Application->ShowException(&exception);
    }
    //
    m1->ReleaseLDevice();
 } catch(Exception &exception) {
    Application->ShowException(&exception);
 }
 return 0;
}
//---------------------------------------------------------------------------

