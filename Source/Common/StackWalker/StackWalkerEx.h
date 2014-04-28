#pragma once
#include <Include/Common/CommonDef.h>

#include <dbgeng.h>
#include <vector>
using std::vector;

template  class COMMON_API CComPtr<IDebugClient>;
template  class COMMON_API CComPtr<IDebugControl>;
template  class COMMON_API CComPtr<IDebugSymbols>;
template  class COMMON_API CComPtr<IDebugOutputCallbacks>;

class COMMON_API StackWalkerEx
{
public:
	StackWalkerEx(CONTEXT * pContext);
	~StackWalkerEx();
	void DumpStack();
private:
	BOOL CreateDbgEng();
	BOOL SelfAttach(void);
private:
	CComPtr<IDebugClient> pClient_;
	CComPtr<IDebugControl> pControl_;
	CComPtr<IDebugSymbols> p_Symbols_;
	CComPtr<IDebugOutputCallbacks> pOutputCb_;
	CONTEXT * pContext_;
};

