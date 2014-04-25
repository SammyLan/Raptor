#include "stdafx.h"
#include "DbgHelpWrapper.h"

DbgHelpWrapper::DbgHelpWrapper()
{
	InitFunc();
}

DbgHelpWrapper::~DbgHelpWrapper()
{
	if (NULL != hDbhHelpDLL_ )
	{
		FreeLibrary(hDbhHelpDLL_);
		hDbhHelpDLL_ = NULL;

		RESET_Member(StackWalk64);

		RESET_Member(SymInitialize);
		RESET_Member(SymCleanup);

		RESET_Member(SymFunctionTableAccess64);

		RESET_Member(SymGetLineFromAddr64);
		RESET_Member(SymGetSymFromAddr64);
		RESET_Member(SymGetModuleInfo64);

		RESET_Member(SymGetOptions);
		RESET_Member(SymSetOptions);

		RESET_Member(SymGetModuleBase64);
		RESET_Member(SymLoadModule64);

		RESET_Member(UnDecorateSymbolName);

		RESET_Member(SymGetSearchPath);
		RESET_Member(SymSetSearchPath);
	}
}

BOOL DbgHelpWrapper::LoadDbhHelpDLLL()
{
	 hDbhHelpDLL_ = LoadLibrary( _T("dbghelp.dll") );
	 return hDbhHelpDLL_ != NULL;
}

void DbgHelpWrapper::InitFunc()
{
	LoadDbhHelpDLLL();
	if (NULL != hDbhHelpDLL_ && this->StackWalk64 == NULL)
	{
		SET_Member(StackWalk64);

		SET_Member(SymInitialize);
		SET_Member(SymCleanup);

		SET_Member(SymFunctionTableAccess64);

		SET_Member(SymGetLineFromAddr64);
		SET_Member(SymGetSymFromAddr64);
		SET_Member(SymGetModuleInfo64);

		SET_Member(SymGetOptions);
		SET_Member(SymSetOptions);

		SET_Member(SymGetModuleBase64);
		SET_Member(SymLoadModule64);

		SET_Member(UnDecorateSymbolName);

		SET_Member(SymGetSearchPath);
		SET_Member(SymSetSearchPath);
	}
}