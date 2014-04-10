#include "stdafx.h"
#include <locale.h>
#include "ConsoleWnd.h"
#include "RaptThreadPool.h"
namespace 
{
	TCHAR s_strPrompt[] = _T(":>");					///< 命令行提示符
	DWORD WM_CONSOLE_MESSAGE = WM_USER + 100;

	VOID WINAPI APCDummy(ULONG_PTR dwParam)
	{
	}
}
CConsoleWnd::CConsoleWnd()
:hStdIn_(NULL)
,hStdOut_(NULL)
,bReading_(FALSE)
,bConsoleReadThreadCont_(TRUE)
{
	setlocale(LC_ALL, "chs");
}
BOOL CConsoleWnd::OnInit()
{
	BOOL bRet = BASE_CLASS::OnInit();
	if (!bRet)
	{
		return FALSE;
	}
	bRet = FALSE;
	do
	{
		if (AllocConsole() == FALSE)
		{
			break;
		}
		if (NULL == ::freopen("CONIN$","r",stdin))
		{
			break;
		}

		if (NULL == ::freopen("CONOUT$","w",stdout))
		{
			break;
		}

		::SetConsoleTitle(_T("IMConsole"));
		hStdIn_= GetStdHandle(STD_INPUT_HANDLE);
		hStdOut_ = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
		if (!::SetConsoleMode(hStdIn_, dwMode))
		{
			break;
		}
		::SetConsoleTextAttribute(hStdOut_, FOREGROUND_GREEN | FOREGROUND_INTENSITY);

		hConsoleReadThread_ = Raptor::Util::HRTXCreateThread(this,&THIS_CLASS::ConsoleReadThreadProc);
		WritePrompt(FALSE);
		bRet = TRUE;
	}while (FALSE);
	return bRet;
}

BOOL CConsoleWnd::OnExit()
{
	bConsoleReadThreadCont_ = FALSE;
	CloseHandle(hStdIn_);
	if(WaitForSingleObject(hConsoleReadThread_,500) == WAIT_TIMEOUT)
	{
		QueueUserAPC(APCDummy,hConsoleReadThread_,NULL);
		WaitForSingleObject(hConsoleReadThread_,INFINITE);
	}

	CloseHandle(hConsoleReadThread_);
	hConsoleReadThread_ = NULL;

	FreeConsole();
	return BASE_CLASS::OnExit();
}

void CConsoleWnd::ProcessCmd(CString const & strCmd)
{	
	Sleep(10000);
	WriteString(strCmd);
}

LRESULT CConsoleWnd::OnMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CONSOLE_MESSAGE)
	{
		TCHAR * pBuf = (TCHAR *) lParam;
		ProcessCmd(pBuf);		
		WritePrompt(FALSE);
		delete [] pBuf;
	}
	return S_OK;
}


void CConsoleWnd::WritePrompt(BOOL bCRLF)
{
	if (bCRLF)
	{
		WriteString(_T("\r\n%s"), s_strPrompt);
	}
	else
	{
		DoWriteString(s_strPrompt);
	}
}

void CConsoleWnd::WriteString(LPCTSTR lpszFormat, ...)
{
	va_list arglist;
	va_start(arglist, lpszFormat);
	CString str;
	str.FormatV(lpszFormat, arglist);
	va_end(arglist);
	DoWriteString(str);
}

void CConsoleWnd::DoWriteString(const CString & str)
{
	/// -# 如果处于输入状态,备份输入开始位置到当前输入位置所在行的数据
	CHAR_INFO *pBackupBlock = NULL;
	COORD crdBackupBufSize = {0};
	BOOL bResult = FALSE;
	COORD curPos;

	if (bReading_)
	{		
		CONSOLE_SCREEN_BUFFER_INFO csbiStartPos = csbiReadStartPos_;
		CONSOLE_SCREEN_BUFFER_INFO csbiCur;
		if (::GetConsoleScreenBufferInfo(hStdOut_, &csbiCur))
		{		
			curPos = csbiCur.dwCursorPosition;
			crdBackupBufSize.X = csbiCur.dwMaximumWindowSize.X;
			crdBackupBufSize.Y = (csbiCur.dwCursorPosition.Y - csbiStartPos.dwCursorPosition.Y) + 1;

			COORD crdBuf = {0,0};

			SMALL_RECT srcReg;
			srcReg.Left = 0;
			srcReg.Top = csbiStartPos.dwCursorPosition.Y;
			srcReg.Right = csbiCur.dwMaximumWindowSize.X;
			srcReg.Bottom = csbiCur.dwCursorPosition.Y;

			pBackupBlock = new CHAR_INFO[crdBackupBufSize.X * crdBackupBufSize.Y];
			::ZeroMemory(pBackupBlock, crdBackupBufSize.X * crdBackupBufSize.Y * sizeof(CHAR_INFO));

			bResult = ::ReadConsoleOutput(hStdOut_, pBackupBlock, crdBackupBufSize, crdBuf, &srcReg);

			if (bResult)
			{
				/// 清除这一段内容，定位光标到其开头
				COORD crdWrite;
				crdWrite.X = srcReg.Left;
				crdWrite.Y = srcReg.Top;

				DWORD dwWritten = 0;
				if (::FillConsoleOutputCharacter(
					hStdOut_, 
					_T(' '), 
					crdBackupBufSize.X * crdBackupBufSize.Y, 
					crdWrite, 
					&dwWritten))
				{
					::SetConsoleCursorPosition(hStdOut_, crdWrite);
				}
			}
		}
	}

	/// -# 输出文本
	DWORD	dwWritten	= 0;
	::WriteConsole(hStdOut_, (LPCTSTR)str, str.GetLength(), &dwWritten, NULL);
	/*if (g_bCaptureOutput)
	{
		g_strCaptureBuffer += str;
	}*/

	/// -# 把之前备份的输出缓冲数据接在输出的文本后一行开始的位置
	if (bResult)
	{
		// 如果输出文本最后并非换行，强制换行
		if (str.Right(2).Find(_T('\n')) == -1)
		{
			::WriteConsole(hStdOut_, _T("\r\n"), 2, &dwWritten, NULL);
		}		

		CONSOLE_SCREEN_BUFFER_INFO csbiCur;
		if (::GetConsoleScreenBufferInfo(hStdOut_, &csbiCur))
		{
			COORD crdBuf = {0};
			crdBuf.X = 0;
			crdBuf.Y = 0;

			SMALL_RECT srcReg;
			srcReg.Left = 0;
			srcReg.Top = csbiCur.dwCursorPosition.Y;
			srcReg.Right = csbiCur.dwMaximumWindowSize.X;
			srcReg.Bottom = csbiCur.dwCursorPosition.Y + crdBackupBufSize.Y;

			bResult = ::WriteConsoleOutput(hStdOut_, pBackupBlock, crdBackupBufSize, crdBuf, &srcReg);

			if (bResult)
			{
				COORD crdLast;
				crdLast.X = curPos.X;
				crdLast.Y = srcReg.Bottom;
				csbiReadStartPos_.dwCursorPosition.Y += crdLast.Y - curPos.Y;
				bResult = ::SetConsoleCursorPosition(hStdOut_, crdLast);
			}
		}

		delete[]pBackupBlock;
		pBackupBlock = NULL;
	}
}

void CConsoleWnd::ConsoleReadThreadProc()
{
	while (bConsoleReadThreadCont_)
	{
		DWORD dwRet = ::WaitForSingleObjectEx(hStdIn_,INFINITE,TRUE);
		if (dwRet == WAIT_OBJECT_0)
		{
			INPUT_RECORD inp[1];
			::ZeroMemory(&inp,sizeof(inp));
			DWORD dwRead = 0;
			::PeekConsoleInput(hStdIn_, &inp[0], 1, &dwRead);

			if (dwRead == 0)
			{
				continue;
			}

			if (inp[0].EventType == KEY_EVENT)
			{
				bReading_ = TRUE;
				::ZeroMemory(&csbiReadStartPos_, sizeof(csbiReadStartPos_));
				::GetConsoleScreenBufferInfo(hStdOut_, &csbiReadStartPos_);

				// 收到Key Event，那就集齐一行的新信息，放入命令缓冲，并通知主线程分发处理
				DWORD const SIZE = 1024;
				TCHAR	*pBuf = new TCHAR[SIZE];
				
				if (::ReadConsole(hStdIn_, pBuf, SIZE * sizeof(TCHAR), &dwRead, NULL))
				{
					bReading_ = FALSE;
					pBuf[dwRead] = _T('\0');
					if (bConsoleReadThreadCont_)
					{
						BASE_CLASS::PostMessage(WM_CONSOLE_MESSAGE,dwRead,(LPARAM)pBuf);
					}
				}
				if (!bConsoleReadThreadCont_)
				{
					delete [] pBuf;
					break;
				}
				bReading_ = FALSE;
			}
			else
			{
				// filter all other type events
				::ReadConsoleInput(hStdIn_, &inp[0], 1, &dwRead);
			}
		}
		if (dwRet == WAIT_IO_COMPLETION)
		{
			break;
		}
		else if (bConsoleReadThreadCont_)//failed, maybe handle invalid?
		{
			hStdIn_ = ::GetStdHandle(STD_INPUT_HANDLE);
		}
	}
}