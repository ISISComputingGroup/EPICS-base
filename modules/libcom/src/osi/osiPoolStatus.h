/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

#ifndef INC_osiPoolStatus_H
#define INC_osiPoolStatus_H

/*
 * Author: Jeff Hill
 *
 * Functions which interrogate the state of the system wide pool
 *
 */

#include <stdlib.h>
#include "shareLib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * tests to see if there is sufficent space for a block of the requested size
 * along with whatever additional free space is necessary to keep the system running 
 * reliably
 *
 * The meaning of "safely allocated" is target-specific, some additional free
 * space is usually necessary to keep the system running reliably.
 * On vxWorks this returns True if at least 100000 bytes is free.
 *
 * \note This routine is called quite frequently by the IOC so an efficient
 * implementation is important.
 *
 * \param contiguousBlockSize Block size to check.
 * \return True if the requested memory should be available.
 */
epicsShareFunc int epicsShareAPI osiSufficentSpaceInPool ( size_t contiguousBlockSize );

#ifdef __cplusplus
}
#endif

#include "osdPoolStatus.h"

#endif /* INC_osiPoolStatus_H */
