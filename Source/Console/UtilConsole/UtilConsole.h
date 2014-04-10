#pragma once
#include "UtilConsoleDef.h"
#include "UtilDef.h"

BEGIN_NS(Util)
BEGIN_NS(Console)

CONSOLE_API HRESULT InitConsole();
CONSOLE_API HRESULT ExitConsole();
CONSOLE_API BOOL	WriteString(LPCTSTR lpszFormat, ...);
CONSOLE_API void	WritePrompt(BOOL bCRL);

END_NS()
END_NS()

