#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/verbose.c,v 1.3 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* LINTLIBRARY */

int sp_verbose = 0;

int sp_set_verbose(n)
int n;
{
    sp_verbose = n;
}

