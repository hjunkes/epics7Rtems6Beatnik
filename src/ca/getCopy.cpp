
/*  
 *  $Id$
 *
 *                              
 *                    L O S  A L A M O S
 *              Los Alamos National Laboratory
 *               Los Alamos, New Mexico 87545
 *                                  
 *  Copyright, 1986, The Regents of the University of California.
 *                                  
 *           
 *	Author Jeffrey O. Hill
 *	johill@lanl.gov
 *	505 665 1831
 */

#include "iocinf.h"
#include "oldAccess.h"
#include "cac_IL.h"

tsFreeList < class getCopy, 1024 > getCopy::freeList;
epicsMutex getCopy::freeListMutex;

getCopy::getCopy ( cac &cacCtxIn, unsigned typeIn, 
        unsigned long countIn, void *pValueIn ) :
    count ( countIn ), cacCtx ( cacCtxIn ), pValue ( pValueIn ), 
        readSeq ( cacCtxIn.sequenceNumberOfOutstandingIO () ), type ( typeIn )
{
    cacCtxIn.incrementOutstandingIO ();
}

getCopy::~getCopy () 
{
}

void getCopy::release () 
{
    delete this;
}

void getCopy::completionNotify ( cacChannelIO &io )
{
    this->cacNotify::completionNotify ( io );
}

void getCopy::completionNotify ( cacChannelIO &chan, unsigned typeIn, 
    unsigned long countIn, const void *pDataIn )
{
    if  ( this->type == typeIn ) {
        memcpy ( this->pValue, pDataIn, dbr_size_n ( typeIn, countIn ) );
        this->cacCtx.decrementOutstandingIO ( this->readSeq );
    }
    else {
        this->exceptionNotify ( chan, ECA_INTERNAL, 
            "bad data type match in get copy back response" );
    }
}


void getCopy::exceptionNotify ( cacChannelIO &chan, 
    int status, const char *pContext )
{
    this->cacNotify::exceptionNotify ( chan, status, pContext );
}

void getCopy::exceptionNotify ( cacChannelIO &chan, 
    int status, const char *pContext, unsigned typeIn, unsigned long countIn )
{
    this->cacNotify::exceptionNotify ( chan, status, pContext, typeIn, countIn );
}

void getCopy::show ( unsigned level ) const
{
    int tmpType = static_cast <int> ( this->type );
    printf ( "read copy IO at %p, type %s, element count %lu\n", 
        static_cast <const void *> ( this ), dbf_type_to_text ( tmpType ), this->count );
    if ( level > 0u ) {
        printf ( "\tsequence number %u, user's storage %p\n",
            this->readSeq, static_cast <const void *> ( this->pValue ) );
    }
}
