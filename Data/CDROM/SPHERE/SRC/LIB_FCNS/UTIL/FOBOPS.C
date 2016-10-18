/* LINTLIBRARY */


#ifndef lint
    static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/fobops.c,v 1.2 1993/03/25 00:20:51 stan Exp stan $";
#endif


#include <stdio.h>
#include <stdlib.h>
#include <util/chars.h>
#include <util/fob.h>
#include <util/min.h>
#include <util/mtrf.h>

FOB * fob_create( fp )
	FILE * fp;
{
	FOB * fobp;


	fobp = (FOB *) mtrf_malloc( sizeof(FOB) );
	if ( fobp == FOBPNULL )
		return FOBPNULL;

	fobp->fp = fp;
	fobp->length = 0;	
	fobp->read_byte_swap = 0;
	fobp->write_byte_swap = 0;
	fobp->buf_swap = CNULL;

	fobp->buf = CNULL;

	return fobp;
}


int fob_destroy( fobp )
	FOB * fobp;
{
	mtrf_free( (char *) fobp );
	if (fobp->buf != CNULL)
	    mtrf_free (fobp->buf);
	if (fobp->buf_swap != CNULL)
	    mtrf_free (fobp->buf_swap);
	return 0;
}


int fob_fflush( fobp )
FOB * fobp;
{
    if (fobp == FOBPNULL)
	return -1;
    if (fob_is_fp(fobp))
	fflush(fobp->fp);
}


int fob_create2( fpin, fpout, fobpin, fobpout )
	FILE * fpin;
	FILE * fpout;
	FOB ** fobpin;
	FOB ** fobpout;
{
	FOB * new1;
	FOB * new2;


	new1 = fob_create( fpin );
	if ( new1 == FOBPNULL )
		return -1;

	new2 = fob_create( fpout );
	if ( new2 == FOBPNULL ) {
		(void) fob_destroy( new1 );
		return -1;
	}

	*fobpin = new1;
	*fobpout = new2;

	return 0;
}


int fob_destroy2( fobpin, fobpout )
	FOB * fobpin;
	FOB * fobpout;
{
	int n1;
	int n2;


	n1 = fob_destroy( fobpin );
	n2 = fob_destroy( fobpout );
	return ( n1 < 0 || n2 < 0 ) ? -1 : 0;
}

fob_read_byte_swap(f)
FOB *f;
{
    f->read_byte_swap = 1;
}

fob_write_byte_swap(f)
FOB *f;
{
    f->write_byte_swap = 1;
}

fob_bufinit( f, buf, len )
	FOB * f;
	char * buf;
	int len;
{
	if ( len > 0 ) {
		f->buf = buf;
		f->pos = buf;
		f->bufsize = len;
	}
}

void fob_rewind( f )
FOB * f;
{
    if (f->fp != FPNULL){
	rewind(f->fp);
	f->length = 0;
    } else
	f->pos = f->buf;
}

int fob_ftell( f )
FOB * f;
{
    if (f->fp != FPNULL)
	return(ftell(f->fp));
    else
	return(f->pos - f->buf);
}

int fob_is_fp( f )
FOB * f;
{
    if (f->fp != FPNULL)
	return(TRUE);
    return(FALSE);
}

int fob_flush_to_fp( f , fp)
FOB * f;
FILE *fp;
{
    if (f == FOBPNULL)
	return -1;
    if (fp == FPNULL)
	return -1;

    printf("FOB: flushing %d chars\n",f->length);

    if (fwrite(f->buf,1,f->length,fp) != f->length)	
	return -1;
    return 0;
}

/*
 * Sets *len to length of output buffer
 * Sets *buf to output buffer if length is positive
 * Trims buffer to proper length using realloc(), since space
 *	is always allocated in large chunks
 */

fob_bufcleanup( f, buf, len )
	FOB * f;
	char ** buf;
	int * len;
{
	if ( f->buf == CNULL )
		*len = 0;
	else {
		*len = f->pos - f->buf;
		if ( *len > 0 ) {
			*buf = realloc( f->buf, *len );
			if ( *buf == CNULL )
				return -1;
			f->buf = CNULL;
		}
	}
}



int fob_bufput( fobp, p, len )
	FOB * fobp;
	char * p;
	int len;
{
	int nb;
	int space;


	if ( fobp == FOBPNULL )
		return -1;
	if ( p == CNULL )
		return -1;
	if ( len <= 0 )
		return -1;

	if ( fobp->buf == CNULL ) {		/* need at least one block */
/*		fobp->buf = mtrf_malloc( FOB_BUF_GRAN );
		fobp->bufsize = FOB_BUF_GRAN;*/
		if ((fobp->buf = mtrf_malloc( 300000 )) == CNULL)
		    return(-1);
		fobp->bufsize = 300000;
		fobp->pos = fobp->buf;
	}

	nb = fobp->pos - fobp->buf;		/* #bytes stored */
	space = fobp->bufsize - nb;		/* #bytes left */

	if ( len > space ) {			/* if not enough space ... */
		char * newbuf;
		int newsize;

		newsize = fobp->bufsize;
		do {				/* calculate #bytes needed to store */
			space += FOB_BUF_GRAN;
			newsize += FOB_BUF_GRAN;
		} while ( len > space );

						/* allocate (possibly) new buffer */
		newbuf = mtrf_realloc( fobp->buf, newsize );
		if ( newbuf == CNULL )
			return -1;

		fobp->bufsize = newsize;	/* update buffer size */

		if ( newbuf != fobp->buf ) {	/* if new buffer was allocated */
			fobp->buf = newbuf;	/* reset buffer pointers */
			fobp->pos = fobp->buf + nb;
		}
	}

	memcpy( fobp->pos, p, len );		/* copy the data */
	fobp->length += len;
	fobp->pos += len;
	return len;
}


int fob_bufget( fobp, p, len )
	FOB * fobp;
	char * p;
	int len;
{
	int nbread;
	int nbleft;
	int nbret;


	if ( fobp == FOBPNULL )
		return -1;
	if ( p == CNULL )
		return -1;
	if ( len <= 0 )
		return -1;

	nbread = fobp->pos - fobp->buf;		/* #bytes already read */
#ifdef debug
	nbleft = fobp->bufsize - nbread;	/* #bytes left to be read */
	/* JGF,  I think that this is an error, the length should really be used here */
#endif
	nbleft = fobp->length - nbread;		/* #bytes left to be read */
	nbret = MIN( len, nbleft );		/* #bytes to be returned */
	if ( nbret > 0 ) {
		memcpy( p, fobp->pos, nbret );	/* copy the data */
		fobp->pos += nbret;
	}
	return nbret;
}

buffer_swap_bytes(mem,blen)
char *mem;
int blen;
{
    char c, *p;
    for (p=mem; p<mem+blen; p+=2){
        c= *p; *p = *(p+1); *(p+1) = c;
    }
}

copy_buffer_swap_bytes(to,from,blen)
char *to, *from;
int blen;
{
    char *tp, *p;
    for (p=from, tp=to; p<from+blen; p+=2, tp+=2){
        *tp= *(p+1); *(tp+1) = *p;
    }

}

int fob_fread( p, size, nitems, fobp )
	char * p;
	int size;
	int nitems;
	FOB * fobp;
{
	int n;
	int bytes;


	if ( p == CNULL )
		return -1;
	if ( fobp == FOBPNULL )
		return -1;

	if ( fobp->fp != FPNULL ) {
		n = fread( p, size, nitems, fobp->fp );
		if ( n > 0 )
			fobp->length += n * size;
	} else {
		bytes = size * nitems;
		n = fob_bufget( fobp, p, bytes );
		if ( n > 0 )
			n /= size;
	}
        if (fobp->read_byte_swap){
/*            printf("Swapping bytes (%dx%d)  read %d\n",size,nitems,n); */
     
            if ((n*size > 0) && ((n*size % 2) != 0)) {
/*                fprintf(stderr,"Error: tried to byte swap an odd length buffer of %d bytes\n",n); */
                return -1;
	    }

            buffer_swap_bytes((char *)p,nitems*size);
	}
	return n;
}


int fob_fwrite( p, size, nitems, fobp )
	char * p;
	int size;
	int nitems;
	FOB * fobp;
{
	int n;
	int bytes;
	char * write_buf;

	if ( p == CNULL )
		return -1;
	if ( fobp == FOBPNULL )
		return -1;

        if (fobp->write_byte_swap){
	    /* alloc mem if it wasn't already done */
            if (fobp->buf_swap == CNULL){
		if ((fobp->buf_swap=(char *)mtrf_malloc(size*nitems)) == CNULL)
		    return -1;
		write_buf=fobp->buf_swap;
	    } else {
		write_buf=realloc(fobp->buf_swap,size*nitems);
                if (write_buf == CNULL)
		    return -1;

		fobp->buf_swap = write_buf;	
	    }
            if (((nitems*size) % 2) != 0) {
                fprintf(stderr,"Error: tried to byte swap an odd byte length buffer\n");
                return -1;
	    }
            copy_buffer_swap_bytes(write_buf,(char *)p,nitems*size);
	} else
	    write_buf = p;

	if ( fobp->fp != FPNULL ) {
		n = fwrite( write_buf, size, nitems, fobp->fp );
		if ( n > 0 )
			fobp->length += n * size;
	} else {
		bytes = size * nitems;
		n = fob_bufput( fobp, write_buf, bytes );
		if ( n > 0 )
			n /= size;
	}
	return n;
}

int fob_putc( c , fobp)
     char c;
     FOB *fobp;
{ 
     return (fob_fwrite(&c,1,1,fobp) == (-1) ? EOF : 1);
}

int fob_getc( fobp)
     FOB *fobp;
{ 
     unsigned char c;
     if ( fob_fread(&c,1,1,fobp) == EOF)
         return(EOF);
     return ( (int)c );

}


int fob_getw( fobp)
     FOB *fobp;
{ 
     int c;
     if ( fob_fread(&c,sizeof(int),1,fobp) == EOF)
         return(EOF);
     return ( c );

}



int fob_ferror( fobp )
	FOB * fobp;
{
	int n;


	if ( fobp == FOBPNULL )
		return -1;
	
	if ( fobp->fp != FPNULL )
		n = ferror( fobp->fp );
	else
		n = 0;
	return n;
}



int fob_feof( fobp )
FOB * fobp;
{
    int n;


    if ( fobp == FOBPNULL )
	return -1;
	
    if ( fobp->fp != FPNULL )
	n = feof( fobp->fp );
    else {
	if (fobp->pos >= (fobp->buf + fobp->length))
	    n = -1;
	else
	    n = 0;
    }
    return n;
}

