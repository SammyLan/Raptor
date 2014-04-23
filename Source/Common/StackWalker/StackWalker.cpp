#include "stdafx.h"
#include <windows.h>
#include "StackWalker.h"
#include <dbghelp.h>
#include <Psapi.h>
#include <iostream>
using namespace std;
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"Psapi.lib")

#ifdef COMMON_EXPORTS
#include "Include/Common/DumpStack.h"
#endif

namespace{
	BOOL CALLBACK ReadProcessMemoryProc64(  HANDLE hProcess,  DWORD64 lpBaseAddress, PVOID lpBuffer,  DWORD nSize,  LPDWORD lpNumberOfBytesRead)
	{
		return TRUE;
	}

	PVOID CALLBACK FunctionTableAccessProc64(  HANDLE hProcess,  DWORD64 AddrBase)
	{
		return SymFunctionTableAccess64(hProcess,AddrBase);
	}

	DWORD64 CALLBACK GetModuleBaseProc64(  HANDLE hProcess,  DWORD64 Address)
	{
		return SymGetModuleBase64(hProcess,Address);
	}

	DWORD64 CALLBACK TranslateAddressProc64(  HANDLE hProcess,  HANDLE hThread,LPADDRESS64 lpaddr)
	{
		return 0;
	}
}


StackWalker::StackWalker(CONTEXT const &context)
{
	memcpy(&context_,&context,sizeof(context));	
	hProcess_ = GetCurrentProcess();
	hThread_ = GetCurrentThread();
	SaveStackInfo();
}

StackWalker::~StackWalker()
{ 
}
 
void StackWalker::InitSymbol()
{
	if(!SymInitialize(hProcess_,NULL,TRUE))
	{
		cout<<"Failed"<<endl;
	}
	DWORD symOptions = SymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
	SymSetOptions(symOptions);

}

void StackWalker::UnInitSymbol()
{
	if(!SymCleanup(hProcess_))
	{
		cout<<"Failed"<<endl;
	}
}
void StackWalker::SaveStackInfo()
{
	STACKFRAME64 stackFrame; 
	memset(&stackFrame, 0, sizeof(stackFrame));
	DWORD imageType;
#ifdef _M_IX86
	// normally, call ImageNtHeader() and use machine info from PE header
	imageType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = context_.Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context_.Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context_.Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	imageType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = context_.Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context_.Rsp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context_.Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
	imageType = IMAGE_FILE_MACHINE_IA64;
	stackFrame.AddrPC.Offset = context_.StIIP;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context_.IntSp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrBStore.Offset = context_.RsBSP;
	stackFrame.AddrBStore.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context_.IntSp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif


	int dwSize = 30;
	for (int i = 0; i < dwSize; ++i)
	{
		if(!StackWalk64(imageType,hProcess_,hThread_,&stackFrame,&context_, NULL/*ReadProcessMemoryProc64*/,FunctionTableAccessProc64, GetModuleBaseProc64,NULL/*TranslateAddressProc64*/))
		{
			break;			 
		}

		if (stackFrame.AddrPC.Offset == stackFrame.AddrReturn.Offset
			||stackFrame.AddrPC.Offset == 0)
		{
			break;
		}
		callStack_.push_back(stackFrame.AddrPC.Offset);
	}
}

void StackWalker::PrintLineInfo(PCHAR fileName,DWORD dwLine,PCHAR funcName,DWORD dwAddr)
{
	if (funcName == NULL)
	{
		funcName = "<unknown>";
	}
	DWORD dwBaseAddr = SymGetModuleBase(hProcess_,dwAddr);
	DWORD dwRVAOffset = dwAddr - dwBaseAddr;
	TCHAR szModuleName[MAX_PATH];
	DWORD dwLen = GetModuleBaseName(hProcess_,(HMODULE)dwBaseAddr,szModuleName,sizeof(szModuleName)/sizeof(szModuleName[0]));
	szModuleName[dwLen] = '\0';


	CHAR szLineInfo[MAX_PATH + 512];
	sprintf_s(szLineInfo,sizeof(szLineInfo)/sizeof(szLineInfo[0]),"0x%08x[0x%06x] %S:%s",dwAddr,dwRVAOffset,szModuleName,funcName);

	if (NULL == fileName)
	{
		cout<<szLineInfo<<endl;
		OutputDebugStringA(szLineInfo);
		OutputDebugStringA("\n");
	}
	else
	{
		cout<<szLineInfo<<" "<<fileName<<"("<<dwLine<<")"<<endl;
		CHAR szFullLineInfo[MAX_PATH + 32];
		sprintf_s(szFullLineInfo,sizeof(szFullLineInfo)/sizeof(szFullLineInfo[0]),"%s(%d):  %s\n",fileName,dwLine,szLineInfo);
		OutputDebugStringA(szFullLineInfo);
	}
}

void StackWalker::DumpStack()
{
	InitSymbol();
	for (size_t i = 2; i < callStack_.size();++i)
	{
		DWORD dwAddr = (DWORD)callStack_[i];

		//��ȡ������
		DWORD64  dwDisplacement = 0;
		ULONG64 buffer[(sizeof(SYMBOL_INFO) +
			MAX_SYM_NAME*sizeof(TCHAR) +
			sizeof(ULONG64) - 1) /
			sizeof(ULONG64)];
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		if (SymFromAddr(hProcess_, dwAddr, &dwDisplacement, pSymbol))
		{
			IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
			DWORD dwLineDisplacement;
			if( SymGetLineFromAddr( hProcess_, dwAddr, &dwLineDisplacement, &lineInfo ))
			{
				PrintLineInfo(lineInfo.FileName,lineInfo.LineNumber,pSymbol->Name,dwAddr);
			}
			else
			{
				PrintLineInfo(NULL,0,pSymbol->Name,dwAddr);
			}
		}
		else
		{
			PrintLineInfo(NULL,0,NULL,dwAddr);
		}		
	}
	UnInitSymbol();
}

DWORD AssertionExceptionDump(PEXCEPTION_POINTERS Exception)
{
	StackWalker walker(*(Exception->ContextRecord));
	walker.DumpStack();

	return EXCEPTION_EXECUTE_HANDLER;
}

namespace Util
{
	namespace Common
	{
		void DumpStack()
		{
			__try
			{
				//ʹ��RtlCaptureContext��ȡ�����Ĳ�����,�Ż���Ͳ��ܻ�ȡ����ȷ��������,����SEH��ȡ
				RaiseException(0x1234, 0, 0, NULL);
			}
			__except(AssertionExceptionDump(GetExceptionInformation()))
			{
				return;
			}
		}
	}
}
