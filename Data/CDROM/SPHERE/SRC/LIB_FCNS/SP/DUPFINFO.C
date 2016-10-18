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

/* 
  set the atime and mtime of path1 to be the same as that of path0

  I only know how to write (and test) UNIX code - if anyone would
  care to donate a DOS version of this procedure I will incorporate
  it.
*/

#ifdef unix
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <util/fob.h>
#include "shorten.h"

struct utimbuf {
  time_t  actime;  /* set the access time */
  time_t  modtime; /* set the modification time */
} times;

extern int	utime		PROTO((const char*, const struct utimbuf*));
extern int	chown		PROTO((const char*, long, long));

int dupfileinfo(path0, path1) char *path0, *path1; {
  int errcode;
  struct stat buf;

  errcode = stat(path0, &buf);
  if(!errcode) {
    /* do what can be done, and igore errors */
    (void) chmod(path1, buf.st_mode);
    (void) chown(path1, buf.st_uid, -1);
    (void) chown(path1, -1, buf.st_gid);
    times.actime  = buf.st_atime;
    times.modtime = buf.st_mtime;
    (void) utime(path1, &times);
  }
  return(errcode);
}
#else
int dupfileinfo(path0, path1) char *path0, *path1; {
  return(0);
}
#endif

#ifdef PROGTEST
int main(int argc, char **argv) {
  return(dupfileinfo(argv[1], argv[2]));
}
#endif
