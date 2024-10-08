/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

#ifndef epicsMathh
#define epicsMathh

#include <math.h>
#include <float.h>
#include <libComAPI.h>

#ifndef finite
#define finite(D) _finite(D)
#endif

#ifndef __cplusplus

/* these are macros in C but inline functions in C++
 * so ifndef does not work as a check in c++ 
 * and in a recent VS2022 update causes a conflict and
 * compile time error 
 */

#ifndef isnan
#define isnan(D) _isnan(D)
#endif

#ifndef isinf
#define isinf(D) ( !_finite(D) && !_isnan(D) )
#endif

#endif /* ifndef __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

LIBCOM_API extern const float epicsNAN;
LIBCOM_API extern const float epicsINF;

#ifdef __cplusplus
}
#endif

#endif /* epicsMathh */
