//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

namespace vgui
{
class SurfacePlat
{
public:
#ifdef _WIN32
	HWND    hwnd;
	HDC     hdc;
	HDC     hwndDC;
	HDC	  textureDC;
	HGLRC   hglrc;
	HRGN    clipRgn;
	HBITMAP bitmap;
#endif
	int     bitmapSize[2];
	int     restoreInfo[4];
	bool    isFullscreen;
	int     fullscreenInfo[3];
};

class BaseFontPlat
{
public:
	virtual ~BaseFontPlat() {}
	virtual bool equals(const char* name,int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol)=0;
	virtual void getCharRGBA(int ch,int rgbaX,int rgbaY,int rgbaWide,int rgbaTall,uchar* rgba)=0;
	virtual void getCharABCwide(int ch,int& a,int& b,int& c)=0;
	virtual int getTall()=0;
	virtual void drawSetTextFont(SurfacePlat* plat)=0;
};
}
