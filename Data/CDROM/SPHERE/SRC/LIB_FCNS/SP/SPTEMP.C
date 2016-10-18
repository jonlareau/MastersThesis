#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/sptemp.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* LINTLIBRARY */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sp/sphere.h>

char * sptemp( file )
char * file;
{
    int len;
    char * s;


    if ( file == CNULL )
	return CNULL;
    len = strlen( file ) + strlen( TMPEXT ) + 1;
    s = mtrf_malloc( len );
    if ( s == CNULL )
	return CNULL;

    (void) strcpy( s, file );
    (void) strcat( s, TMPEXT );
    return s;
}

char * sptemp_dirfile()
{
    int max_attempt=999, attempt=0;
    char * s, *n;
    static int call=0;
    struct stat fileinfo;


    do {
	s = rsprintf("%s/%s%d.sph",TEMP_DIR,TEMP_BASE_NAME,call++);
	if (attempt++ >= max_attempt)
	    return(CNULL);
    }  while (stat(s,&fileinfo) == 0);
    if ((n = mtrf_malloc(strlen(s) + 1)) == CNULL)
	return(CNULL);
    strcpy(n,s);
    return(n);
}

