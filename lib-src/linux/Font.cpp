//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "VGUI.h"
#include "VGUI_Font.h"
#include "VGUI_Dar.h"
#include "fileimage.h"
#include "vfontdata.h"
#include "utlrbtree.h"
#include "vgui_linux.h"

using namespace vgui;
typedef unsigned char uint8;

static int staticFontId=100;
static Dar<BaseFontPlat*> staticFontPlatDar;

namespace vgui
{
class FontPlat : public BaseFontPlat
{
protected:
	struct font_name_entry
	{
		char *m_OSSpecificName;
		uint8 m_cbOSSpecificName;
		char *m_pchFriendlyName;
	};

	bool ms_bSetFriendlyNameCacheLessFunc;
	CUtlRBTree<font_name_entry, int> m_FriendlyNameCache;

	static bool FontLessFunc( const font_name_entry &lhs, const font_name_entry &rhs )
	{
		return strcasecmp( rhs.m_pchFriendlyName, lhs.m_pchFriendlyName ) > 0;
	}

	virtual int getWide()
	{
		return m_iMaxCharWidth;
	}

	int bufSize[2];
	uchar* buf;

	VFontData m_BitmapFont;
	bool m_bBitmapFont;

	char m_szName[32];
	int m_iTall;
	int m_iWeight;
	int m_iFlags;
	bool m_bAntiAliased;
	bool m_bRotary;
	bool m_bAdditive;
	int m_iDropShadowOffset;
	bool m_bUnderlined;
	int m_iOutlineSize;
	int m_iHeight;
	int m_iMaxCharWidth;
	int m_iAscent;

	// abc widths
	struct abc_t
	{
		short b;
		char a;
		char c;
	};

	// On PC we cache char widths on demand when actually requested to minimize our use of the kernels 
	// paged pool (GDI may cache information about glyphs we have requested and take up lots of paged pool)
	struct abc_cache_t
	{
		wchar_t wch;
		abc_t abc;
	};
	CUtlRBTree<abc_cache_t, unsigned short> m_ExtendedABCWidthsCache;
	//static bool ExtendedABCWidthsCacheLessFunc(const abc_cache_t &lhs, const abc_cache_t &rhs);

	int m_iScanLines;
	int m_iBlur;
	float *m_pGaussianDistribution;

public:
	FontPlat(const char* name,int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol) : m_ExtendedABCWidthsCache(256, 0, &FontPlat::ExtendedABCWidthsCacheLessFunc)
	{
		m_bBitmapFont = false;

		strncpy( m_szName, name, sizeof( m_szName ));
		m_iTall = tall;
		m_iWeight = weight;
		m_iFlags = 0;
		m_bAntiAliased = false;
		m_bUnderlined = underline;
		m_iDropShadowOffset = 0;
		m_iOutlineSize = 0;
		m_iBlur = 0;
		m_iScanLines = 0;
		m_bRotary = false;
		m_bAdditive = false;
	}

	virtual ~FontPlat()
	{
	}

	static bool ExtendedABCWidthsCacheLessFunc(const abc_cache_t &lhs, const abc_cache_t &rhs)
	{
		return lhs.wch < rhs.wch;
	}

	virtual bool equals(const char* name,int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol)
	{
		if (!strcasecmp(name, m_szName) 
			&& m_iTall == tall
			&& m_iWeight == weight
			&& m_bUnderlined == underline)
			return true;

		return false;
	}

	void CreateFontList()
	{
	}

	virtual void getCharRGBA(int ch,int rgbaX,int rgbaY,int rgbaWide,int rgbaTall,uchar* rgba)
	{
	}

	virtual void getCharABCwide(int ch,int& a,int& b,int& c)
	{
		// look for it in the cache
		abc_cache_t finder = { (wchar_t)ch };

		unsigned short i = m_ExtendedABCWidthsCache.Find(finder);
		if (m_ExtendedABCWidthsCache.IsValidIndex(i))
		{
			a = m_ExtendedABCWidthsCache[i].abc.a;
			b = m_ExtendedABCWidthsCache[i].abc.b;
			c = m_ExtendedABCWidthsCache[i].abc.c;
			return;
		}
	}

	virtual int getTall()
	{
		return m_iHeight;
	}

	virtual void drawSetTextFont(SurfacePlat* plat)
	{
	}
};

class FontPlat_Bitmap : public BaseFontPlat
{
protected:
	virtual int getWide()
	{
		return m_FontData.m_BitmapCharWidth;
	}

	VFontData m_FontData;
	char *m_pName;

public:
	FontPlat_Bitmap()
	{
		m_pName=null;
	}

	virtual ~FontPlat_Bitmap()
	{
	}

	static FontPlat_Bitmap* Create(const char* name, FileImageStream* pStream)
	{
		FontPlat_Bitmap* pBitmap=new FontPlat_Bitmap();
		if(!pBitmap)
			return null;

		if(!LoadVFontDataFrom32BitTGA(pStream,&pBitmap->m_FontData))
		{
			delete pBitmap;
			return null;
		}

		pBitmap->m_pName=new char[strlen(name)+1];
		if(!pBitmap->m_pName)
		{
			delete pBitmap;
			return null;
		}

		strcpy(pBitmap->m_pName,name);
		return pBitmap;
	}

	virtual bool equals(const char* name,int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol)
	{
		return false;
	}

	virtual void getCharRGBA(int ch,int rgbaX,int rgbaY,int rgbaWide,int rgbaTall,uchar* rgba)
	{
		uchar* pSrcPos;
		uchar* pOutPos;
		int x,y,outX,outY;

		if(ch<0)
			ch=0;
		else if(ch>=256)
			ch=256;

		for(y=0;y<m_FontData.m_BitmapCharHeight;y++)
		{
			pSrcPos=&m_FontData.m_pBitmap[m_FontData.m_BitmapCharWidth*(ch+y*256)];
			for(x=0;x<m_FontData.m_BitmapCharWidth;x++)
			{
				outX=rgbaX+x;
				outY=rgbaY+y;
				if ((outX<rgbaWide)&&(outY<rgbaTall))
				{
					pOutPos=&rgba[(outY*rgbaWide+outX)*4];
					if(pSrcPos[x])
					{
						pOutPos[0]=pOutPos[1]=pOutPos[2]=pOutPos[3]=255;
					}
					else
					{
						pOutPos[0]=pOutPos[1]=pOutPos[2]=pOutPos[3]=0;
					}
				}
			}
		}
	}

	virtual void getCharABCwide(int ch,int& a,int& b,int& c)
	{
		if(ch<0)
			ch=0;
		else if(ch>255)
			ch=255;

		a=c=0;
		b=m_FontData.m_CharWidths[ch]+1;
	}

	virtual int getTall()
	{
		return m_FontData.m_BitmapCharHeight;
	}
	virtual void drawSetTextFont(SurfacePlat* plat)
	{
	}
};
};

Font::Font(const char* name,int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol)
{
	init(name,null,0,wide,tall,rotation,weight,italic,underline,strikeout,symbol);
}

Font::Font(const char* name,void *pFileData,int fileDataLen, int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol)
{
	init(name,pFileData,fileDataLen,wide,tall,rotation,weight,italic,underline,strikeout,symbol);
}

void Font::init(const char* name,void *pFileData,int fileDataLen, int tall,int wide,float rotation,int weight,bool italic,bool underline,bool strikeout,bool symbol)
{
	FontPlat_Bitmap*pBitmapFont;

	_name=strdup(name);
	_id=-1;

	if(pFileData)
	{
		FileImageStream_Memory memStream(pFileData,fileDataLen);
		pBitmapFont=FontPlat_Bitmap::Create(name,&memStream);
		if(pBitmapFont)
		{
			_plat=pBitmapFont;
			staticFontPlatDar.addElement(_plat);
			_id=staticFontId++;
		}
	}
	else
	{
		_plat=null;
		for(int i=0;i<staticFontPlatDar.getCount();i++)
		{
			if(staticFontPlatDar[i]->equals(name,tall,wide,rotation,weight,italic,underline,strikeout,symbol))
			{
				_plat=staticFontPlatDar[i];
				break;
			}
		}
		if(_plat==null)
		{
			_plat=new FontPlat(name,tall,wide,rotation,weight,italic,underline,strikeout,symbol);
			staticFontPlatDar.addElement(_plat);
			_id=staticFontId++;
		}
	}
}

void Font::getCharRGBA(int ch,int rgbaX,int rgbaY,int rgbaWide,int rgbaTall,uchar* rgba)
{
	_plat->getCharRGBA(ch,rgbaX,rgbaY,rgbaWide,rgbaTall,rgba);
}

void Font::getCharABCwide(int ch,int& a,int& b,int& c)
{
	_plat->getCharABCwide(ch,a,b,c);
}

int Font::getTall()
{
	return _plat->getTall();
}

int Font::getWide()
{
	return _plat->getWide();
}


namespace vgui
{
void Font_Reset()
{
	staticFontPlatDar.removeAll();
}
}
