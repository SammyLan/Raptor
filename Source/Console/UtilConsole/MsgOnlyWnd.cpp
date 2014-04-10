#include "stdafx.h"
#include "MsgOnlyWnd.h"

namespace
{
	BOOL s_bInitWndClass = FALSE;
	WNDCLASSEX s_wndClassEx;
	TCHAR s_szMessageOnlyWindowName[] = _T("Raptor_CMsgOnlyWnd");
	
}

BOOL CMsgOnlyWnd::InitWndClass()
{
	if (!s_bInitWndClass)
	{
		memset(&s_wndClassEx,0,sizeof(WNDCLASSEX));
		s_wndClassEx.cbSize = sizeof(WNDCLASSEX);
		s_wndClassEx.hInstance     = GetModuleHandle(NULL);
		s_wndClassEx.lpszClassName = s_szMessageOnlyWindowName;
		s_wndClassEx.lpfnWndProc   = &CMsgOnlyWnd::OnMessage;
		s_wndClassEx.hbrBackground = NULL;
		s_wndClassEx.cbWndExtra    = 0;
		if(RegisterClassEx(&s_wndClassEx))
		{
			s_bInitWndClass = TRUE;
		}
	}
	return s_bInitWndClass;
}



LRESULT CMsgOnlyWnd::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMsgOnlyWnd * pThis = reinterpret_cast<CMsgOnlyWnd *>(GetWindowLongPtr(hWnd,GWL_USERDATA));
	if (NULL != pThis)
	{
		pThis->OnMessageCallback(hWnd,uMsg,wParam,lParam);
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


CMsgOnlyWnd::CMsgOnlyWnd()
:hwnd_(NULL)
{	
}

CMsgOnlyWnd::~CMsgOnlyWnd() 
{
	ATLASSERT(NULL == hwnd_);

}

BOOL CMsgOnlyWnd::OnInit()
{
	if (InitWndClass())
	{
		hwnd_ = CreateWindowEx(0, s_szMessageOnlyWindowName, s_szMessageOnlyWindowName, WS_OVERLAPPED, 0, 0, 0, 0, 
			HWND_MESSAGE, NULL, s_wndClassEx.hInstance, NULL);
		SetWindowLongPtr(hwnd_, GWL_USERDATA,reinterpret_cast<LONG_PTR>(this));

	}
	return hwnd_ != NULL;
}

BOOL CMsgOnlyWnd::OnExit()
{
	if (hwnd_ != NULL)
	{
		DestroyWindow(hwnd_);
		hwnd_ =NULL;
	}
	return TRUE;
}

BOOL CMsgOnlyWnd::PostMessage(UINT Msg,WPARAM wParam,LPARAM lParam)
{
	return ::PostMessage(hwnd_,Msg,wParam,lParam);
}

LRESULT CMsgOnlyWnd::SendMessage(UINT Msg,WPARAM wParam,LPARAM lParam)
{
	return ::SendMessage(hwnd_,Msg,wParam,lParam);
}