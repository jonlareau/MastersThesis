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

/* watch out, these are all 0 .. order inclusive arrays */

# define E_BITS_PER_COEF (2 + LPCQUANT)
# define VERY_SMALL	 (1e-36)

int wav2lpc(buf, nbuf, qlpc, nlpc, presn) long *buf; int nbuf, *qlpc, nlpc,
     *presn; {
  int   i, j, bestnbit, bestnlpc;
  float e = 0.0, ci, esize;
  float acf[MAX_LPC_ORDER];
  float ref[MAX_LPC_ORDER];
  float lpc[MAX_LPC_ORDER];
  float tmp[MAX_LPC_ORDER];
  float escale = 0.5 * M_LN2 * M_LN2 / nbuf;

  if(nlpc >= nbuf) nlpc = nbuf - 1;

  for(j = 0; j < nbuf; j++)
    e += (float) buf[j] * (float) buf[j];
  acf[0] = e;
  esize = 0.5 * log(escale * e + VERY_SMALL) / M_LN2;
  bestnbit = nbuf * esize;
  *presn = floor(esize + 0.5);
  bestnlpc = 0;

  for(i = 1; i <= nlpc; i++) {
    float sum = 0.0;

    for(j = i; j < nbuf; j++)
      sum += (float) buf[j] * (float) buf[j - i];
    acf[i] = sum;

    ci = 0.0;
    for(j = 1; j < i; j++) ci += lpc[j] * acf[i - j];
    ref[i] = ci = (acf[i] - ci) / e;
    lpc[i] = ci;
    for(j = 1; j < i; j++) tmp[j] = lpc[j] - ci * lpc[i - j];
    for(j = 1; j < i; j++) lpc[j] = tmp[j];

    e = (1 - ci * ci) * e;
    esize = 0.5 * log(escale * e + VERY_SMALL) / M_LN2;
    if(nbuf * esize + i * E_BITS_PER_COEF < bestnbit){
      int stable = 1;
      int tmpqlpc[MAX_LPC_ORDER];

      /* quantise lpc coefficients */
      for(j = 1; j <= i; j++)
	tmpqlpc[j] = lpc[j] * (1 << LPCQUANT);

#ifdef NEED_TO_GUARANTEE_LPC_STABILITY
      {
	int k;
	float tmpref[MAX_LPC_ORDER];

	/* check for stabilty */
	for(j = 1; j <= i; j++)
	  tmpref[j] = tmpqlpc[j] / (float) (1 << LPCQUANT);
      
	for(k = i; k > 0 && stable; k--) {
	  for(j = k - 1; j > 0; j--)
	    tmpref[j] = (tmpref[j] + tmpref[k] * tmpref[k - j]) /
	      (1.0 - tmpref[k] * tmpref[k]);
	  if(fabs(tmpref[k]) >= 1.0) stable = 0;
	}	
      }
#ifdef DEBUG_STABILITY

      if(!stable) {
	for(k = 1; k <= i; k++)
	  printf("%d\t%f\t%f\t%f\n", k, lpc[k], ref[k], tmpref[k]);
        printf("\n");
      }
#endif
#endif

      if(stable) {
	bestnbit = nbuf * esize + i * E_BITS_PER_COEF;
	bestnlpc = i;
	for(j = 0; j < bestnlpc; j++)
	  qlpc[j] = tmpqlpc[j + 1];
	*presn = floor(esize + 0.5);
      }
    }
  }
  if(*presn < 0) *presn = 0;

  return(bestnlpc);
}
