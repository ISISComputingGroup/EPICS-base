/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/*
 *      Author  W. Eric Norum
 *              norume@aps.anl.gov
 *              630 252 4793
 */

/*
 * Interthread message passing
 */
#ifndef epicsMessageQueueh
#define epicsMessageQueueh

#include "epicsAssert.h"
#include "shareLib.h"

typedef struct epicsMessageQueueOSD *epicsMessageQueueId;

#ifdef __cplusplus

class epicsShareClass epicsMessageQueue {
public:
    epicsMessageQueue ( unsigned int capacity,
                        unsigned int maximumMessageSize );
    ~epicsMessageQueue ();
    int trySend ( void *message, unsigned int messageSize );

    /**
     *  \brief Send a message.
     *  \returns 0 if the message was sent to a receiver or queued for
     *  future delivery.
     *  \returns -1 if the message is larger than the queue's maximum
     *  message size.
     **/
    int send ( void *message, unsigned int messageSize );

    /**
     *  \brief Send a message or timeout.
     *  \param message Pointer to the message to be sent
     *  \param messageSize The size of \p message
     *  \param timeout The timeout delay in seconds. A timeout of zero is
     *  equivalent to calling trySend(); NaN or any value too large to be
     *  represented to the target OS is equivalent to no timeout.
     *  \returns 0 if the message was sent to a receiver or queued for
     *  future delivery.
     *  \returns -1 if the timeout was reached before the
     *  message could be sent or queued, or if the message is larger
     *  than the queue's maximum message size.
     **/
    int send ( void *message, unsigned int messageSize, double timeout );

    /**
     *  \brief Try to receive a message.
     *  \param[out] message Output buffer to store the received message
     *  \param size Size of the buffer pointed to by \p message
     *
     *  If the queue holds at least one message,
     *  the first message on the queue is moved to the specified location
     *  and the length of that message is returned.
     *
     *  If the received message is larger than the specified message size
     *  the implementation may either return -1, or truncate the
     *  message. It is most efficient if the messageBufferSize is equal
     *  to the maximumMessageSize with which the message queue was
     *  created.
     *  \returns Number of bytes in the message.
     *  \returns -1 if the message queue is empty, or the buffer too small.
     **/
    int tryReceive ( void *message, unsigned int size );

    /**
     *  \brief Fetch the next message on the queue.
     *  \param[out] message Output buffer to store the received message
     *  \param size Size of the buffer pointed to by \p message
     *
     *  Wait for a message to be sent if the queue is empty, then move
     *  the first message queued to the specified location.
     *
     *  If the received message is larger than the specified message size
     *  the implementation may either return -1, or truncate the
     *  message. It is most efficient if the messageBufferSize is equal
     *  to the maximumMessageSize with which the message queue was
     *  created.
     *  \returns Number of bytes in the message.
     *  \returns -1 if the buffer is too small for the message.
     **/
    int receive ( void *message, unsigned int size );

    /**
     *  \brief Wait for and fetch the next message.
     *  \param[out] message Output buffer to store the received message
     *  \param size Size of the buffer pointed to by \p message
     *  \param timeout The timeout delay in seconds. A timeout of zero is
     *  equivalent to calling tryReceive(); NaN or any value too large to
     *  be represented to the target OS is equivalent to no timeout.
     *
     *  Waits up to \p timeout seconds for a message to arrive if the queue
     *  is empty, then moves the first message to the message buffer.
     *
     *  If the received message is larger than the buffer size
     *  the implementation may either return -1, or truncate the
     *  message. It is most efficient if the messageBufferSize is equal
     *  to the maximumMessageSize with which the message queue was
     *  created.
     *  \returns Number of bytes in the message.
     *  \returns -1 if a message is not received within the timeout
     *  interval.
     **/
    int receive ( void *message, unsigned int size, double timeout );
    void show ( unsigned int level = 0 );
    unsigned int pending ();

private: /* Prevent compiler-generated member functions */
    /* default constructor, copy constructor, assignment operator */
    epicsMessageQueue();
    epicsMessageQueue(const epicsMessageQueue &);
    epicsMessageQueue& operator=(const epicsMessageQueue &);

    epicsMessageQueueId id;
};

extern "C" {
#endif /*__cplusplus */

epicsShareFunc epicsMessageQueueId epicsShareAPI epicsMessageQueueCreate(
    unsigned int capacity,
    unsigned int maximumMessageSize);
epicsShareFunc void epicsShareAPI epicsMessageQueueDestroy(
    epicsMessageQueueId id);
epicsShareFunc int epicsShareAPI epicsMessageQueueTrySend(
    epicsMessageQueueId id,
    void *message,
    unsigned int messageSize);
epicsShareFunc int epicsShareAPI epicsMessageQueueSend(
    epicsMessageQueueId id,
    void *message,
    unsigned int messageSize);
epicsShareFunc int epicsShareAPI epicsMessageQueueSendWithTimeout(
    epicsMessageQueueId id,
    void *message,
    unsigned int messageSize,
    double timeout);
epicsShareFunc int epicsShareAPI epicsMessageQueueTryReceive(
    epicsMessageQueueId id,
    void *message,
    unsigned int size);
epicsShareFunc int epicsShareAPI epicsMessageQueueReceive(
    epicsMessageQueueId id,
    void *message,
    unsigned int size);
epicsShareFunc int epicsShareAPI epicsMessageQueueReceiveWithTimeout(
    epicsMessageQueueId id,
    void *message,
    unsigned int size,
    double timeout);
epicsShareFunc int epicsShareAPI epicsMessageQueuePending(
    epicsMessageQueueId id);
epicsShareFunc void epicsShareAPI epicsMessageQueueShow(
    epicsMessageQueueId id,
    int level);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#include "osdMessageQueue.h"

#endif /* epicsMessageQueueh */
