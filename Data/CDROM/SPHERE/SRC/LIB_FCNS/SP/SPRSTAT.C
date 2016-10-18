#include <stdio.h>
#include <sp/sphere.h>

int sp_print_return_status(fp)
FILE *fp;
{
    char *proc_name="sp_print_return_status";
    
    if (fp == FPNULL)
	return_err(proc_name,100,100,"Null File pointer");
    print_return_status(stderr);
    return(0);
}

int sp_get_return_status()
{
    return(return_status());
}
