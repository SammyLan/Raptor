#include "stdafx.h"
#include "StackWalkerEx.h"

#include <windows.h>
#include <dbghelp.h>
#include <iostream>
using namespace std;
#pragma comment(lib,"dbgeng.lib")

#ifdef COMMON_EXPORTS
#include "Include/Common/DumpStack.h"
#endif

namespace{
	class StdioOutputCallbacks : 
		public CComObjectRootEx< CComSingleThreadModel>,
		public IDebugOutputCallbacks
	{
	public:
		BEGIN_COM_MAP(StdioOutputCallbacks)
			COM_INTERFACE_ENTRY(IDebugOutputCallbacks)
			COM_INTERFACE_ENTRY(IUnknown)
		END_COM_MAP()

		// IDebugOutputCallbacks.
		STDMETHOD(Output)(ULONG Mask,PCSTR Text)
		{
			if (Mask == 1)
			{
				cout<<Text;
				OutputDebugStringA(Text);
			}
			return S_OK;
		}
		static void CreateInstance(IDebugOutputCallbacks **ppDebug)
		{
			IDebugOutputCallbacks *pDebug = new CComObjectNoLock<StdioOutputCallbacks>();
			pDebug->AddRef();
			*ppDebug = pDebug;
		}
	};
}


StackWalkerEx::StackWalkerEx(CONTEXT * pContext)
:pContext_(pContext)
{
	StdioOutputCallbacks::CreateInstance(&pOutputCb_);
	CreateDbgEng();
	SelfAttach();
}

StackWalkerEx::~StackWalkerEx()
{ 
}

BOOL StackWalkerEx::CreateDbgEng()
{
	HRESULT Status;

	// Start things off by getting an initial interface from
	// the engine.  This can be any engine interface but is
	// generally IDebugClient as the client interface is
	// where sessions are started.
	if ((Status = DebugCreate(__uuidof(IDebugClient),
		(void**)&pClient_)) != S_OK)
	{
		return FALSE;
	}

	// Query for some other interfaces that we'll need.
	if ((Status = pClient_->QueryInterface(__uuidof(IDebugControl),	(void**)&pControl_)) != S_OK 
		||	(Status = pClient_->QueryInterface(__uuidof(IDebugSymbols),	(void**)&p_Symbols_)) != S_OK)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL StackWalkerEx::SelfAttach(void)
{
	HRESULT Status;

	// Don't set the output callbacks yet as we don't want
	// to see any of the initial debugger output.

	/*if (p_Symbols_ != NULL)
	{
		if ((Status = p_Symbols_->SetSymbolPath(L)) != S_OK)
		{
			return FALSE;
		}
	}*/

	// Everything's set up so do the attach.
	if ((Status = pClient_->AttachProcess(
		0, GetCurrentProcessId(),
		DEBUG_ATTACH_NONINVASIVE |DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND)) != S_OK)
	{
		return FALSE;
	}

	// Finish initialization by waiting for the attach event.
	// This should return quickly as a non-invasive attach
	// can complete immediately.
	if ((Status = pControl_->WaitForEvent(DEBUG_WAIT_DEFAULT,INFINITE)) != S_OK)
	{
		return FALSE;
	}

	// Everything is now initialized and we can make any
	// queries we want.
	return TRUE;
}

void StackWalkerEx::DumpStack()
{
	HRESULT Status;
	int Count = 50;
	char CxrCommand[64];

	printf("\nFirst %d frames of the call stack:\n", Count);

	// Install output callbacks so we get the output from the stack dump.
	if ((Status = pClient_->SetOutputCallbacks(pOutputCb_)) != S_OK)
	{
		return;
	}

	sprintf_s(CxrCommand, 64, ".cxr 0x%p", pContext_);

	// Print the call stack for the given context.
	if ((Status = pControl_->
		Execute(DEBUG_OUTCTL_IGNORE, CxrCommand,
		DEBUG_EXECUTE_NOT_LOGGED)) != S_OK)
	{
		return;
	}

	// If the code is optimized at all it is important to have
	// accurate symbols to get the correct stack.
	if ((Status = pControl_->
		OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS, NULL,
		Count, DEBUG_STACK_SOURCE_LINE |
		DEBUG_STACK_FRAME_ADDRESSES |
		//DEBUG_STACK_COLUMN_NAMES |
		DEBUG_STACK_FRAME_NUMBERS)) != S_OK)
	{
		return;
	}

	// Done with output.
	if ((Status = pClient_->SetOutputCallbacks(NULL)) != S_OK)
	{
		return;
	}


	//
	// The full engine API is available so many other things
	// could be done here.
	//
	// A dump file could be written with WriteDumpFile.
	// The raw stack data could be collected with GetStackTrace and
	// saved along with or instead of the text.
	// An analysis of the current program state could be done
	// to automatically diagnose simple problems.
	//
	// The primary thing to watch out for is that context information
	// for running threads will be stale.  This could be avoided
	// by enumerating and suspending all other threads after the
	// attach completes and then resuming before the assert
	// returns controls.  Otherwise, switching between threads will
	// refresh the thread context and can be used to poll the context
	// state.
	//
}

static DWORD AssertionExceptionDump(PEXCEPTION_POINTERS Exception)
{
	StackWalkerEx walker(Exception->ContextRecord);
	walker.DumpStack();

	return EXCEPTION_EXECUTE_HANDLER;
}

namespace Util
{
	namespace Common
	{
		void DumpStackEx()
		{
			__try
			{
				//使用RtlCaptureContext获取上下文不靠谱,优化后就不能获取到正确的上下文,改用SEH获取
				RaiseException(0x1234, 0, 0, NULL);
			}
			__except(AssertionExceptionDump(GetExceptionInformation()))
			{
				return;
			}
		}
	}
}
