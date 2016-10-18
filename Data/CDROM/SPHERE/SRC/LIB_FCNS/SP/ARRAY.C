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
# include <util/fob.h>
# include "shorten.h"

void *pmalloc(size) ulong size; {
  void *ptr;

  ptr = malloc(size);

  if(ptr == NULL)
    perror_exit_sd("malloc(%d)", size);

  return(ptr);
}

long **long2d(n0, n1) ulong n0, n1; {
  long **array0;

  if((array0 = (long**)pmalloc(n0 * sizeof(long*) +n0*n1*sizeof(long)))!=NULL){
    long *array1 = (long*) (array0 + n0);
    int i;
    
    for(i = 0; i < n0; i++)
      array0[i] = array1 + i * n1;
  }
  return(array0);
}
