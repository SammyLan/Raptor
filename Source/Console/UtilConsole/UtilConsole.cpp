#include "stdafx.h"

#include "UtilConsole.h"
#include "ConsoleWnd.h"

extern "C"  CONSOLE_API HRESULT InitConsole()
{
	CConsoleWnd * pConsolWnd = CConsoleWnd::CreateInstance();
	pConsolWnd->OnInit();
	return S_OK;
}

extern "C"  CONSOLE_API HRESULT ExitConsole()
{
	CConsoleWnd * pConsolWnd = CConsoleWnd::GetInstance();
	pConsolWnd->OnExit();
	CConsoleWnd::DestoryInstance();
	return S_OK;
}

BEGIN_NS(Util)
BEGIN_NS(Console)
HRESULT InitConsole()
{
	return ::InitConsole();
}

HRESULT ExitConsole()
{
	return ::ExitConsole();
}

BOOL WriteString(LPCTSTR lpszFormat, ...)
{
	va_list arglist;
	va_start(arglist, lpszFormat);
	CString str;
	str.FormatV(lpszFormat, arglist);
	va_end(arglist);
	CConsoleWnd * pConsolWnd = CConsoleWnd::GetInstance();
	return pConsolWnd->DoWriteString(str);
}

void WritePrompt(BOOL bCRL)
{
	CConsoleWnd * pConsolWnd = CConsoleWnd::GetInstance();
	pConsolWnd->WritePrompt(bCRL);
}

END_NS()
END_NS()