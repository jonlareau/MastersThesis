/* File: mtrf.c */
/* converted from spmalloc to be a general memory tracing function */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <util/mtrf.h>

static int mtrf_dealloc = 1;
static int mtrf_verbose = 0;

char *mtrf_malloc(bytes)
u_int bytes;
{
    char *cp;
    if (bytes < 0)
	return((char *)0);
    if (bytes == 0)
	bytes = 1;
    cp=malloc(bytes);
    if (mtrf_verbose)
	printf("Malloc %x %d\n",cp,bytes), fflush(stdout);
    return cp;
}

char *mtrf_realloc(ptr,bytes)
char *ptr;
u_int bytes;
{
    char *cp;

    cp = realloc(ptr, bytes);
    if (cp != ptr)
	if (mtrf_verbose)
	    printf("REALLOC\nFree %x\nMalloc %x %d\n",ptr,cp,bytes), fflush(stdout);
    return cp;
}

int mtrf_free(p)
char *p;
{
    int rtn;

    rtn = 1;
    if (mtrf_dealloc)
	free(p);
    if (mtrf_verbose)
	printf("Free %x\n",p), fflush(stdout);
    return(rtn);
}

char *mtrf_strdup(p)
char *p;
{
    char *cp;
    int len;
    len = ((strlen(p) == 0) ? 1 : strlen(p)) + 1;

    cp=mtrf_malloc(len);
    strcpy(cp,p);
    if (mtrf_dealloc == 0)
	printf("Malloc %x %d\n",cp,len), fflush(stdout);
    return(cp);
}

void mtrf_set_dealloc(n)
int n;
{
    mtrf_dealloc = n;
}

void mtrf_set_verbose(n)
int n;
{
    mtrf_verbose = n;
}

int mtrf_get_dealloc()
{
    return mtrf_dealloc;
}

int mtrf_get_verbose()
{
    return mtrf_verbose;
}

