#pragma once
#include <Windows.h>

class CMsgOnlyWnd
{
public:
	BOOL PostMessage(UINT Msg,WPARAM wParam,LPARAM lParam);
	LRESULT SendMessage(UINT Msg,WPARAM wParam,LPARAM lParam);
protected:
	CMsgOnlyWnd();
	virtual ~CMsgOnlyWnd() = 0;
	virtual BOOL OnInit();
	virtual BOOL OnExit();
	virtual LRESULT OnMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
private:
	static BOOL InitWndClass();
	static LRESULT CALLBACK OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:	
	HWND hwnd_;		
};
