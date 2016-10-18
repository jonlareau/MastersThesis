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
# include <util/fob.h>
# include "shorten.h"

int find_longtap(buffer, blocksize, minlongtap, maxlongtap) int *buffer,
       blocksize, minlongtap, maxlongtap; {
  int i, sum, bestvar, longtap, besttap = 0;

  sum = 0;
  for(i = 0; i < blocksize; i++) sum += abs(buffer[i]);
  bestvar = 0.6 * sum;

  for(longtap = minlongtap; longtap < maxlongtap; longtap++) {
    sum = 0;
    for(i = 0; sum < bestvar && i < blocksize; i++)
      sum += abs(buffer[i] - buffer[i - longtap]);

    if(sum < bestvar) {
      bestvar = sum;
      besttap = longtap;
    }
  }

  return(besttap);
}

void remove_longtap(buffer, blocksize, longtap) int *buffer, blocksize,
       longtap; {
  int i;
  
  for(i = blocksize - 1; i >= 0; i--)
    buffer[i] -= buffer[i - longtap];
}

void replace_longtap(buffer, blocksize, longtap) int *buffer, blocksize,
       longtap; {
  int i;
  
  for(i = 0; i < blocksize; i++)
    buffer[i] += buffer[i - longtap];
}
