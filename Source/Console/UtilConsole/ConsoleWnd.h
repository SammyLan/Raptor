#pragma once
#include "MsgOnlyWnd.h"


class CConsoleWnd:CMsgOnlyWnd
{
	typedef CMsgOnlyWnd BASE_CLASS;
	typedef CConsoleWnd THIS_CLASS;
public:
	CConsoleWnd();
	virtual BOOL OnInit();
	virtual BOOL OnExit();
	void WritePrompt(BOOL bCRLF);
	void DoWriteString(const CString & str);
	void WriteString(LPCTSTR lpszFormat, ...);
protected:
	LRESULT OnMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ConsoleReadThreadProc();
	void ProcessCmd(CString const & strCmd);
private:
	HANDLE	hStdIn_;			//标准输入
	HANDLE	hStdOut_;			//标准输出
	HANDLE	hConsoleReadThread_;//读取标准输入线程
	CONSOLE_SCREEN_BUFFER_INFO csbiReadStartPos_;//输入的开始位置
	BOOL	bConsoleReadThreadCont_;	//读取线程是否要继续运行
	BOOL	bReading_;			//正在读
};