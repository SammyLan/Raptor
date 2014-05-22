// Common.cpp : 定义 DLL 应用程序的入口点。
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