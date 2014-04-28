#pragma once
#include <vector>
#include <Include/Common/CommonDef.h>
#include "DbgHelpWrapper.h"

using std::vector;

struct  StackWalkerCallback
{
	virtual void BeginDump() = 0; 
	virtual void PrintLineInfo(DWORD dwIndex,PCHAR szFileName,DWORD dwLine,PCHAR szFuncName,DWORD64 dwAddr,DWORD64 dwRVAOffset,PCHAR szModuleName) = 0;
	virtual void EndDump() = 0; 
};

class COMMON_API StackWalker
{
public:
	StackWalker();
	~StackWalker();
	void DumpStack(StackWalkerCallback * pCallback = NULL,DWORD dwMaxDump = 20,CONTEXT * pContext = NULL);
private:
	void InitSymbol();
	void UnInitSymbol();
	static void InitStackFrame(CONTEXT * pContext,DWORD & dwMachineType,STACKFRAME64 & stackFrame);
	void PrintLineInfo(DWORD dwIndex,DWORD64 dwAddr);
	void PrintLineInfo(DWORD dwIndex,PCHAR fileName,DWORD dwLine,PCHAR funcName,DWORD64 dwAddr);
private:
	DbgHelpWrapper			oDbgHelper_;
	StackWalkerCallback *	pCallback_;
	HANDLE hProcess_;
	HANDLE hThread_;
};

