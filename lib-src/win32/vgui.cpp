//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <stdio.h>
#include "VGUI.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <stdarg.h>
#include <string.h>
#endif

namespace vgui
{
int vgui_dprintf(const char* format,...)
{
	char    buf[2048];
	va_list argList;

	va_start(argList,format);
	int ret=vsprintf(buf,format,argList);
	va_end(argList);

#ifdef _WIN32
	::OutputDebugString(buf);
#else
	fputs(buf, stderr);
#endif
	
	return ret;
}

int vgui_dprintf2(const char* format,...)
{
	char    buf[2048];
	va_list argList;
	static int ctr=0;

	sprintf(buf,"%d:",ctr++);

	va_start(argList,format);
	int ret=vsprintf(buf+strlen(buf),format,argList);
	va_end(argList);

#ifdef _WIN32
	::OutputDebugString(buf);
#else
	fputs(buf, stderr);
#endif
	return ret;
}
}
