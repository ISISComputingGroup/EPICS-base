
/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* osdSock.c */
/* Revision-Id: anj@aps.anl.gov-20101005192737-disfz3vs0f3fiixd */
/*
 *      Author:		Jeff Hill 
 *      Date:          	04-05-94 
 *
 */

#include <string.h>

#define epicsExportSharedSymbols
#include "osiSock.h"

/*
 * epicsSocketConvertErrorToString()
 */
void epicsSocketConvertErrorToString (
        char * pBuf, unsigned bufSize, int theSockError )
{
    if ( bufSize ) {
        strncpy ( pBuf, strerror ( theSockError ), bufSize );
        pBuf[bufSize-1] = '\0';
    }
}

/*
 * epicsSocketConvertErrnoToString()
 */
void epicsSocketConvertErrnoToString (
        char * pBuf, unsigned bufSize )
{
    epicsSocketConvertErrorToString ( pBuf, bufSize, SOCKERRNO );
}
