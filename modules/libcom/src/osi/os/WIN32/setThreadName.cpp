
/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
#define VC_EXTRALEAN
#define STRICT
#include <windows.h>

/*
 * Usage: setThreadName (-1, "MainThread");
 */
 
extern "C" void setThreadName ( DWORD dwThreadID, LPCSTR szThreadName )
{
	if (szThreadName == NULL || *szThreadName == '\0')
	{
		return;
	}
#if _MSC_VER >= 1300 && defined ( _DEBUG )
// This was copied directly from an MSDN example
// It sets the thread name by throwing a special exception that is caught by Visual Sudio
// It requires the debugger to be already attached to the process 
// when the exception is thrown for the name to be registered
    static const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType;     // Must be 0x1000.
        LPCSTR szName;    // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags;    // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try
	{
        RaiseException(MS_VC_EXCEPTION, 0,
		               sizeof(info) / sizeof(ULONG_PTR),
					   (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
	{
    }
#pragma warning(pop)
#endif
}
