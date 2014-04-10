#pragma once

#ifdef CONSOLE_EXPORTS
	#define	CONSOLE_API __declspec(dllexport)
#else
	#define	CONSOLE_API __declspec(dllimport)
#pragma comment(lib,"Console.lib")
#endif