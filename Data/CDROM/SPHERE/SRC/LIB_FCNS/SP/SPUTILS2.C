#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/sputils2.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* LINTLIBRARY */

/** File: sputils2.c **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sp/sphere.h>

extern int farray_fields;
extern struct field_t *farray[];


FUNCTION int strsame( s1, s2 )
char * s1;
char * s2;
{
    return ( strcmp( s1, s2 ) == 0 );
}

FUNCTION int strdiff( s1, s2 )
char * s1;
char * s2;
{
    return ! strsame( s1, s2 );
}

#ifdef mmm
/*****************************************************/
/* Returns positive value if field exists in header, */
/* negative on error, and zero otherwise.            */
/*****************************************************/

FUNCTION int sp_field( h, field )
struct header_t * h;
char * field;
{
    int i, j;
    
    
    if ( h == HDRNULL )	return -1;	/* check sanity of arguments */
    if ( field == CNULL )	return -1;
    
    j = h->fc;
    for ( i=0; i < j; i++ ) {
	if ( strsame( h->fv[i]->name, field ))
	    return 1;
    }
    
    return 0;
}


FUNCTION int sp_integer( h, field, value )
struct header_t * h;
char * field;
long * value;
{
    int type, size;
    int n;
    int v;
    char *buf;
    
    
    if ( h == HDRNULL )		return -1;
    if ( field == CNULL )		return -1;
    if ( value == (long *) NULL )	return -1;
    
    n = sp_get_field( h, field, &type, &size );
    if ( n < 0 )
	return -1;
    
    switch ( type ) {
	
      case T_INTEGER:
	n = sp_get_data( h, field, (char *) &v, &size );
	if ( n < 0 )
	    return -1;
	*value = v;
	return 0;
	
      case T_STRING:
	buf = mtrf_malloc( size + 1 );
	if ( buf == CNULL )
	    return -1;
	
	n = sp_get_data( h, field, buf, &size );
	if ( n < 0 ) {
	    mtrf_free( buf );
	    return -1;
	}
	
	*value = atol( buf );
	mtrf_free( buf );
	return 0;
    }
    
    return -1;
}


/* return -1 if doesn't exist or fails, 1 if pass */
/* The value is a pointer to a character pointer  */
int sp_string(h, field, value)
struct header_t *h;
char *field, **value;
{
    int type, size;
    int n;
    int v;
    char *buf;
    
    if ( h == HDRNULL )			return -1;
    if ( field == CNULL )		return -1;
    if ( value == NULL )		return -1;
    
    n = sp_get_field( h, field, &type, &size );
    if ( n < 0 )
	return -1;
    
    switch ( type ) {
      case T_INTEGER:
	return -1;
	
      case T_STRING:
	buf = mtrf_malloc( size + 1 );
	if ( buf == CNULL )
	    return -1;
	n = sp_get_data( h, field, buf, &size );
        buf[size] = (char )0;
	if ( n < 0 ) {
	    mtrf_free( buf );
	    return -1;
	}
	
	*value = buf ;
	return 0;
    }
    return(1);
}

int sp_pass_checksum(sfc)
struct sphere_file_contents_t *sfc;
{
    unsigned short chk_sum;
    int file_checksum;

    if (sp_integer(sfc->header, "sample_checksum", &(file_checksum)) < 0) {
	fprintf(stderr,"Error: Unable to get checksum\n");
	return(-1);
    } else {
	chk_sum=sp_compute_short_checksum(sfc->sample_data.signed_2byte,
					  sfc->sample_count);
	if (sp_verbose)
	    fprintf(stderr,"Checksum (header,file)(%d-%d) ",file_checksum,chk_sum);
	if (chk_sum == file_checksum){
	    if (sp_verbose) 
		fprintf(stderr,"  Same checksum\n");
	    return(1);
	} else {
	    if (sp_verbose)
		fprintf(stderr,"  FAILED CHECKSUM\n");
	    return(-1);
	}
    }
}

#endif
