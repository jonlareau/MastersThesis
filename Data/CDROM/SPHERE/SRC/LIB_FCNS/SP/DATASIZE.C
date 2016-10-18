/* LINTLIBRARY */

#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/datasize.c,v 1.2 1993/03/25 00:20:51 stan Exp stan $";
#endif

#include <stdio.h>
#include <sys/types.h>


u_long datasize()
{
	caddr_t sz;

	extern caddr_t sbrk();


	sz = sbrk(0);
	return (u_long) sz;
}
