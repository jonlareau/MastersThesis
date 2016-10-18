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

# include <stdio.h>
# include <util/fob.h> /* JGF */
# include <util/min.h> /* JGF */
# include "shorten.h"
# include "bitshift.h"

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

#define READSHORTHL(addr)\
  ((short) (*(((uchar*) (addr)) + 1) | (*((char*) (addr)) << 8)))

#define READSHORTLH(addr)\
  ((short) (*((uchar*) (addr)) | (*(((char*) (addr)) + 1) << 8)))

#define READUSHORTHL(addr)\
  ((unsigned short) (*(((uchar*) (addr)) + 1) | \
		     (*((uchar*) (addr)) << 8)))

#define READUSHORTLH(addr)\
  ((ushort) (*((uchar*) (addr)) | \
		     (*(((uchar*) (addr)) + 1) << 8)))

/* read a file for a given data type and convert to signed long ints */
int fread_type(data, ftype, nchan, nitem, stream) long **data; int ftype,
       nchan, nitem; FOB* stream; 
{
  static char *tmpbuf = NULL, *tmpfub = NULL;
  static ntmpbuf = 0;
  int hiloint = 1, hilo = !(*((char*) &hiloint));
  int i, nbyte = 0, nread, datasize = 0, chan;
  long *data0 = data[0];

  if (stream == (FOB *)0){
      ntmpbuf=0;
      return(0);
  }

  switch(ftype) {
  case TYPE_AU:
  case TYPE_S8:
  case TYPE_U8:
    datasize = sizeof(char);
    break;
  case TYPE_S16HL:
  case TYPE_U16HL:
  case TYPE_S16LH:
  case TYPE_U16LH:
    datasize = sizeof(short);
    break;
  default:
    update_exit_sd(1, "can't read file type: %d\n", ftype);
  }

  if(ntmpbuf < nchan * nitem * datasize) {
    ntmpbuf = nchan * nitem * datasize;
    if(tmpbuf != NULL) free(tmpbuf);
    if(tmpfub != NULL) free(tmpfub);
    tmpbuf = (char*) pmalloc((ulong) ntmpbuf);
    tmpfub = (char*) pmalloc((ulong) ntmpbuf);
  }

  switch(ftype) {
  case TYPE_AU:
  case TYPE_S8:
  case TYPE_U8:
    nbyte = fread((char*) tmpbuf, 1 , datasize * nchan * nitem, stream);
    break;
  case TYPE_S16HL:
  case TYPE_U16HL:
    if(hilo)
      nbyte = fread((char*) tmpbuf, 1 , datasize * nchan * nitem, stream);
    else {
      nbyte = fread((char*) tmpfub, 1 , datasize * nchan * nitem, stream);
      swab(tmpfub, tmpbuf, nbyte);
    }
    break;
  case TYPE_S16LH:
  case TYPE_U16LH:
    if(hilo) {
      nbyte = fread((char*) tmpfub, 1 , datasize * nchan * nitem, stream);
      swab(tmpfub, tmpbuf, nbyte);
    }
    else
      nbyte = fread((char*) tmpbuf, 1 , datasize * nchan * nitem, stream);
    break;
  default:
    update_exit_sd(1, "can't read file type: %d\n", ftype);
  }

  { int nextra = nbyte % (datasize * nchan);
    if(nextra != 0)
      usage_exit_sd(1, "alignment problem: %d extra bytes\n", nextra);
  }
  nread = nbyte / (datasize * nchan);

  switch(ftype) {
  case TYPE_AU: {
    uchar *tmpbufp = (uchar*) tmpbuf;
    if(nchan == 0)
      for(i = 0; i < nread; i++)
	data0[i] = *tmpbufp++;
    else
      for(i = 0; i < nread; i++)
	for(chan = 0; chan < nchan; chan++)
	  data[chan][i] = *tmpbufp++;
    break;
  }
  case TYPE_S8: {
    char *tmpbufp = (char*) tmpbuf;
    if(nchan == 0)
      for(i = 0; i < nread; i++)
	data0[i] = *tmpbufp++;
    else
      for(i = 0; i < nread; i++)
	for(chan = 0; chan < nchan; chan++)
	  data[chan][i] = *tmpbufp++;
    break;
  }
  case TYPE_U8: {
    uchar *tmpbufp = (uchar*) tmpbuf;
    if(nchan == 0)
      for(i = 0; i < nread; i++)
	data0[i] = *tmpbufp++;
    else
      for(i = 0; i < nread; i++)
	for(chan = 0; chan < nchan; chan++)
	  data[chan][i] = *tmpbufp++;
    break;
  }
  case TYPE_S16HL:
  case TYPE_S16LH: {
    short *tmpbufp = (short*) tmpbuf;
    if(nchan == 0)
      for(i = 0; i < nread; i++)
	data0[i] = *tmpbufp++;
    else
      for(i = 0; i < nread; i++)
	for(chan = 0; chan < nchan; chan++)
	  data[chan][i] = *tmpbufp++;
    break;
  }
  case TYPE_U16HL:
  case TYPE_U16LH: {
    ushort *tmpbufp = (ushort*) tmpbuf;
    if(nchan == 0)
      for(i = 0; i < nread; i++)
	data0[i] = *tmpbufp++;
    else
      for(i = 0; i < nread; i++)
	for(chan = 0; chan < nchan; chan++)
	  data[chan][i] = *tmpbufp++;
    break;
  }
  default:
    update_exit_sd(1, "can't read file type: %d\n", ftype);
  }
  return(nread);
}

#define WRITESHORTHL(addr, val) \
  *(( uchar*) (addr))      = ((val) >> 8) & 0xff,\
  *(((uchar*) (addr)) + 1) = (val) & 0xff

#define WRITESHORTLH(addr, val) \
  *(( uchar*) (addr))      = (val) & 0xff, \
  *(((uchar*) (addr)) + 1) = ((val) >> 8) & 0xff

/* convert from signed ints to a given type and write */
void fwrite_type(data, ftype, nchan, nitem, stream) long **data; int ftype,
       nchan, nitem; FOB* stream; {
  static char *tmpbuf = NULL, *tmpfub = NULL;
  static ntmpbuf = 0;
  int hiloint = 1, hilo = !(*((char*) &hiloint));
  int i, nwrite = 0, datasize = 0, chan;
  long *data0 = data[0];

  if (stream == (FOB *)0){
      ntmpbuf=0;
      return;
  }

  switch(ftype) {
  case TYPE_AU:
  case TYPE_S8:
  case TYPE_U8:
    datasize = sizeof(char);
    break;
  case TYPE_S16HL:
  case TYPE_U16HL:
  case TYPE_S16LH:
  case TYPE_U16LH:
    datasize = sizeof(short);
    break;
  default:
    update_exit_sd(1, "can't write file type: %d\n", ftype);
  }

  if(ntmpbuf < nchan * nitem * datasize) {
    ntmpbuf = nchan * nitem * datasize;
    if(tmpbuf != NULL) free(tmpbuf);
    if(tmpfub != NULL) free(tmpfub);
    tmpbuf = (char*) pmalloc((ulong) ntmpbuf);
    tmpfub = (char*) pmalloc((ulong) ntmpbuf);
  }

  switch(ftype) {
  case TYPE_AU: { /* leave the conversion to fix_bitshift() */
    uchar *tmpbufp = (uchar*) tmpbuf;
    if(nchan == 1)
      for(i = 0; i < nitem; i++)
	*tmpbufp++ = data0[i];
    else
      for(i = 0; i < nitem; i++)
	for(chan = 0; chan < nchan; chan++)
	  *tmpbufp++ = data[chan][i];
    break;
  }
  case TYPE_S8: {
    char *tmpbufp = (char*) tmpbuf;
    if(nchan == 1)
      for(i = 0; i < nitem; i++)
	*tmpbufp++ = data0[i];
    else
      for(i = 0; i < nitem; i++)
	for(chan = 0; chan < nchan; chan++)
	  *tmpbufp++ = data[chan][i];
    break;
  }
  case TYPE_U8: {
    uchar *tmpbufp = (uchar*) tmpbuf;
    if(nchan == 1)
      for(i = 0; i < nitem; i++)
	*tmpbufp++ = data0[i];
    else
      for(i = 0; i < nitem; i++)
	for(chan = 0; chan < nchan; chan++)
	  *tmpbufp++ = data[chan][i];
    break;
  }
  case TYPE_S16HL:
  case TYPE_S16LH: {
    short *tmpbufp = (short*) tmpbuf;
    if(nchan == 1)
      for(i = 0; i < nitem; i++)
	*tmpbufp++ = data0[i];
    else
      for(i = 0; i < nitem; i++)
	for(chan = 0; chan < nchan; chan++)
	  *tmpbufp++ = data[chan][i];
    break;
  }
  case TYPE_U16HL:
  case TYPE_U16LH: {
    ushort *tmpbufp = (ushort*) tmpbuf;
    if(nchan == 1)
      for(i = 0; i < nitem; i++)
	*tmpbufp++ = data0[i];
    else
      for(i = 0; i < nitem; i++)
	for(chan = 0; chan < nchan; chan++)
	  *tmpbufp++ = data[chan][i];
    break;
  }
  default:
    update_exit_sd(1, "can't write file type: %d\n", ftype);
  }

  switch(ftype) {
  case TYPE_AU:
  case TYPE_S8:
  case TYPE_U8:
    nwrite = fwrite((char*) tmpbuf, datasize * nchan, nitem, stream);
    break;
  case TYPE_S16HL:
  case TYPE_U16HL:
    if(hilo)
      nwrite = fwrite((char*) tmpbuf, datasize * nchan, nitem, stream);
    else {
      swab(tmpbuf, tmpfub, datasize * nchan * nitem);
      nwrite = fwrite((char*) tmpfub, datasize * nchan, nitem, stream);
    }
    break;
  case TYPE_S16LH:
  case TYPE_U16LH:
    if(hilo) {
      swab(tmpbuf, tmpfub, datasize * nchan * nitem);
      nwrite = fwrite((char*) tmpfub, datasize * nchan, nitem, stream);
    }
    else
      nwrite = fwrite((char*) tmpbuf, datasize * nchan, nitem, stream);
    break;
  default:
    update_exit_sd(1, "can't write file type: %d\n", ftype);
  }

  if(nwrite != nitem)
    update_exit_s(1, "failed to decompressed stream\n");
}

int find_bitshift(data, nitem, ftype) long *data; int nitem, ftype; {
  int i, bitshift;
  
  if(ftype == TYPE_AU) {
    bitshift = 15;
    for(i = 0; i < nitem && 
	(bitshift = MIN(bitshift, ulaw_maxshift[data[i]])) != 0; i++);
    for(i = 0; i < nitem; i++)
      data[i] = ulaw_inward[bitshift][data[i]];
  }
  else {
    int hash = 0;
	
    for(i = 0; i < nitem && ((hash |= data[i]) & 1) == 0; i++);
    for(bitshift = 0; (hash & 1) == 0 && bitshift < 32; bitshift++) hash >>= 1;
    if(bitshift != 0)
      for(i = 0; i < nitem; i++) data[i] >>= bitshift;
  }

  return(bitshift);
}

void fix_bitshift(buffer, nitem, bitshift, ftype) long *buffer; int nitem,
       bitshift, ftype; {
  int i;

  if(ftype == TYPE_AU)
    for(i = 0; i < nitem; i++)
      buffer[i] = ulaw_outward[bitshift][buffer[i] + 128];
  else
    if(bitshift != 0)
      for(i = 0; i < nitem; i++)
	buffer[i] <<= bitshift;
}
