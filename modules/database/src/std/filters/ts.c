/*************************************************************************\
* Copyright (c) 2010 Brookhaven National Laboratory.
* Copyright (c) 2010 Helmholtz-Zentrum Berlin
*     fuer Materialien und Energie GmbH.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

/*
 *  Author: Ralph Lange <Ralph.Lange@bessy.de>
 */

#include <stdio.h>

#include <chfPlugin.h>
#include <dbLock.h>
#include <db_field_log.h>
#include <epicsExport.h>

static db_field_log* filter(void* pvt, dbChannel *chan, db_field_log *pfl) {
    epicsTimeStamp now;
    epicsTimeGetCurrent(&now);

    /* If reference and not already copied,
       must make a copy (to ensure coherence between time and data) */
    if (pfl->type == dbfl_type_ref && !pfl->dtor) {
        void *pTarget = calloc(pfl->no_elements, pfl->field_size);
        void *pSource = pfl->u.r.field;
        if (pTarget) {
            long offset = 0;
            long nSource = pfl->no_elements;
            dbScanLock(dbChannelRecord(chan));
            dbChannelGetArrayInfo(chan, &pSource, &nSource, &offset);
            dbExtractArray(pSource, pTarget, pfl->field_size,
                nSource, pfl->no_elements, offset, 1);
            pfl->u.r.field = pTarget;
            pfl->dtor = freeArray;
            pfl->u.r.pvt = pvt;
            dbScanUnlock(dbChannelRecord(chan));
        }
    }

    pfl->time = now;
    return pfl;
}

static void channelRegisterPre(dbChannel *chan, void *pvt,
                               chPostEventFunc **cb_out, void **arg_out, db_field_log *probe)
{
    *cb_out = filter;
}

static void channel_report(dbChannel *chan, void *pvt, int level, const unsigned short indent)
{
    printf("%*sTimestamp (ts)\n", indent, "");
}

static chfPluginIf pif = {
    NULL, /* allocPvt, */
    NULL, /* freePvt, */

    NULL, /* parse_error, */
    NULL, /* parse_ok, */

    NULL, /* channel_open, */
    channelRegisterPre,
    NULL, /* channelRegisterPost, */
    channel_report,
    NULL /* channel_close */
};

static void tsInitialize(void)
{
    chfPluginRegister("ts", &pif, NULL);
}

epicsExportRegistrar(tsInitialize);
