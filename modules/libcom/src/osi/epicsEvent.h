/*************************************************************************\
* Copyright (c) 2011 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/**\file epicsEvent.h
 *
 * \brief APIs for the epicsEvent binary semaphore.
 *
 * Defines the C++ and C API's for a simple binary semaphore. If multiple threads are
 * waiting on the same event, only one of them will be woken when the event is signaled.
 *
 * The primary use of an event semaphore is for thread synchronization. An example of using an
 * event semaphore is a consumer thread that processes requests from one or more producer threads.
 * For example:
 *
 * When creating the consumer thread also create an epicsEvent.
 \code
   epicsEvent event;
 \endcode
 * The consumer thread has code containing:
 \code
       while(1) {
           pevent.wait();
           while( {more work} ) {
               {process work}
           }
       }
 \endcode
 * Producers create requests and issue the statement:
 \code
       pevent.trigger();
 \endcode
 **/

#ifndef epicsEventh
#define epicsEventh

#include "shareLib.h"

typedef struct epicsEventOSD *epicsEventId;

typedef enum {
    epicsEventOK = 0,
    epicsEventWaitTimeout,
    epicsEventError
} epicsEventStatus;

/* Backwards compatibility */
#define epicsEventWaitStatus epicsEventStatus
#define epicsEventWaitOK epicsEventOK
#define epicsEventWaitError epicsEventError

typedef enum {
    epicsEventEmpty,
    epicsEventFull
} epicsEventInitialState;

#ifdef __cplusplus

class epicsShareClass epicsEvent {
public:
    epicsEvent ( epicsEventInitialState initial = epicsEventEmpty );
    ~epicsEvent ();
    void trigger ();
    void signal () { this->trigger(); }
    /**\brief Wait for the event.
     * \note Blocks until full.
     **/
    void wait ();
    /**\brief Wait for the event or until the specified timeout.
     * \param timeout The timeout delay in seconds. A timeout of zero is
     * equivalent to calling tryWait(); NaN or any value too large to be
     * represented to the target OS is equivalent to no timeout.
     * \return True if the event was triggered, False if it timed out.
     **/
    bool wait ( double timeout );
    /**\brief Similar to wait() except that if the event is currently empty the
     * call will return immediately.
     * \return True if the event was full (triggered), False if empty.
     **/
    bool tryWait ();
    /**\brief Display information about the semaphore.
     * \note The information displayed is architecture dependent.
     * \param level An unsigned int for the level of information to be displayed.
     **/
    void show ( unsigned level ) const;

    class invalidSemaphore;         /* exception payload */
private:
    epicsEvent ( const epicsEvent & );
    epicsEvent & operator = ( const epicsEvent & );
    epicsEventId id;
};

extern "C" {
#endif /*__cplusplus */

epicsShareFunc epicsEventId epicsEventCreate(
    epicsEventInitialState initialState);
epicsShareFunc epicsEventId epicsEventMustCreate (
    epicsEventInitialState initialState);
epicsShareFunc void epicsEventDestroy(epicsEventId id);
epicsShareFunc epicsEventStatus epicsEventTrigger(
    epicsEventId id);
epicsShareFunc void epicsEventMustTrigger(epicsEventId id);
#define epicsEventSignal(ID) epicsEventMustTrigger(ID)
epicsShareFunc epicsEventStatus epicsEventWait(
    epicsEventId id);

/**\brief Wait for an event (see epicsEventWait()).
 *
 * This routine does not return if the identifier is invalid.
 * \param id The event identifier.
 */
LIBCOM_API void epicsEventMustWait(epicsEventId id);

/**\brief Wait an the event or until the specified timeout period is over.
 * \note Blocks until full or timeout.
 * \param id The event identifier.
 * \param timeout The timeout delay in seconds. A timeout of zero is
 * equivalent to calling epicsEventTryWait(); NaN or any value too large
 * to be represented to the target OS is equivalent to no timeout.
 * \return Status indicator.
 **/
LIBCOM_API epicsEventStatus epicsEventWaitWithTimeout(
    epicsEventId id, double timeout);

/**\brief Similar to wait() except that if the event is currently empty the
 * call will return immediately with status \c epicsEventWaitTimeout.
 * \param id The event identifier.
 * \return Status indicator, \c epicsEventWaitTimeout when the event is empty.
 **/
LIBCOM_API epicsEventStatus epicsEventTryWait(
    epicsEventId id);

/**\brief Display information about the semaphore.
 * \note The information displayed is architecture dependent.
 * \param id The event identifier.
 * \param level An unsigned int for the level of information to be displayed.
 **/
LIBCOM_API void epicsEventShow(
    epicsEventId id, unsigned int level);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#include "osdEvent.h"

#endif /* epicsEventh */
