#include "stdafx.h"
#include <memory>
#include "UtilConsole.h"
#include "ConsoleWnd.h"

using namespace std;
namespace
{
	auto_ptr<CConsoleWnd> pConsolWnd = NULL;
}
 HRESULT InitConsole()
{
	pConsolWnd.reset(new CConsoleWnd());
	pConsolWnd->OnInit();
	return S_OK;
}

HRESULT ExitConsole()
{
	pConsolWnd->OnExit();
	pConsolWnd.reset(NULL);
	return S_OK;
}