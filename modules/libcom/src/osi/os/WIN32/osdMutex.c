/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS Base is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/
/* osdMutex.c */
/*
 *      WIN32 version
 *
 *      Author  Jeffrey O. Hill
 *              johill@lanl.gov
 *              505 665 1831
 *
 */

#include <stdio.h>
#include <limits.h>

#define VC_EXTRALEAN
#define STRICT
#include <windows.h>
#if _WIN32_WINNT < 0x0501
#   error Minimum supported is Windows XP
#endif

#define EPICS_PRIVATE_API

#include "libComAPI.h"
#include "epicsMutex.h"
#include "epicsAssert.h"
#include "epicsStdio.h"

typedef struct epicsMutexOSD {
    union {
        CRITICAL_SECTION criticalSection;
    } os;
} epicsMutexOSD;

/*
 * epicsMutexCreate ()
 */
epicsMutexOSD * epicsMutexOsdCreate ( void )
{
    epicsMutexOSD * pSem;

    pSem = malloc ( sizeof (*pSem) );
    if ( pSem ) {
        InitializeCriticalSection ( &pSem->os.criticalSection );
    }
    return pSem;
}

/*
 * epicsMutexOsdDestroy ()
 */
void epicsMutexOsdDestroy ( epicsMutexOSD * pSem )
{
    DeleteCriticalSection  ( &pSem->os.criticalSection );
    free ( pSem );
}

/*
 * epicsMutexOsdUnlock ()
 */
void epicsMutexOsdUnlock ( epicsMutexOSD * pSem )
{
    LeaveCriticalSection ( &pSem->os.criticalSection );
}

/*
 * epicsMutexOsdLock ()
 */
epicsMutexLockStatus epicsMutexOsdLock ( epicsMutexOSD * pSem )
{
    EnterCriticalSection ( &pSem->os.criticalSection );
    return epicsMutexLockOK;
}

/*
 * epicsMutexOsdTryLock ()
 */
epicsMutexLockStatus epicsMutexOsdTryLock ( epicsMutexOSD * pSem )
{
    if ( TryEnterCriticalSection ( &pSem->os.criticalSection ) ) {
        return epicsMutexLockOK;
    }
    else {
        return epicsMutexLockTimeout;
    }
}

/*
 * epicsMutexOsdShow ()
 */
void epicsMutexOsdShow ( epicsMutexOSD * pSem, unsigned level )
{
    printf ("epicsMutex: win32 critical section at %p\n",
        (void * ) & pSem->os.criticalSection );
}

void epicsMutexOsdShowAll(void) {}

