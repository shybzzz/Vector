#ifndef _ZOOM_H
#define _ZOOM_H

#include <windows.h>

struct ZoomParam
{
   int GridX,GridY;
   COLORREF BgColor;
   COLORREF ZoomColor;
   COLORREF CursorColor;
   COLORREF ZoomGridColor;
   void Set(int nx, int ny, COLORREF bc, COLORREF zc, COLORREF cc, COLORREF zg)
   {
      GridX=nx; GridY=ny;
      BgColor=bc; ZoomColor=zc; CursorColor=cc; ZoomGridColor=zg;
   }
};


struct ZoomScale
{
   float Xmin;
   float Xmax;
   float Ymin;
   float Ymax;
   float Rmax;
   float Rmin;
   int Width;     // How much digits in scale subscription
   char *Format;  // Format string
   void Set( float xmi=-1, float xma=1, float ymi=-1, float yma=1,
             float rmi=-1, float rma=1, int w=10, char *f="%-10.1f" )
   {
      Xmin=xmi; Xmax=xma; Ymin=ymi; Ymax=yma;
      Rmin=rmi; Rmax=rma; Width=w; Format=f;
   }
};

struct OSCInfo
{
   int CIndex;
   int IndexMin, IndexMax;
   ZoomScale Scale;
};


class TZoomer;

extern "C" TZoomer* WINAPI _export CreateOSC(ZoomParam *zp=0);

class TZoomer
{
   public:
      TZoomer();  // Constructor of object
      ~TZoomer(); // Destructor of object
      void PlotGrid(HDC dc, RECT rect, int xcnt, int ycnt);
      void PlotCursor(HDC dc, RECT rect, POINT point);
      void PlotBorder(HDC dc, RECT Area, HPEN Pen1, HPEN Pen2);
      void PlotZeroLine(HDC dc, RECT rect, ZoomScale scale);
      void PlotMinMax(HDC dc, int Size);
      void PlotSimple(HDC dc, int Size);

      virtual int GetInfo(OSCInfo *);
      virtual int SetParameters(int Prop, ZoomParam *zp=0);
      virtual void SetDataI(int *Data, int Size, int Chan, int ActCh, ZoomScale *zs, COLORREF *rgb=0);
      virtual void MakeFont(HWND hWnd, int H, char *Name=0);
      virtual int OnPaint(HDC dc, RECT &rect, int);
      virtual int OnMouseMove(RECT &rect, WPARAM wParam, LPARAM lParam);
      virtual int OnButtonDown(RECT &rect, WPARAM wParam, LPARAM lParam, int Mode = 0);

      virtual void Release();

   protected:
      TZoomer *zw;

   protected:
      int *a;        // point to data
      int DSize;     // size of data
      int NCh;       // channels in data
      int ActiveCh;  // active channel
      POINT MCoo;    // mouse coordinates
      POINT *p;      // tmp array for drawing
      HDC MDC;       // memory context to draw
      HBITMAP bmp;
      HRGN rgn;
      COLORREF *rgbcur;
      HFONT hFont;   // font descriptor
      HBRUSH hBgBrush, hZoomBrush, hSelectBrush;
      HPEN hWhite, hBlack, hGray, hLtGray, hCursor, hDottedLtGray;
      ZoomScale ZS;  // scale for plotting
      ZoomParam ZP;  // parameters of zoomer
      int XGrid;
      int YGrid;
      int NeedUpdate;
      int ZoomProp;   // Zoom proportion
      /////////////////////////////////////////
      RECT WA;
      RECT WA0;
      RECT CA; // area for cursor coo plots
      POINT CAT; //place to print coo
      RECT YA; // area for Y scale plot
      RECT XA; // area for X
      RECT PA; // area for GRAPH plot ALL depends from FONT size
      RECT PA0; // bounded to 0
      RECT ZA;  // zoomer area
      RECT ZAN; // normalized;
};
#endif

