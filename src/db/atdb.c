/*atodb.c*/
/* share/src/db $Id$ */
/*
 *
 *     Author:	Marty Kraimer
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01	07-13-93	mrk	Original version
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <errMdef.h>
#include <dbDefs.h>
#include <dbStaticLib.h>
DBBASE	*pdbbase;
DBENTRY	*pdbentry;

main(int argc,char **argv)
{
    FILE    	*fpdctsdr;
    FILE	*fp;
    long	status;
    char	*ptime;
    time_t	timeofday;
    char	*rectype;
    

    if(argc!=3) {
	printf("Usage: atdb <dctsdr file> <database file>\n");
	exit(-1);
    }
    fpdctsdr = fopen(argv[1],"r");
    if(!fpdctsdr) {
	errMessage(0,"Error opening file");
	exit(-1);
    }
    pdbbase=dbAllocBase();
    pdbentry=dbAllocEntry(pdbbase);
    status=dbRead(pdbbase,fpdctsdr);
    if(status) {
	errMessage(status,"dbRead");
	exit(-1);
    }
    yyreset();
    yyparse();
    fp=fopen(argv[2],"w");
    if(fp==NULL) {
    	errMessage(0,"Error opening file");
    	exit(-1);
    }
    status = dbWrite(pdbbase,fpdctsdr,fp);
    if(status) errMessage(status,"dbWrite");
    fclose(fpdctsdr);
    fclose(fp);
    dbFreeEntry(pdbentry);
    dbFreeBase(pdbbase);
    return(0);
}
