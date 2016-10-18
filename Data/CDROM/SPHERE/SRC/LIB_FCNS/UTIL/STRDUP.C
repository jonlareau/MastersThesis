#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/spmalloc.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* LINTLIBRARY */

/* File: strdup.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

char *strdup(p)
char *p;
{
    return((char *)mtrf_strdup(p));
}
