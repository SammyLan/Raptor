#pragma once
#include <DbgHelp.h>


template<bool> struct StaticAssert_Failed;
template<> struct StaticAssert_Failed<true>{};
#define STATIC_ASSERT(exp) (StaticAssert_Failed<(bool)(exp)>())

template <typename Dest,typename Src>
Dest StaticCast(Src src)
{
	STATIC_ASSERT(sizeof(src) == sizeof(Dest));
	static union
	{
		Src src_;
		Dest dest_;
	};
	src_ = src;
	return dest_;
}

#define ZeroMemberBegin()	DWORD dwZeroMemberBegin
#define ZeroMemberEnd()		DWORD dwZeroMemberEnd
#define ZeroMember()																				\
do																									\
{																									\
	this->dwZeroMemberBegin = 10000;																\
	this->dwZeroMemberEnd	= 10001;																\
	BYTE * begin = StaticCast<BYTE*>(&this->dwZeroMemberBegin) + sizeof(this->dwZeroMemberBegin);	\
	BYTE * end  = StaticCast<BYTE*>(&this->dwZeroMemberEnd);										\
	memset(begin,0,end - begin);																	\
} while(0)


namespace DbgHelp
{
	typedef BOOL (WINAPI *StackWalk64)( 
		DWORD MachineType, 
		HANDLE hProcess,
		HANDLE hThread, 
		LPSTACKFRAME64 StackFrame, 
		PVOID ContextRecord,
		PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
		PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
		PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
		PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );

	typedef BOOL (WINAPI * SymInitialize)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
	typedef BOOL (WINAPI * SymCleanup)( IN HANDLE hProcess );

	typedef PVOID (WINAPI * SymFunctionTableAccess64)( HANDLE hProcess, DWORD64 AddrBase );

	typedef BOOL (WINAPI * SymFromAddr)(HANDLE hProcess,DWORD64 Address,PDWORD64 Displacement,PSYMBOL_INFO Symbol);
	typedef BOOL (WINAPI * SymGetLineFromAddr64)( IN HANDLE hProcess, IN DWORD64 dwAddr,OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );
	typedef BOOL (WINAPI * SymGetSymFromAddr64)( IN HANDLE hProcess, IN DWORD64 dwAddr,OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol );
	typedef BOOL (WINAPI * SymGetModuleInfo64)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT  PIMAGEHLP_MODULE64 *ModuleInfo );
	
	typedef DWORD (WINAPI * SymGetOptions)( VOID );
	typedef DWORD (WINAPI * SymSetOptions)( IN DWORD SymOptions );

	
	typedef DWORD64 (WINAPI * SymGetModuleBase64)( IN HANDLE hProcess, IN DWORD64 dwAddr );
	typedef DWORD64 (WINAPI * SymLoadModule64)( IN HANDLE hProcess, IN HANDLE hFile,IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll );

	typedef DWORD (WINAPI * UnDecorateSymbolName)( PCSTR DecoratedName, PSTR UnDecoratedName,	DWORD UndecoratedLength, DWORD Flags ); 

	typedef BOOL (WINAPI * SymGetSearchPath)(HANDLE hProcess, PSTR SearchPath, DWORD SearchPathLength);
	typedef BOOL (WINAPI * SymSetSearchPath)(HANDLE hProcess,PCTSTR SearchPath	);	
}
#define DECLARE_Member(pfnName)	DbgHelp::##pfnName pfnName
#define SET_Member(pfnName)		this->##pfnName = (DbgHelp::##pfnName) GetProcAddress(hDbhHelpDLL_, #pfnName )
#define RESET_Member(pfnName)	this->##pfnName = NULL


class DbgHelpWrapper
{
public:
	DbgHelpWrapper();
	~DbgHelpWrapper();
	BOOL IsDbhHelpDLLLoaded(){return hDbhHelpDLL_ != NULL;}
private:
	BOOL	LoadDbhHelpDLLL();
	void	InitFunc();
	HMODULE hDbhHelpDLL_;
public:
	ZeroMemberBegin();
	DECLARE_Member(StackWalk64);

	DECLARE_Member(SymInitialize);
	DECLARE_Member(SymCleanup);

	DECLARE_Member(SymFunctionTableAccess64);

	DECLARE_Member(SymFromAddr);
	DECLARE_Member(SymGetLineFromAddr64);
	DECLARE_Member(SymGetSymFromAddr64);
	DECLARE_Member(SymGetModuleInfo64);

	DECLARE_Member(SymGetOptions);
	DECLARE_Member(SymSetOptions);

	DECLARE_Member(SymGetModuleBase64);
	DECLARE_Member(SymLoadModule64);

	DECLARE_Member(UnDecorateSymbolName);

	DECLARE_Member(SymGetSearchPath);
	DECLARE_Member(SymSetSearchPath);
	ZeroMemberEnd();
};