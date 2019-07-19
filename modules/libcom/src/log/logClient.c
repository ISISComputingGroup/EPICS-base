/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* logClient.c,v 1.25.2.6 2004/10/07 13:37:34 mrk Exp */
/*
 *      Author:         Jeffrey O. Hill 
 *      Date:           080791 
 */

/*
 * ANSI C
 */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/timeb.h>

#define epicsExportSharedSymbols
#include "dbDefs.h"
#include "epicsEvent.h"
#include "iocLog.h"
#include "errlog.h"
#include "epicsMutex.h"
#include "epicsThread.h"
#include "epicsTime.h"
#include "osiSock.h"
#include "epicsAssert.h"
#include "epicsExit.h"
#include "epicsSignal.h"
#include "epicsString.h"
#include "epicsStdio.h"
#include "macLib.h"

#include "logClient.h"

typedef struct {
    char                msgBuf[0x4000];
    struct sockaddr_in  addr;
    char                name[64];
    epicsMutexId        mutex;
    SOCKET              sock;
    epicsThreadId       restartThreadId;
    epicsEventId        stateChangeNotify;
    unsigned            connectCount;
    unsigned            nextMsgIndex;
    unsigned            connected;
    unsigned            shutdown;
    unsigned            shutdownConfirm;
    int                 connFailStatus;
} logClient;

static const double      LOG_RESTART_DELAY = 5.0; /* sec */
static const double      LOG_SERVER_CREATE_CONNECT_SYNC_TIMEOUT = 5.0; /* sec */
static const double      LOG_SERVER_SHUTDOWN_TIMEOUT = 30.0; /* sec */

/*
 * If set using iocLogPrefix() this string is prepended to all log messages:
 */
static char* logClientPrefix = NULL;

/*
 * logClientClose ()
 */
static void logClientClose ( logClient *pClient )
{
#   ifdef DEBUG
        fprintf (stderr, "log client: lingering for connection close...");
        fflush (stderr);
#   endif

    /*
     * mutex on
     */
    epicsMutexMustLock (pClient->mutex);
    
    /*
     * close any preexisting connection to the log server
     */
    if ( pClient->sock != INVALID_SOCKET ) {
        epicsSocketDestroy ( pClient->sock );
        pClient->sock = INVALID_SOCKET;
    }

    pClient->nextMsgIndex = 0u;
    memset ( pClient->msgBuf, '\0', sizeof ( pClient->msgBuf ) );
    pClient->connected = 0u;

    /*
     * mutex off
     */
    epicsMutexUnlock (pClient->mutex);

#   ifdef DEBUG
        fprintf (stderr, "done\n");
#   endif
}

/*
 * logClientDestroy
 */
static void logClientDestroy (logClientId id)
{
    enum epicsSocketSystemCallInterruptMechanismQueryInfo interruptInfo;
    logClient *pClient = (logClient *) id;
    epicsTimeStamp begin, current;
    double diff;

    /* command log client thread to shutdown - taking mutex here */
    /* forces cache flush on SMP machines */
    epicsMutexMustLock ( pClient->mutex );
    pClient->shutdown = 1u;
    epicsMutexUnlock ( pClient->mutex );

    /* unblock log client thread blocking in send() or connect() */
    interruptInfo =
        epicsSocketSystemCallInterruptMechanismQuery ();
    switch ( interruptInfo ) {
    case esscimqi_socketCloseRequired:
        logClientClose ( pClient );
        break;
    case esscimqi_socketBothShutdownRequired:
        shutdown ( pClient->sock, SHUT_WR );
        break;
    case esscimqi_socketSigAlarmRequired:
        epicsSignalRaiseSigAlarm ( pClient->restartThreadId );
        break;
    default:
        break;
    };

    /* wait for confirmation that the thread exited - taking */
    /* mutex here forces cache update on SMP machines */
    epicsTimeGetCurrent ( & begin );
    epicsMutexMustLock ( pClient->mutex );
    do {
        epicsMutexUnlock ( pClient->mutex );
        epicsEventWaitWithTimeout ( 
            pClient->stateChangeNotify, 
            LOG_SERVER_SHUTDOWN_TIMEOUT / 10.0 ); 
        epicsTimeGetCurrent ( & current );
        diff = epicsTimeDiffInSeconds ( & current, & begin );
        epicsMutexMustLock ( pClient->mutex );
    }
    while ( ! pClient->shutdownConfirm && diff < LOG_SERVER_SHUTDOWN_TIMEOUT );
    epicsMutexUnlock ( pClient->mutex );

    if ( ! pClient->shutdownConfirm ) {
        fprintf ( stderr, "log client shutdown: timed out stopping"
            " reconnect thread for \"%s\" after %.1f seconds - cleanup aborted\n",
            pClient->name, LOG_SERVER_SHUTDOWN_TIMEOUT );
        return;
    }

    errlogRemoveListeners ( logClientSendMessage, (void *) pClient );

    logClientClose ( pClient );

    epicsMutexDestroy ( pClient->mutex );
   
    epicsEventDestroy ( pClient->stateChangeNotify );

    free ( pClient );
}

/*
 * This method requires the pClient->mutex be owned already.
 */
 
static const char* xml_format = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<message>"
"<clientName>%s</clientName>"
"<severity>%s</severity>"
"<contents><![CDATA[%s]]></contents>"
"<type>ioclog</type>"
"<eventTime>%s</eventTime>"
"</message>\n";


static int timezoneOffset(char* buffer, int len_buffer, epicsTimeStamp* the_time)
{
    time_t ansi_time;
    struct tm tm_struct, tm_struct_gm;
    int tzhdiff, tzmdiff, tzddiff;
    epicsTimeToTime_t(&ansi_time, the_time);
    epicsTime_gmtime(&ansi_time, &tm_struct_gm);
    epicsTime_localtime(&ansi_time, &tm_struct);
    tzddiff = (7 + tm_struct.tm_wday - tm_struct_gm.tm_wday) % 7;  /* allow for day wrap */
	tzhdiff = 24 * tzddiff + (tm_struct.tm_hour - tm_struct_gm.tm_hour);
    tzmdiff = tm_struct.tm_min - tm_struct_gm.tm_min;
    epicsSnprintf(buffer, len_buffer, "%c%02d:%02d", (tzhdiff < 0 ? '-' : '+'), tzhdiff, tzmdiff);
	return 0;
}

static void sendMessageChunk(logClient * pClient, const char * orig_message)
{
    int i;
    unsigned char c;
    unsigned int strSize, max_len_msg;
    char *ioc, *iocname; 
	char *message, *message_ptr;
	char sev_str[16];
	char event_time[32];
    epicsTimeStamp the_time;
    /* severity to use when it has not otherwise been specified via e.g. errlogSevPrintf()
       Changed from MAJOR to MINOR to reduce number of messages displayed by default in IBEX GUI */
    static const char* default_severity = "MINOR";
	
    if ( ! pClient || ! orig_message ) {
        return;
    }
    epicsTimeGetCurrent(&the_time);
	i = epicsTimeToStrftime(event_time, sizeof(event_time), "%Y-%m-%dT%H:%M:%S.%03f", &the_time);
	timezoneOffset(event_time + i, sizeof(event_time) - i, &the_time); 
	iocname = macEnvExpand("$(IOCNAME=)");
	if (iocname == NULL || *iocname == '\0')
	{
	    ioc = macEnvExpand("$(IOC=)"); /* this variable is autogenerated by the EPICS build and written to envPaths */
		if (ioc != NULL && *ioc != '\0')
		{
		    if (strlen(ioc) > 3)
		    {
		        iocname = strdup(ioc + 3); /* remove "ioc" prefix from name */
			}
		    free(ioc);
		}
	}
	if ( 1 == sscanf(orig_message, "sevr=%15s ", sev_str) )  /* message from errlogSevPrintf() with sevr= prefix */ 
	{
	    for(i=0; i<strlen(sev_str); ++i)
		{
		    sev_str[i] = toupper(sev_str[i]);
		}
	    orig_message = orig_message + strlen(sev_str) + 6; /* skip over sevr= bit */
	}
	else
	{
	    strcpy(sev_str, default_severity);
	}
	max_len_msg = strlen(xml_format) + (iocname != NULL ? strlen(iocname) : 8) + strlen(sev_str) + strlen(orig_message) + strlen(event_time) + 1;
	message = message_ptr = (char*)malloc(max_len_msg + 1);
	epicsSnprintf(message, max_len_msg, xml_format, (iocname != NULL ? iocname : "UNKNOWN"), sev_str, orig_message, event_time);
	message[max_len_msg] = '\0';
    strSize = strlen ( message );
	// remove any invalid characters
	for(i=0; i<strSize; ++i)
	{
        c = message[i];
	    if ( !(isascii(c) && (isprint(c) || isspace(c))) )
		{
		    message[i] = '?'; /* avoid replacing with space in case it changes column meaning */
		}
	}
	if (iocname != NULL)
	{
	    free(iocname);
	}

    while ( strSize ) {
        unsigned msgBufBytesLeft = 
            sizeof ( pClient->msgBuf ) - pClient->nextMsgIndex;

        if ( strSize > msgBufBytesLeft ) {
            int status;

            if ( ! pClient->connected ) {
                break;
            }

// as we use XML messages try to avoid sending half a message that may be invalid XML
// so we only split a message if it will not fit into an empty buffer
            if ( strSize > sizeof(pClient->msgBuf) ) {
                if ( msgBufBytesLeft > 0u ) {
                    memcpy ( & pClient->msgBuf[pClient->nextMsgIndex],
                        message, msgBufBytesLeft );
                    pClient->nextMsgIndex += msgBufBytesLeft;
                    strSize -= msgBufBytesLeft;
                    message += msgBufBytesLeft;
                }
            }

            status = send ( pClient->sock, pClient->msgBuf, 
                pClient->nextMsgIndex, 0 );
            if ( status > 0 ) {
                unsigned nSent = (unsigned) status;
                if ( nSent < pClient->nextMsgIndex ) {
                    unsigned newNextMsgIndex = pClient->nextMsgIndex - nSent;
                    memmove ( pClient->msgBuf, & pClient->msgBuf[nSent], 
                        newNextMsgIndex );
                    pClient->nextMsgIndex = newNextMsgIndex;
                }
                else {
                    pClient->nextMsgIndex = 0u;
                }
            }
            else {
                if ( ! pClient->shutdown ) {
                    char sockErrBuf[64];
                    if ( status ) {
                        epicsSocketConvertErrnoToString ( sockErrBuf, sizeof ( sockErrBuf ) );
                    }
                    else {
                        strcpy ( sockErrBuf, "server initiated disconnect" );
                    }
                    fprintf ( stderr, "log client: lost contact with log server at \"%s\" because \"%s\"\n", 
                        pClient->name, sockErrBuf );
                }
                logClientClose ( pClient );
                break;
            }
        }
        else {
            memcpy ( & pClient->msgBuf[pClient->nextMsgIndex],
                message, strSize );
            pClient->nextMsgIndex += strSize;
            break;
        }
    }
	free(message_ptr); /* message itself can get incremented, hence need message_ptr for free() */
}


/* 
 * logClientSend ()
 */
void epicsShareAPI logClientSend ( logClientId id, const char * message )
{
    logClient * pClient = ( logClient * ) id;

	/* we have got called during an IOC shutdown, checking   pClient->shutdown
	should help as it will either be 1 (set during logClientDestroy) or some
	random value (after free() is called in logClientDestroy) but it is
	unlikely to be 0 in either case */
	if ( ! pClient || ! message || pClient->shutdown ) {
        return;
    }

    epicsMutexMustLock ( pClient->mutex );

    if (logClientPrefix) {
        sendMessageChunk(pClient, logClientPrefix);
    }
    sendMessageChunk(pClient, message);

    epicsMutexUnlock (pClient->mutex);
}


void epicsShareAPI logClientFlush ( logClientId id )
{
    logClient * pClient = ( logClient * ) id;

    if ( ! pClient ) {
        return;
    }

    epicsMutexMustLock ( pClient->mutex );

    while ( pClient->nextMsgIndex && pClient->connected ) {
        int status = send ( pClient->sock, pClient->msgBuf, 
            pClient->nextMsgIndex, 0 );
        if ( status > 0 ) {
            unsigned nSent = (unsigned) status;
            if ( nSent < pClient->nextMsgIndex ) {
                unsigned newNextMsgIndex = pClient->nextMsgIndex - nSent;
                memmove ( pClient->msgBuf, & pClient->msgBuf[nSent], 
                    newNextMsgIndex );
                pClient->nextMsgIndex = newNextMsgIndex;
            }
            else {
                pClient->nextMsgIndex = 0u;
            }
        }
        else {
            if ( ! pClient->shutdown ) {
                char sockErrBuf[64];
                if ( status ) {
                    epicsSocketConvertErrnoToString ( sockErrBuf, sizeof ( sockErrBuf ) );
                }
                else {
                    strcpy ( sockErrBuf, "server initiated disconnect" );
                }
                fprintf ( stderr, "log client: lost contact with log server at \"%s\" because \"%s\"\n", 
                    pClient->name, sockErrBuf );
            }
            logClientClose ( pClient );
            break;
        }
    }
    epicsMutexUnlock ( pClient->mutex );
}

/*
 *  logClientMakeSock ()
 */
static void logClientMakeSock (logClient *pClient)
{
    
#   ifdef DEBUG
        fprintf (stderr, "log client: creating socket...");
#   endif

    epicsMutexMustLock (pClient->mutex);
   
    /* 
     * allocate a socket 
     */
    pClient->sock = epicsSocketCreate ( AF_INET, SOCK_STREAM, 0 );
    if ( pClient->sock == INVALID_SOCKET ) {
        char sockErrBuf[64];
        epicsSocketConvertErrnoToString ( 
            sockErrBuf, sizeof ( sockErrBuf ) );
        fprintf ( stderr, "log client: no socket error %s\n", 
            sockErrBuf );
    }
    
    epicsMutexUnlock (pClient->mutex);

#   ifdef DEBUG
        fprintf (stderr, "done\n");
#   endif

}

/*
 *  logClientConnect()
 */
static void logClientConnect (logClient *pClient)
{
    osiSockIoctl_t  optval;
    int             errnoCpy;
    int             status;
   
    if ( pClient->sock == INVALID_SOCKET ) {
        logClientMakeSock ( pClient );
        if ( pClient->sock == INVALID_SOCKET ) {
            return;
        }
    }
    
    while ( 1 ) {
        status = connect (pClient->sock, 
            (struct sockaddr *)&pClient->addr, sizeof(pClient->addr));
        if ( status >= 0 ) {
            break;
        }
        else {
            errnoCpy = SOCKERRNO;
            if ( errnoCpy == SOCK_EINTR ) {
                continue;
            }
            else if (
                errnoCpy==SOCK_EINPROGRESS ||
                errnoCpy==SOCK_EWOULDBLOCK ) {
                return;
            }
            else if ( errnoCpy==SOCK_EALREADY ) {
                break;
            }
            else {
                if ( pClient->connFailStatus != errnoCpy && ! pClient->shutdown ) {
                    char sockErrBuf[64];
                    epicsSocketConvertErrnoToString (
                        sockErrBuf, sizeof ( sockErrBuf ) );
                    fprintf (stderr,
                        "log client: failed to connect to \"%s\" because %d=\"%s\"\n", 
                        pClient->name, errnoCpy, sockErrBuf);
                    pClient->connFailStatus = errnoCpy;
                }
                logClientClose ( pClient );
                return;
            }
        }
    }

    epicsMutexMustLock (pClient->mutex);

    pClient->connected = 1u;
    pClient->connFailStatus = 0;

    /*
     * discover that the connection has expired
     * (after a long delay)
     */
    optval = TRUE;
    status = setsockopt (pClient->sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof(optval));
    if (status<0) {
        char sockErrBuf[64];
        epicsSocketConvertErrnoToString ( 
            sockErrBuf, sizeof ( sockErrBuf ) );
        fprintf (stderr, "log client: unable to enable keepalive option because \"%s\"\n", sockErrBuf);
    }

    /*
     * we don't need full-duplex (clients only write), so we shutdown
     * the read end of our socket
     */
    status = shutdown (pClient->sock, SHUT_RD);
    if (status < 0) {
        char sockErrBuf[64];
        epicsSocketConvertErrnoToString ( 
            sockErrBuf, sizeof ( sockErrBuf ) );
        fprintf (stderr, "%s:%d shutdown(%d,SHUT_RD) error was \"%s\"\n", 
            __FILE__, __LINE__, pClient->sock, sockErrBuf);
        /* not fatal (although it shouldn't happen) */
    }

    /*
     * set how long we will wait for the TCP state machine
     * to clean up when we issue a close(). This
     * guarantees that messages are serialized when we
     * switch connections.
     */
    {
        struct  linger      lingerval;
        
        lingerval.l_onoff = TRUE;
        lingerval.l_linger = 60*5; 
        status = setsockopt (pClient->sock, SOL_SOCKET, SO_LINGER, (char *) &lingerval, sizeof(lingerval));
        if (status<0) {
            char sockErrBuf[64];
            epicsSocketConvertErrnoToString ( 
                sockErrBuf, sizeof ( sockErrBuf ) );
            fprintf (stderr, "log client: unable to set linger options because \"%s\"\n", sockErrBuf);
        }
    }
    
    pClient->connectCount++;

    epicsMutexUnlock ( pClient->mutex );
    
    epicsEventSignal ( pClient->stateChangeNotify );

    fprintf ( stderr, "log client: connected to log server at \"%s\"\n", pClient->name );
}

/*
 * logClientRestart ()
 */
static void logClientRestart ( logClientId id )
{
    logClient *pClient = (logClient *)id;

    /* SMP safe state inspection */
    epicsMutexMustLock ( pClient->mutex );
    while ( ! pClient->shutdown ) {
        unsigned isConn;

        isConn = pClient->connected;

        epicsMutexUnlock ( pClient->mutex );

        if ( isConn ) {
            logClientFlush ( pClient );
        }
        else {
            logClientConnect ( pClient );
        }
        
        epicsThreadSleep ( LOG_RESTART_DELAY );

        epicsMutexMustLock ( pClient->mutex );
    }
    epicsMutexUnlock ( pClient->mutex );

    pClient->shutdownConfirm = 1u;
    epicsEventSignal ( pClient->stateChangeNotify );
}

/*
 *  logClientCreate()
 */
logClientId epicsShareAPI logClientCreate (
    struct in_addr server_addr, unsigned short server_port)
{
    epicsTimeStamp begin, current;
    logClient *pClient;
    double diff;

    pClient = calloc (1, sizeof (*pClient));
    if (pClient==NULL) {
        return NULL;
    }

    pClient->addr.sin_family = AF_INET;
    pClient->addr.sin_addr = server_addr;
    pClient->addr.sin_port = htons(server_port);
    ipAddrToDottedIP (&pClient->addr, pClient->name, sizeof(pClient->name));

    pClient->mutex = epicsMutexCreate ();
    if ( ! pClient->mutex ) {
        free ( pClient );
        return NULL;
    }

    pClient->sock = INVALID_SOCKET;
    pClient->connected = 0u;
    pClient->connFailStatus = 0;
    pClient->shutdown = 0;
    pClient->shutdownConfirm = 0;

    epicsAtExit (logClientDestroy, (void*) pClient);
    
    pClient->stateChangeNotify = epicsEventCreate (epicsEventEmpty);
    if ( ! pClient->stateChangeNotify ) {
        epicsMutexDestroy ( pClient->mutex );
        free ( pClient );
        return NULL;
    }
   
    pClient->restartThreadId = epicsThreadCreate (
        "logRestart", epicsThreadPriorityLow, 
        epicsThreadGetStackSize(epicsThreadStackSmall),
        logClientRestart, pClient );
    if ( pClient->restartThreadId == NULL ) {
        epicsMutexDestroy ( pClient->mutex );
        epicsEventDestroy ( pClient->stateChangeNotify );
        free (pClient);
        fprintf(stderr, "log client: unable to start log client connection watch dog thread\n");
        return NULL;
    }

    /*
     * attempt to synchronize with circuit connect
     */
    epicsTimeGetCurrent ( & begin );
    epicsMutexMustLock ( pClient->mutex );
    do {
        epicsMutexUnlock ( pClient->mutex );
        epicsEventWaitWithTimeout ( 
            pClient->stateChangeNotify, 
            LOG_SERVER_CREATE_CONNECT_SYNC_TIMEOUT / 10.0 ); 
        epicsTimeGetCurrent ( & current );
        diff = epicsTimeDiffInSeconds ( & current, & begin );
        epicsMutexMustLock ( pClient->mutex );
    }
    while ( ! pClient->connected && diff < LOG_SERVER_CREATE_CONNECT_SYNC_TIMEOUT );
    epicsMutexUnlock ( pClient->mutex );

    if ( ! pClient->connected ) {
        fprintf (stderr, "log client create: timed out synchronizing with circuit connect to \"%s\" after %.1f seconds\n",
            pClient->name, LOG_SERVER_CREATE_CONNECT_SYNC_TIMEOUT );
    }
        
    return (void *) pClient;
}

/*
 * logClientShow ()
 */
void epicsShareAPI logClientShow (logClientId id, unsigned level)
{
    logClient *pClient = (logClient *) id;

    if ( pClient->connected ) {
        printf ("log client: connected to log server at \"%s\"\n", pClient->name);
    }
    else {
        printf ("log client: disconnected from log server at \"%s\"\n", pClient->name);
    }

    if (level>1) {
        printf ("log client: sock=%s, connect cycles = %u\n",
            pClient->sock==INVALID_SOCKET?"INVALID":"OK",
            pClient->connectCount);
    }

    if (logClientPrefix) {
        printf ("log client: prefix is \"%s\"\n", logClientPrefix);
    }
}

/*
 * logClientSendMessage (); deprecated
 */
void logClientSendMessage ( logClientId id, const char * message )
{
    if ( !iocLogDisable ) {
        logClientSend (id, message);
    }
}

/*
 * iocLogPrefix()
 */
void epicsShareAPI iocLogPrefix(const char * prefix)
{

    /* If we have already established a log prefix, don't let the user change
     * it.  The iocLogPrefix command is expected to be run from the IOC startup
     * script during initialization; the prefix can't be changed once it has
     * been set.
     */

    if (logClientPrefix) {
        printf ("iocLogPrefix: The prefix was already set to \"%s\" "
            "and can't be changed.\n", logClientPrefix);
        return;
    }

    if (prefix) {
        unsigned prefixLen = strlen(prefix);
        if (prefixLen > 0) {
            char * localCopy = malloc(prefixLen+1);
            strcpy(localCopy, prefix);
            logClientPrefix = localCopy;
        }
    }
}
