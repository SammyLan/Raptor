// Common.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Include/Common/UtilFile.h>

namespace Util
{
	namespace File
	{
		CString GetExeDir()
		{
			CString    sPath;   
			GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
			sPath.ReleaseBuffer    ();   
			int    nPos;   
			nPos=sPath.ReverseFind('\\');   
			sPath=sPath.Left(nPos);   
			return    sPath;   
		}
	}
}