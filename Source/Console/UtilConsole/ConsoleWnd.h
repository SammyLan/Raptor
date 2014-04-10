#pragma once
#include "MsgOnlyWnd.h"
#include <atlcore.h>
#include <memory>
using namespace std;

class CConsoleWnd:CMsgOnlyWnd
{
	typedef CMsgOnlyWnd BASE_CLASS;
	typedef CConsoleWnd THIS_CLASS;
public:
	static CConsoleWnd * CreateInstance();
	static CConsoleWnd * GetInstance();
	static void DestoryInstance();
	virtual BOOL OnInit();
	virtual BOOL OnExit();
	void WritePrompt(BOOL bCRLF);
	BOOL DoWriteString(const CString & str);
	BOOL WriteString(LPCTSTR lpszFormat, ...);
protected:
	CConsoleWnd();
	LRESULT OnMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ConsoleReadThreadProc();
	void ProcessCmd(CString const & strCmd);
	static BOOL WINAPI HandlerRoutine(  DWORD dwCtrlType);
private:
	HANDLE	hStdIn_;			//标准输入
	HANDLE	hStdOut_;			//标准输出
	HANDLE	hConsoleReadThread_;//读取标准输入线程
	CONSOLE_SCREEN_BUFFER_INFO csbiReadStartPos_;//输入的开始位置
	BOOL	bConsoleReadThreadCont_;	//读取线程是否要继续运行
	BOOL	bReading_;			//正在读
	static auto_ptr<CConsoleWnd> s_pConsolWnd;
};