/** 
@file 
@brief		
@version	2012-11-08 SammyLan
*/

#pragma once
#include "UtilDef.h"
#include <memory>

BEGIN_NS(Raptor)
BEGIN_NS(Util)

template <typename T>
class HRTXThreadPool
{
	typedef std::pair<void (T::*)(), T *> ThreadInfo;
public:
	static BOOL QueueUserWorkItem(void (T::*function)(void), T *object, ULONG flags = WT_EXECUTELONGFUNCTION)
	{        
		std::auto_ptr<ThreadInfo> p(new ThreadInfo(function, object));
		if (::QueueUserWorkItem(ThreadProc, p.get(), flags))
		{
			//现在由线程函数负责删除前面申请的内存
			p.release();
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

private:
	static DWORD WINAPI ThreadProc(PVOID context)
	{
		std::auto_ptr<ThreadInfo> p(static_cast<ThreadInfo *>(context));
		(p->second->*p->first)();
		return 0;
	}
};
template <typename T>
inline BOOL HRTXQueueUserWorkItem(T *object,void (T::*function)(void), ULONG flags = WT_EXECUTELONGFUNCTION)
{
	return HRTXThreadPool<T>::QueueUserWorkItem(function,object,WT_EXECUTELONGFUNCTION);
}



template <typename T>
class HRTXThreadObject
{
	typedef unsigned int uint;
	typedef std::pair<void (T::*)(), T *> ThreadInfo;
public:
	static HANDLE CreateThread(void (T::*function)(void), T *object)
	{        
		std::auto_ptr<ThreadInfo> p(new ThreadInfo(function, object));
		uint dwTHreadID = 0;
		HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadProc, p.get(),0,&dwTHreadID);
		if (hThread != NULL)
		{
			//现在由线程函数负责删除前面申请的内存
			p.release();
		}
		return hThread;
	}

private:
	static uint WINAPI ThreadProc(PVOID context)
	{
		std::auto_ptr<ThreadInfo> p(static_cast<ThreadInfo *>(context));
		(p->second->*p->first)();
		return 0;
	}
};
template <typename T>
inline HANDLE HRTXCreateThread(T *object,void (T::*function)(void))
{
	return HRTXThreadObject<T>::CreateThread(function,object);
}

END_NS()
END_NS()