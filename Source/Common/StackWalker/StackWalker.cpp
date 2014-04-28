#include "stdafx.h"
#include <windows.h>
#include "StackWalker.h"
#include <dbghelp.h>
#include <Psapi.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"Psapi.lib")

#ifdef COMMON_EXPORTS
#include "Include/Common/DumpStack.h"
#endif


DWORD const STACKWALK_MAX_NAMELEN = 1024;
#define GET_CURRENT_CONTEXT(c, contextFlags)	\
	do {										\
	memset(&c, 0, sizeof(CONTEXT));				\
	c.ContextFlags = contextFlags;				\
	__asm    call x								\
	__asm x: pop eax							\
	__asm    mov c.Eip, eax						\
	__asm    mov c.Ebp, ebp						\
	__asm    mov c.Esp, esp						\
	} while(0);

namespace 
{
	struct  StackWalkerCallbackDefault:public StackWalkerCallback
	{
		virtual void PrintLineInfo(PCHAR szFileName,DWORD dwLine,PCHAR szFuncName,DWORD64 dwAddr,DWORD64 dwRVAOffset,PCHAR szModuleName)
		{
			CHAR szLineInfo[MAX_PATH + 512];
			sprintf_s(szLineInfo,sizeof(szLineInfo)/sizeof(szLineInfo[0]),"0x%08I64x[0x%06I64x] %s:%s",dwAddr,dwRVAOffset,szModuleName,szFuncName);

			if (NULL == szFileName)
			{
				cout<<szLineInfo<<endl;
				OutputDebugStringA(szLineInfo);
				OutputDebugStringA("\n");
			}
			else
			{
				cout<<szLineInfo<<" "<<szFileName<<"("<<dwLine<<")"<<endl;
				CHAR szFullLineInfo[MAX_PATH + 32];
				sprintf_s(szFullLineInfo,sizeof(szFullLineInfo)/sizeof(szFullLineInfo[0]),"%s(%d):  %s\n",szFileName,dwLine,szLineInfo);
				OutputDebugStringA(szFullLineInfo);
			}
		}
	};
	StackWalkerCallbackDefault s_oStackWalkerCallback;
}
StackWalker::StackWalker()
{
	hProcess_ = GetCurrentProcess();
	hThread_ = GetCurrentThread();
	InitSymbol();
}

StackWalker::~StackWalker()
{
	UnInitSymbol();
}
void StackWalker::InitSymbol()
{
	if(!oDbgHelper_.SymInitialize(hProcess_,NULL,TRUE))
	{
		cout<<"Failed"<<endl;
	}
	DWORD symOptions = oDbgHelper_.SymGetOptions();
	symOptions |= (SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS);
	oDbgHelper_.SymSetOptions(symOptions);
}


void StackWalker::UnInitSymbol()
{
	if(!oDbgHelper_.SymCleanup(hProcess_))
	{
		cout<<"Failed"<<endl;
	}
}
void StackWalker::InitStackFrame(CONTEXT * pContext,DWORD & dwMachineType,STACKFRAME64 & stackFrame)
{
	memset(&stackFrame, 0, sizeof(stackFrame));
	
#ifdef _M_IX86
	// normally, call ImageNtHeader() and use machine info from PE header
	dwMachineType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = pContext->Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = pContext->Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = pContext->Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	dwMachineType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = pContext->Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = pContext->Rsp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = pContext->Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
	dwMachineType = IMAGE_FILE_MACHINE_IA64;
	stackFrame.AddrPC.Offset = pContext->StIIP;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = pContext->IntSp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrBStore.Offset = pContext->RsBSP;
	stackFrame.AddrBStore.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = pContext->IntSp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

} 


void StackWalker::DumpStack(StackWalkerCallback * pCallback ,DWORD dwMaxDump,CONTEXT * pContext)
{
	pCallback_= pCallback;
	if (NULL == pCallback)
	{
		pCallback_ = &s_oStackWalkerCallback;
	}

	DWORD const dwBeginDump = 1;
	CONTEXT context;
	if (NULL == pContext)
	{
		memset(&context,0,sizeof(context));
		GET_CURRENT_CONTEXT(context,CONTEXT_FULL);
		pContext = &context;
	}

	STACKFRAME64 stackFrame;
	DWORD imageType = 0;
	InitStackFrame(pContext,imageType,stackFrame);

	dwMaxDump = dwBeginDump + dwMaxDump;
	for (DWORD frameNum = 0; frameNum < dwMaxDump; ++frameNum )
	{
		if ( ! oDbgHelper_.StackWalk64(imageType, GetCurrentProcess(), GetCurrentThread(), &stackFrame, pContext, NULL, oDbgHelper_.SymFunctionTableAccess64, oDbgHelper_.SymGetModuleBase64, NULL) )
		{
			//´íÎó
			break;
		}
		
		if (stackFrame.AddrFrame.Offset == 0 || stackFrame.AddrPC.Offset == stackFrame.AddrReturn.Offset || stackFrame.AddrPC.Offset == 0 || stackFrame.AddrReturn.Offset == 0)
		{
			break;
		}
		if (frameNum >= dwBeginDump)
		{
			PrintLineInfo(stackFrame.AddrPC.Offset);
		}		
	}
}


void StackWalker::PrintLineInfo(DWORD64 dwAddr)
{
	BYTE pBuf[sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN];
	memset(pBuf,0,sizeof(pBuf));
	IMAGEHLP_SYMBOL64 *pSym = (IMAGEHLP_SYMBOL64 *) pBuf;

	pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	pSym->MaxNameLength = STACKWALK_MAX_NAMELEN;

	DWORD64 offsetFromSmybol;
	if (oDbgHelper_.SymGetSymFromAddr64(GetCurrentProcess(), dwAddr, &(offsetFromSmybol), pSym) != FALSE)
	{
		CHAR szName[STACKWALK_MAX_NAMELEN];
		oDbgHelper_.UnDecorateSymbolName( pSym->Name, szName, STACKWALK_MAX_NAMELEN, UNDNAME_NAME_ONLY );

		oDbgHelper_.UnDecorateSymbolName( pSym->Name, szName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE );

		if (oDbgHelper_.SymGetLineFromAddr64 != NULL )
		{ 
			DWORD offsetFromLine;
			IMAGEHLP_LINE64 Line;
			memset(&Line, 0, sizeof(Line));
			Line.SizeOfStruct = sizeof(Line);

			if (oDbgHelper_.SymGetLineFromAddr64(GetCurrentProcess(), dwAddr, &(offsetFromLine), &Line) != FALSE)
			{
				PrintLineInfo(Line.FileName,Line.LineNumber,pSym->Name,dwAddr);
			}
			else
			{
				PrintLineInfo(NULL,NULL,pSym->Name,dwAddr);
			}
		}
	}
	else
	{
		PrintLineInfo(NULL,NULL,NULL,dwAddr);
	}	
}
void StackWalker::PrintLineInfo(PCHAR fileName,DWORD dwLine,PCHAR funcName,DWORD64 dwAddr)
{
	if (funcName == NULL)
	{
		funcName = "<unknown>";
	}
	DWORD64 dwBaseAddr = oDbgHelper_.SymGetModuleBase64(GetCurrentProcess(),dwAddr);
	DWORD64 dwRVAOffset = dwAddr - dwBaseAddr;
	CHAR szModuleName[MAX_PATH];
	DWORD dwLen = GetModuleBaseNameA(GetCurrentProcess(),(HMODULE)dwBaseAddr,szModuleName,sizeof(szModuleName)/sizeof(szModuleName[0]));
	szModuleName[dwLen] = '\0';

	pCallback_->PrintLineInfo(fileName,dwLine,funcName,dwAddr,dwRVAOffset,szModuleName);
}

namespace Util
{
	namespace Common
	{
		void DumpStack()
		{
			StackWalker walker;
			CONTEXT c;
			memset(&c,0,sizeof(c));
			GET_CURRENT_CONTEXT(c,CONTEXT_FULL);			
			walker.DumpStack(NULL,20,&c);
		}
	}
}
