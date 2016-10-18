/* File: sp_compute_short_checksum.c, Updated 10/31/91.

This function for computing a speech waveform checksum was adapted
from a function contributed by Mike Phillips at M.I.T.  It differs
from his function in that it computes an unsigned (vs. signed) integer
checksum.  It takes as arguments a pointer to an array of shorts
containing the waveform samples (wav) and the number of shorts/samples
in the array (len).  Please note that it works on 16-bit data ONLY and
it is not guaranteed to work on machines with less than 32-bit longs */

/*

Modified by Charles Hemphill at TI to provide a better functional
interface and increased portability.

*/


/* Modifications: Jon Fiscus  June 20, 1993
    This function did not take into account arrays which do not
    have the same byte format orientation as the host machine.  If
    the flag, swap_bytes is true, a byte swap is performed before the
    sample is added to the checksum.

*/

#include <stddef.h>		/* for size_t */
#include <stdio.h>
#include <sp/sphere.h>

SP_CHECKSUM sp_compute_short_checksum(/* short * */wav, /* size_t */ len, /* int */ swap_bytes)
short *wav;
size_t len;
int swap_bytes;
{ 
  unsigned short *p;
  unsigned short *end;
  unsigned long checksum;
  short sample;
  char *s_ptr, *p_ptr;

  checksum = 0;

  p = (unsigned short *) wav;
  end = p + len;

  if (! swap_bytes){
      while (p < end) {
	  checksum = (checksum + (*p++)) & 0xffff;
      }
  } else {
      s_ptr = (char *)&sample;
      p_ptr = (char *)p;
      while (p_ptr < (char *)end) {
	  *s_ptr = *(p_ptr+1);
	  *(s_ptr+1) = *p_ptr;
	  checksum = (checksum + sample) & 0xffff;
	  p_ptr += 2;
      }
  }
  return (SP_CHECKSUM) checksum;
}

SP_CHECKSUM sp_add_checksum(csum1, csum2)
SP_CHECKSUM csum1, csum2;
{
    long checksum;
    checksum = (csum1 + csum2) & 0xffff;
    return((SP_CHECKSUM) checksum);
}
