#include <stdio.h>
#include <sp/sphere.h>

char *get_natural_byte_order()
{
    short s;
    char *cp;
    s = 1;
    cp = (char *)&s;
/*    printf(" cp = %d\n",*cp);*/
    if (*cp == 0)
	return("10");
    return("01");
}

enum SP_sample_byte_fmt get_natural_sbf(sample_size)
int sample_size;
{
    short s;
    char *cp;

    if (sample_size == 1)
	return(SP_sbf_1);

    s = 1;
    cp = (char *)&s;
/*    printf(" cp = %d , cp+1 = %d\n",*cp,*(cp+1)); */

    if ((sample_size == 2) && (*cp == 0))
	return(SP_sbf_10);
    if ((sample_size == 2) && (*cp != 0))
	return(SP_sbf_01);
    return(SP_sbf_null);    
}
