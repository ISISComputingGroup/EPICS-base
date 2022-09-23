/*************************************************************************\
* Copyright (c) 2009 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* epicsTime.h */
/* Author Jeffrey O. Hill */

#ifndef epicsTimehInclude
#define epicsTimehInclude

#include <time.h>

#include "shareLib.h"
#include "epicsTypes.h"
#include "osdTime.h"
#include "errMdef.h"

/* The EPICS Epoch is 00:00:00 Jan 1, 1990 UTC */
#define POSIX_TIME_AT_EPICS_EPOCH 631152000u

#ifdef __cplusplus

#include <stdexcept>
#include <ostream>

extern "C" {
#endif

/** \brief EPICS time stamp, for use from C code.
 *
 * Because it uses an unsigned 32-bit integer to hold the seconds count, an
 * epicsTimeStamp can safely represent time stamps until the year 2106.
 */
typedef struct epicsTimeStamp {
    epicsUInt32    secPastEpoch;   /* seconds since 0000 Jan 1, 1990 */
    epicsUInt32    nsec;           /* nanoseconds within second */
} epicsTimeStamp;

/*TS_STAMP is deprecated */
#define TS_STAMP epicsTimeStamp


struct timespec; /* POSIX real time */
struct timeval; /* BSD */

/** \name Return status values
 * epicsTime routines return \c S_time_ error status values:
 * @{
 */
/** \brief Success */
#define epicsTimeOK 0
/** \brief No time provider */
#define S_time_noProvider       (M_time| 1) /*No time provider*/
/** \brief Bad event number */
#define S_time_badEvent         (M_time| 2) /*Bad event number*/
/** \brief Invalid arguments */
#define S_time_badArgs          (M_time| 3) /*Invalid arguments*/
/** \brief Out of memory */
#define S_time_noMemory         (M_time| 4) /*Out of memory*/
/** \brief Provider not synchronized */
#define S_time_unsynchronized   (M_time| 5) /*Provider not synchronized*/
/** \brief Invalid timezone */
#define S_time_timezone         (M_time| 6) /*Invalid timezone*/
/** \brief Time conversion error */
#define S_time_conversion       (M_time| 7) /*Time conversion error*/
/** @} */

/** \name epicsTimeEvent numbers
 * Some special values for eventNumber:
 * @{
 */
#define epicsTimeEventCurrentTime 0
#define epicsTimeEventBestTime -1
#define epicsTimeEventDeviceTime -2
/** @} */

/** \name generalTime functions
 * These are implemented in the "generalTime" framework:
 * @{
 */
/** \brief Get current time into \p *pDest */
LIBCOM_API int epicsStdCall epicsTimeGetCurrent ( epicsTimeStamp * pDest );
/** \brief Get time of event \p eventNumber into \p *pDest */
LIBCOM_API int epicsStdCall epicsTimeGetEvent (
    epicsTimeStamp *pDest, int eventNumber);
/** \brief Get monotonic time into \p *pDest */
LIBCOM_API int epicsTimeGetMonotonic ( epicsTimeStamp * pDest );
/** @} */

/** \name ISR-callable
 * These routines may be called from an Interrupt Service Routine, and
 * will return a value from the last current time or event time provider
 * that sucessfully returned a result from the equivalent non-ISR routine.
 * @{
 */
/** \brief Get current time into \p *pDest (ISR-safe) */
LIBCOM_API int epicsTimeGetCurrentInt(epicsTimeStamp *pDest);
/** \brief Get time of event \p eventNumber into \p *pDest (ISR-safe) */
LIBCOM_API int epicsTimeGetEventInt(epicsTimeStamp *pDest, int eventNumber);
/** @} */

/** \name ANSI C time_t conversions
 * Convert to and from ANSI C \c time_t
 * @{
 */
/** \brief Convert epicsTimeStamp to ANSI C \c time_t */
LIBCOM_API int epicsStdCall epicsTimeToTime_t (
    time_t * pDest, const epicsTimeStamp * pSrc );
/** \brief Convert ANSI C \c time_t to epicsTimeStamp */
LIBCOM_API int epicsStdCall epicsTimeFromTime_t (
    epicsTimeStamp * pDest, time_t src );
/** @} */

/** \name ANSI C struct tm conversions
 * Convert to and from ANSI C's <tt>struct tm</tt> with nanoseconds
 * @{
 */
/** \brief Convert epicsTimeStamp to <tt>struct tm</tt> in local time zone */
LIBCOM_API int epicsStdCall epicsTimeToTM (
    struct tm * pDest, unsigned long * pNSecDest, const epicsTimeStamp * pSrc );
/** \brief Convert epicsTimeStamp to <tt>struct tm</tt> in UTC/GMT */
LIBCOM_API int epicsStdCall epicsTimeToGMTM (
    struct tm * pDest, unsigned long * pNSecDest, const epicsTimeStamp * pSrc );
/** \brief Set epicsTimeStamp from <tt>struct tm</tt> in local time zone */
LIBCOM_API int epicsStdCall epicsTimeFromTM (
    epicsTimeStamp * pDest, const struct tm * pSrc, unsigned long nSecSrc );
/** \brief Set epicsTimeStamp from <tt>struct tm</tt> in UTC/GMT */
LIBCOM_API int epicsStdCall epicsTimeFromGMTM (
    epicsTimeStamp * pDest, const struct tm * pSrc, unsigned long nSecSrc );
/** @} */

/** \name POSIX RT struct timespec conversions
 * Convert to and from the POSIX RealTime <tt>struct timespec</tt>
 * format.
 * @{ */
/** \brief Convert epicsTimeStamp to <tt>struct timespec</tt> */
LIBCOM_API int epicsStdCall epicsTimeToTimespec (
    struct timespec * pDest, const epicsTimeStamp * pSrc );
/** \brief Set epicsTimeStamp from <tt>struct timespec</tt> */
LIBCOM_API int epicsStdCall epicsTimeFromTimespec (
    epicsTimeStamp * pDest, const struct timespec * pSrc );
/** @} */

/** \name BSD's struct timeval conversions
 * Convert to and from the BSD <tt>struct timeval</tt> format.
 * @{ */
/** \brief Convert epicsTimeStamp to <tt>struct timeval</tt> */
LIBCOM_API int epicsStdCall epicsTimeToTimeval (
    struct timeval * pDest, const epicsTimeStamp * pSrc );
/** \brief Set epicsTimeStamp from <tt>struct timeval</tt> */
LIBCOM_API int epicsStdCall epicsTimeFromTimeval (
    epicsTimeStamp * pDest, const struct timeval * pSrc );
/** @} */

/** \name Arithmetic operations
 * Arithmetic operations on epicsTimeStamp objects and time differences
 * which are normally expressed as a \c double in seconds.
 * @{ */
/** \brief Time difference between \p left and \p right in seconds. */
LIBCOM_API double epicsStdCall epicsTimeDiffInSeconds (
    const epicsTimeStamp * pLeft, const epicsTimeStamp * pRight );/* left - right */
/** \brief Add some number of seconds to \p dest */
LIBCOM_API void epicsStdCall epicsTimeAddSeconds (
    epicsTimeStamp * pDest, double secondsToAdd ); /* adds seconds to *pDest */
/** \brief Time difference between \p left and \p right, as a signed integer
 * number of nanoseconds.
 * @since EPICS 7.0.6.1
 */
LIBCOM_API epicsInt64 epicsStdCall epicsTimeDiffInNS (
    const epicsTimeStamp *pLeft, const epicsTimeStamp *pRight);
/** @} */

/** \name Comparison operators
 * Comparisons between epicsTimeStamp objects, returning 0=false, 1=true.
 * @{ */
/** \brief \p left equals \p right */
LIBCOM_API int epicsStdCall epicsTimeEqual (
    const epicsTimeStamp * pLeft, const epicsTimeStamp * pRight);
/** \brief \p left not equal to \p right */
LIBCOM_API int epicsStdCall epicsTimeNotEqual (
    const epicsTimeStamp * pLeft, const epicsTimeStamp * pRight);
/** \brief \p left was before \p right */
LIBCOM_API int epicsStdCall epicsTimeLessThan (
    const epicsTimeStamp * pLeft, const epicsTimeStamp * pRight);
/** \brief \p right was no later than \p left */
LIBCOM_API int epicsStdCall epicsTimeLessThanEqual (
    const epicsTimeStamp * pLeft, const epicsTimeStamp * pRight);
/** \brief \p left was after \p right */
LIBCOM_API int epicsStdCall epicsTimeGreaterThan (
    const epicsTimeStamp * pLeft, const epicsTimeStamp * pRight);
/** \brief \p right was not before \p left */
LIBCOM_API int epicsStdCall epicsTimeGreaterThanEqual (
    const epicsTimeStamp * pLeft, const epicsTimeStamp * pRight);
/** @} */

/** \brief Convert epicsTimeStamp to string. See epicsTime::strftime() */
LIBCOM_API size_t epicsStdCall epicsTimeToStrftime (
    char * pBuff, size_t bufLength, const char * pFormat, const epicsTimeStamp * pTS );

/** \brief Dump current state to stdout */
LIBCOM_API void epicsStdCall epicsTimeShow (
    const epicsTimeStamp *, unsigned interestLevel );

/** \name Re-entrant time_t to struct tm conversions
 * OS-specific reentrant versions of the ANSI C interface because the
 * vxWorks \c gmtime_r interface does not match POSIX standards
 * @{ */
/** \brief Break down a \c time_t into a <tt>struct tm</tt> in the local timezone */
LIBCOM_API int epicsStdCall epicsTime_localtime ( const time_t * clock, struct tm * result );
/** \brief Break down a \c time_t into a <tt>struct tm</tt> in the UTC timezone */
LIBCOM_API int epicsStdCall epicsTime_gmtime ( const time_t * clock, struct tm * result );
/** @} */

/** \name Monotonic time routines
* @{ */
/** \brief Monotonic time resolution, may not be accurate. Returns the
 * minimum non-zero time difference between two calls to epicsMonotonicGet()
 * in units of nanoseconds.
 */
LIBCOM_API epicsUInt64 epicsMonotonicResolution(void);
/** \brief Fetch monotonic counter, returns the number of nanoseconds since
 * some unspecified time. */
LIBCOM_API epicsUInt64 epicsMonotonicGet(void);
/** @} */

#ifdef EPICS_EXPOSE_LIBCOM_MONOTONIC_PRIVATE
LIBCOM_API void osdMonotonicInit(void);
#endif

#ifdef __cplusplus
} // extern "C"

/*
 * extend ANSI C RTL "struct tm" to include nano seconds within a second
 * and a struct tm that is adjusted for the local timezone
 */
struct local_tm_nano_sec {
    struct tm ansi_tm; /* ANSI C time details */
    unsigned long nSec; /* nano seconds extension */
};

/*
 * extend ANSI C RTL "struct tm" to includes nano seconds within a second
 * and a struct tm that is adjusted for GMT (UTC)
 */
struct gm_tm_nano_sec {
    struct tm ansi_tm; /* ANSI C time details */
    unsigned long nSec; /* nano seconds extension */
};

/*
 * wrapping this in a struct allows conversion to and
 * from ANSI time_t but does not allow unexpected
 * conversions to occur
 */
struct time_t_wrapper {
    time_t ts;
};

class epicsShareClass epicsTimeEvent
{
public:
    epicsTimeEvent (const int &number) :eventNumber(number) {}
    operator int () const { return eventNumber; }
private:
    int eventNumber;
};

class epicsShareClass epicsTime
{
    // translate S_time_* code to exception
    static void throwError(int code);
public:
    /* exceptions */
    typedef std::runtime_error unableToFetchCurrentTime;
    typedef std::logic_error formatProblemWithStructTM;

#if __cplusplus>=201103L
    constexpr epicsTime() :ts{} {}
#else
    epicsTime () {
        ts.secPastEpoch = ts.nsec = 0u;
    }
#endif

    static inline epicsTime getEvent ( const epicsTimeEvent & evt) ;
    static epicsTime getCurrent ();
    static epicsTime getMonotonic () {
        epicsTime ret;
        epicsTimeGetMonotonic(&ret.ts); // can't fail
        return  ret;
    }

    /* convert to and from EPICS epicsTimeStamp format */
    operator const epicsTimeStamp& () const { return ts; }
    epicsTime ( const epicsTimeStamp & replace );
    epicsTime & operator = ( const epicsTimeStamp & replace) {
        ts = replace;
        return *this;
    }

    /* convert to and from ANSI time_t */
    operator time_t_wrapper () const {
        time_t_wrapper ret;
        throwError(epicsTimeToTime_t(&ret.ts, &ts));
        return ret;
    }
    epicsTime ( const time_t_wrapper & replace ) {
        throwError(epicsTimeFromTime_t(&ts, replace.ts));
    }
    epicsTime & operator = ( const time_t_wrapper & replace) {
        throwError(epicsTimeFromTime_t(&ts, replace.ts));
        return *this;
    }

    /*
     * convert to and from ANSI Cs "struct tm" (with nano seconds)
     * adjusted for the local time zone
     */
    operator local_tm_nano_sec () const {
        local_tm_nano_sec ret;
        throwError(epicsTimeToTM(&ret.ansi_tm, 0, &ts));
        ret.nSec = ts.nsec;
        return ret;
    }
    epicsTime ( const local_tm_nano_sec & replace) {
        throwError(epicsTimeFromTM(&ts, &replace.ansi_tm, replace.nSec));
    }
    epicsTime & operator = ( const local_tm_nano_sec & replace) {
        throwError(epicsTimeFromTM(&ts, &replace.ansi_tm, replace.nSec));
        return *this;
    }

    /*
     * convert to and from ANSI Cs "struct tm" (with nano seconds)
     * adjusted for GM time (UTC)
     */
    operator gm_tm_nano_sec () const {
        gm_tm_nano_sec ret;
        throwError(epicsTimeToGMTM(&ret.ansi_tm, 0, &ts));
        ret.nSec = ts.nsec;
        return ret;
    }
    epicsTime ( const gm_tm_nano_sec & replace) {
        throwError(epicsTimeFromGMTM(&ts, &replace.ansi_tm, replace.nSec));
    }
    epicsTime & operator = ( const gm_tm_nano_sec & replace) {
        throwError(epicsTimeFromGMTM(&ts, &replace.ansi_tm, replace.nSec));
        return *this;
    }

    /* convert to and from POSIX RTs "struct timespec" */
    operator struct timespec () const {
        timespec ret;
        epicsTimeToTimespec(&ret, &ts);
        return ret;
    }
    epicsTime ( const struct timespec & replace) {
        throwError(epicsTimeFromTimespec(&ts, &replace));
    }
    epicsTime & operator = ( const struct timespec & replace ) {
        throwError(epicsTimeFromTimespec(&ts, &replace));
        return *this;
    }

    /* convert to and from BSDs "struct timeval" */
    operator struct timeval () const ;
    epicsTime ( const struct timeval & replace);
    epicsTime & operator = ( const struct timeval & replace);

#ifdef _WIN32
    /* convert to and from WIN32s FILETIME (implemented only on WIN32) */
    operator struct _FILETIME () const;
    /** \brief Construct from Windows <tt>struct _FILETIME</tt> */
    epicsTime ( const struct _FILETIME & );
    epicsTime & operator = ( const struct _FILETIME & );
#endif /* _WIN32 */

    /** \name Arithmetic operators
     * Standard operators involving epicsTime objects and time differences
     * which are always expressed as a \c double in seconds.
     * @{ */
     /// \brief \p lhs minus \p rhs, in seconds
    double operator- ( const epicsTime & other) const {
        return epicsTimeDiffInSeconds(&ts, &other.ts);
    }
     /// \brief \p lhs plus rhs seconds
    epicsTime operator+ (double delta) const {
        epicsTime ret(*this);
        epicsTimeAddSeconds(&ret.ts, delta);
        return ret;
    }
     /// \brief \p lhs minus rhs seconds
    epicsTime operator- (double delta ) const {
        return (*this)+(-delta);
    }
     /// \brief add rhs seconds to  \p lhs
    epicsTime operator+= (double delta) {
        epicsTimeAddSeconds(&ts, delta);
        return *this;
    }
     /// \brief subtract rhs seconds from \p lhs
    epicsTime operator-= ( double delta ) {
        return (*this) += (-delta);
    }
    /** @} */

    /** \name Comparison operators
     * Standard comparisons between epicsTime objects.
     * @{ */
     /// \brief \p lhs equals \p rhs
    bool operator == ( const epicsTime & other) const {
        return epicsTimeEqual(&ts, &other.ts);
    }
     /// \brief \p lhs not equal to \p rhs
     bool operator != ( const epicsTime & other) const {
         return epicsTimeNotEqual(&ts, &other.ts);
     }
     /// \brief \p rhs no later than \p lhs
     bool operator <= ( const epicsTime & other) const {
         return epicsTimeLessThanEqual(&ts, &other.ts);
     }
     /// \brief \p lhs was before \p rhs
     bool operator < ( const epicsTime & other) const {
         return epicsTimeLessThan(&ts, &other.ts);
     }
     /// \brief \p rhs not before \p lhs
     bool operator >= ( const epicsTime & other) const {
         return epicsTimeGreaterThanEqual(&ts, &other.ts);
     }
     /// \brief \p lhs was after \p rhs
     bool operator > ( const epicsTime & other) const {
         return epicsTimeGreaterThan(&ts, &other.ts);
     }
    /** @} */

    /* convert current state to user-specified string */
     size_t strftime ( char * pBuff, size_t bufLength, const char * pFormat ) const {
         return epicsTimeToStrftime(pBuff, bufLength, pFormat, &ts);
     }

    /* dump current state to standard out */
     void show ( unsigned interestLevel ) const {
         epicsTimeShow(&ts, interestLevel);
     }

private:
    epicsTimeStamp ts;
};
LIBCOM_API
std::ostream& operator<<(std::ostream& strm, const epicsTime& ts);

#endif /* __cplusplus */



#endif /* epicsTimehInclude */

