/**********************************************************************/
/*                                                                    */
/*             FILENAME:  itoa.c                                       */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general routines used          */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#include <util/chars.h>
#include <stdio.h>

/**********************************************************************/
/* build a string zero padded string of max_len for the val           */
/**********************************************************************/
itoa(val,buff,max_len)
int val, max_len;
char *buff;
{
    int i, tmp, pos;

    buff[max_len] = NULL_CHAR;
    for (i=0, pos=max_len-1; i<max_len; i++, pos--, val/=10){
        buff[pos] = val - ((val/10)*10) + '0';
    }
}

