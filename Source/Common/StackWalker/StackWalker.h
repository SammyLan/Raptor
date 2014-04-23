#pragma once
#include <vector>
using std::vector;
class StackWalker
{
public:
	StackWalker(CONTEXT * pContext);
	~StackWalker();
	void DumpStack();
protected:
	virtual void PrintLineInfo(PCHAR fileName,DWORD dwLine, PCHAR funcName,DWORD dwAddr);
private:
	void SaveStackInfo();
	void InitSymbol();
	void UnInitSymbol();
private:
	HANDLE hProcess_;
	HANDLE hThread_;
	CONTEXT * pContext_;
	vector<DWORD64> callStack_;
};

