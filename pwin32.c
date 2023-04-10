/******************************************************************************
 * Copyright (c) 2023 Jaroslav Hensl                                          *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person                *
 * obtaining a copy of this software and associated documentation             *
 * files (the "Software"), to deal in the Software without                    *
 * restriction, including without limitation the rights to use,               *
 * copy, modify, merge, publish, distribute, sublicense, and/or sell          *
 * copies of the Software, and to permit persons to whom the                  *
 * Software is furnished to do so, subject to the following                   *
 * conditions:                                                                *
 *                                                                            *
 * The above copyright notice and this permission notice shall be             *
 * included in all copies or substantial portions of the Software.            *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,            *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES            *
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                   *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT                *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,               *
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING               *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR              *
 * OTHER DEALINGS IN THE SOFTWARE.                                            *
 *                                                                            *
 ******************************************************************************/
#ifdef _WIN32
#include <windows.h>
#endif

#include "patcher9x.h"

#ifdef _WIN32
/*
*  NT function tip from: https://stackoverflow.com/a/57130 
 * but here is modification, 9x kernel has these function defined, but call them
 * fail with ERROR_CALL_NOT_IMPLEMENTED.
 */
typedef BOOL (WINAPI * GetThreadPriorityBoostF)(HANDLE hThread, PBOOL pDisablePriorityBoost);
static BOOL version_is_nt()
{
	HANDLE h = GetModuleHandleA("kernel32.dll");
	if(h)
	{
		GetThreadPriorityBoostF GetThreadPriorityBoostH = (GetThreadPriorityBoostF)GetProcAddress(h, "GetThreadPriorityBoost");
		if(GetThreadPriorityBoostH != NULL)
		{
			BOOL junk;
			if(GetThreadPriorityBoostH(GetCurrentThread(), &junk))
			{
				return TRUE;	
			}
			
			if(GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
			{
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

void set_default_path(char *default_path)
{
	if(version_is_nt())
	{
		GetModuleFileNameA(NULL, default_path, MAX_PATH);
		default_path[MAX_PATH-1] = '\0';
		
		/* cutoff file name */
		char *p1 = strrchr(default_path, '\\');
		char *p2 = strrchr(default_path, '/');
		
		if(p1 > p2)
		{
			*p1 = '\0';
		}
		else if(p2 != NULL)
		{
			*p2 = '\0';
		}
	}
}

#else

void set_default_path(char *default_path)
{
	(void)default_path;
}

#endif



