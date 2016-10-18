/******************************************************************************

Copyright (C) 1992,1993 Tony Robinson

Permission is granted to use this software for non-commercial purposes.
Explicit permission must be obtained from the author to use this software
for commercial purposes.

This software carries no warranty, expressed or implied.  The user assumes
all risks, known or unknown, direct or indirect, which involve this software
in any way.

Dr Tony Robinson
Cambridge University Engineering Department
Trumpington Street, Cambridge, CB2 1PZ, UK.
ajr@eng.cam.ac.uk     voice: +44-223-332815

******************************************************************************/

# include <math.h>
# include <stdio.h>
# include <util/fob.h> /* JGF */
# include "shorten.h"


#ifdef fread
#       undef fread
#endif
#define fread(a,b,c,d)          fob_fread((a),(b),(c),(d))
 
#ifdef fwrite
#       undef fwrite
#endif
#define fwrite(a,b,c,d)         fob_fwrite((a),(b),(c),(d))
 
#ifdef putc
#       undef putc
#endif
#define putc(a,b)               fob_putc((a),(b))

#ifdef getc
#       undef getc
#endif
#define getc(a)                 fob_getc((a))


extern char *argv0;

# define MASKTABSIZE 33
ulong masktab[MASKTABSIZE];

void mkmasktab() {
  int i;
  ulong val = 0;

  masktab[0] = val;
  for(i = 1; i < 33; i++) {
    val <<= 1;
    val |= 1;
    masktab[i] = val;
  }
}

uchar *putbuf;
uchar *putbufp;

void var_put_init(stream) FOB *stream; {
  mkmasktab();

  putbuf = (uchar*) pmalloc((ulong) BUFSIZ);
  putbufp = putbuf;
}

uchar *getbuf;

void var_get_init(stream) FOB *stream; {
  mkmasktab();

  getbuf = (uchar*) pmalloc((ulong) BUFSIZ);
}

void word_put(buffer, stream) ulong buffer; FOB *stream; {
  static int nbytebuf = 0;

  if (stream == (FOB *)0){
      nbytebuf = 0;
      return;
  }

  *putbufp++ = buffer >> 24;
  *putbufp++ = buffer >> 16;
  *putbufp++ = buffer >>  8;
  *putbufp++ = buffer;

  nbytebuf += 4;

  if(nbytebuf == BUFSIZ) {
    if(fwrite((char*) putbuf, 1, BUFSIZ, stream) != BUFSIZ)
      update_exit_s(1, "failed to write compressed stream\n");
    
    putbufp = putbuf;
    nbytebuf = 0;
  }
}

void uvar_put(val, nbin, stream) ulong val; int nbin; FOB *stream; {
  static ulong buffer = 0;
  static int nbuffer = 0;
  ulong lobin = (1L << nbin) | (val & masktab[nbin]);	/* SAL */
  ulong	nsd = val >> nbin;
  int  i, nlobin = nbin + 1;

  if (stream == (FOB *)0){
      buffer = 0;
      nbuffer = 0;
      return;
  }

  if(nbuffer + nsd >= 32) {
    for(i = 0; i < (nbuffer + nsd) >> 5; i++) {
      word_put(buffer, stream);
      buffer = 0;
    }
    nbuffer = (nbuffer + nsd) % 32;
  }
  else
    nbuffer += nsd;
  
  while(nlobin != 0) {
    if(nbuffer + nlobin >= 32) {
      buffer |= (lobin >> (nbuffer + nlobin - 32));
      word_put(buffer, stream);
      buffer = 0;
      nlobin -= 32 - nbuffer;
      nbuffer = 0;
    }
    else {
      nbuffer += nlobin;
      buffer |= (lobin << (32 - nbuffer));
      nlobin = 0;
    }
  }
}

void ulong_put(val, stream) ulong val; FOB *stream; {
  int i, nbit;
  
  for(i = 31; i >= 0 && (val & (1L << i)) == 0; i--);	/* SAL */
  nbit = i + 1;
  
  uvar_put((ulong) nbit, ULONGSIZE, stream);
  uvar_put(val & masktab[nbit], nbit, stream);
}

ulong word_get(stream) FOB *stream; {
  static int nbytebuf = 0;
  static uchar *getbufp;
  ulong buffer;

  if (stream == (FOB *)0){
      nbytebuf = 0;
      getbufp = (unsigned char *)0;
      return(0);
  }

  if(nbytebuf == 0) {
    nbytebuf = fread((char*) getbuf, 1, BUFSIZ, stream);
    if(nbytebuf == 0)
      update_exit_s(1, "premature EOF on compressed stream\n");
    getbufp = getbuf;
  }
  buffer = ((long) getbufp[0]) << 24 | ((long) getbufp[1]) << 16 |
           ((long) getbufp[2]) <<  8 | ((long) getbufp[3]);	/* SAL */

  getbufp += 4;
  nbytebuf -= 4;;

  return(buffer);
}

long uvar_get(nbin, stream) int nbin; FOB *stream; {
  static ulong buffer = 0;
  static int nbuffer = 0;
  long result;

  if (stream == (FOB *)0){
      buffer = 0;
      nbuffer = 0;
      return(0);
  }

  if(nbuffer == 0) {
    buffer = word_get(stream);
    nbuffer = 32;
  }

  for(result = 0; !(buffer & (1L << --nbuffer)); result++) {	/* SAL */
    if(nbuffer == 0) {
      buffer = word_get(stream);
      nbuffer = 32;
    }
  }

  while(nbin != 0) {
    if(nbuffer >= nbin) {
      result = (result << nbin) | ((buffer >> (nbuffer-nbin)) & masktab[nbin]);
      nbuffer -= nbin;
      nbin = 0;
    } 
    else {
      result = (result << nbuffer) | (buffer & masktab[nbuffer]);
      buffer = word_get(stream);
      nbin -= nbuffer;
      nbuffer = 32;
    }
  }

  return(result);
}

ulong ulong_get(stream) FOB *stream; {
  int nbit = uvar_get(ULONGSIZE, stream);
  return(uvar_get(nbit, stream));
}

void var_put(val, nbin, stream) long val; int nbin; FOB *stream; {
  if(val < 0) uvar_put((ulong) ((~val) << 1) | 1L, nbin + 1, stream);
  else uvar_put((ulong) ((val) << 1), nbin + 1, stream);
}

void var_put_quit(stream) FOB *stream; {
  unsigned long l;
  int i;

  /* flush to a word boundary */
  uvar_put((ulong) 0, 31, stream);

  /* and write out the remaining chunk in the buffer */
  if(fwrite((char*) putbuf, 1, putbufp - putbuf, stream) != 
     putbufp - putbuf)
    update_exit_s(1, "failed to write compressed stream\n");

  free((char*) putbuf);
  word_put(l, (FOB *)0);
  uvar_put(l, i, (FOB *)0);
}

long var_get(nbin, stream) int nbin; FOB *stream; {
  ulong uvar = uvar_get(nbin + 1, stream);

  if(uvar & 1) return((long) ~(uvar >> 1));
  else return((long) (uvar >> 1));
}

void var_get_quit(stream) FOB *stream; {
  int i;

  free((char*) getbuf);
  word_get((FOB *)0);
  uvar_get(i,(FOB *)0);
}


int sizeof_uvar(val, nbin) ulong val; int nbin; {
  return((val >> nbin) + nbin);
}

int sizeof_var(val, nbin) long val; int nbin; {
  return((abs(val) >> nbin) + nbin + 1);
}


#ifdef DEBUG

#define PANIC_putc(val, stream)\
{ char rval;\
  if((rval = putc((val), (stream))) != (char) (val)) {\
    fprintf(stderr, "%s: PANIC: putc(%d, 0x%x) == %d\n", argv0, (val), stream,\
	    rval);\
    fprintf(stderr, "%s: PANIC: write failure\n", argv0);\
    exit(1);\
    }\
}

int     PANIC_getc_val;

#define PANIC_getc(stream)\
(\
  ((PANIC_getc_val = getc(stream)) == EOF) ? \
    fprintf(stderr, "%s: PANIC: getc(0x%x) == EOF\n", argv0, stream),\
    fprintf(stderr, "%s: PANIC: unexpected EOF\n", argv0),\
    exit(1), PANIC_getc_val :\
  PANIC_getc_val\
)


void bit_put(val, stream) int val; FOB* stream; {
  static uchar buffer = 0;
  static int nbuffer = 0;

  if (stream == (FOB *)0){
      buffer=0;
      nbuffer=0;
      return;
  }

  if(val != 0) buffer |= (1 << (7 - nbuffer));
  if(nbuffer == 7) {
    PANIC_putc(buffer, stream);
    buffer = 0;
    nbuffer = 0;
  }
  else 
    nbuffer++;
}

void uvar_put(val, nbin, stream) ulong val; int nbin; FOB *stream; {
  int i;

  for(i = 0; i < (val >> nbin); i++) bit_put(0, stream);
  bit_put(1, stream);
  
  for(i = nbin - 1; i >= 0; i--) {
    if((val & (1L << i)) == 0) bit_put(0, stream);	/* SAL */
    else bit_put(1, stream);
  }
}

int bit_get(stream) FOB* stream; {
  static uchar buffer = 0;
  static int nbuffer = 0;

  if (stream == (FOB *)0){
      buffer=0;
      nbuffer=0;
      return;
  }

  if(nbuffer == 0) {
    buffer = PANIC_getc(stream);
    nbuffer = 8;
  }
  nbuffer--;

  return(((buffer & (1 << nbuffer)) == 0) ? 0 : 1);
}

long uvar_get(nbin, stream) int nbin; FOB *stream; {
  long result;
  int  i;

  for(result = 0; bit_get(stream) == 0; result++);
  
  for(i = 0; i < nbin; i++)
    result = (result << 1) | bit_get(stream);

  return(result);
}

#endif
