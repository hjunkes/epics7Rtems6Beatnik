/* 
 * Copyright: Stanford University / SLAC National Laboratory.
 *
 * EPICS BASE is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution. 
 *
 * Author: Till Straumann <strauman@slac.stanford.edu>, 2011, 2014
 */ 

#include <stdarg.h>
#include <stdlib.h>

#include "epicsStackTracePvt.h"
#include "epicsThread.h"
#include "epicsMutex.h"
#include "errlog.h"

#define epicsExportSharedSymbols
#include "epicsStackTrace.h"

/* How many stack frames to capture               */
#define MAXDEPTH 100

static epicsThreadOnceId stackTraceInitId = EPICS_THREAD_ONCE_INIT;
static epicsMutexId      stackTraceMtx;

static void stackTraceInit(void *unused)
{
    stackTraceMtx = epicsMutexMustCreate();
}

static void stackTraceLock(void)
{
    epicsThreadOnce( &stackTraceInitId, stackTraceInit, 0 );
    epicsMutexLock( stackTraceMtx );
}

static void stackTraceUnlock(void)
{
    epicsMutexUnlock( stackTraceMtx );
}

static int
dump(char **buf, int *buf_sz, int *good, const char *fmt, ...)
{
va_list ap;
int rval, put;
    va_start(ap, fmt);
        if ( *buf ) {
            put = rval = vsnprintf(*buf, *buf_sz, fmt, ap);
            if ( put > *buf_sz )
                put = *buf_sz;
            *buf    += put;
            *buf_sz -= put;
        } else {
            rval = errlogVprintf(fmt, ap);
        }
    va_end(ap);
    if ( rval > 0 )
        *good += rval;
    return rval;
}

static int
symDump(char *buf, int buf_sz, void *addr, epicsSymbol *sym_p)
{
int rval = 0;

    dump( &buf, &buf_sz, &rval, "[%*p]", sizeof(addr)*2 + 2, addr);
	if ( sym_p ) {
		if ( sym_p->f_nam ) {
			dump( &buf, &buf_sz, &rval, ": %s", sym_p->f_nam );
		}
		if ( sym_p->s_nam ) {
			/* windows didn't grok the void* pointer arithmetic */
			dump( &buf, &buf_sz, &rval, "(%s+0x%lx)", sym_p->s_nam, (unsigned long)((char*)addr - (char*)sym_p->s_val));
		}
	}
	dump( &buf, &buf_sz, &rval, "\n");
	if ( ! buf )
		errlogFlush();

    return rval;
}

void epicsStackTrace(void)
{
void        **buf;
int         i,n;
epicsSymbol sym;

	if ( 0 == epicsStackTraceGetFeatures() ) {
		/* unsupported on this platform */
		return;
	}

    if ( ! (buf = malloc(sizeof(*buf) * MAXDEPTH))) {
        free(buf);
        errlogPrintf("epicsStackTrace(): not enough memory for backtrace\n");
        return;
    }

    n = epicsBackTrace(buf, MAXDEPTH);

	if ( n > 0 ) {

		stackTraceLock();

		errlogPrintf("Dumping a stack trace of thread '%s':\n", epicsThreadGetNameSelf());

		errlogFlush();

		for ( i=0; i<n; i++ ) {
			if ( 0 == epicsFindAddr(buf[i], &sym) )
				symDump(0, 0, buf[i], &sym);
			else
				symDump(0, 0, buf[i], 0);
		}

		errlogFlush();

		stackTraceUnlock();

	}

    free(buf);
}
