#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/h_add.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* File: h_add.c */

/******************************************/
/* This program demonstrates how a file   */
/* with a NIST header can be created      */
/******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sp/sphere.h>

main(argc,argv)
int argc;
char *argv[];
{
char *prog;
static char usage[] = "Usage:  %s {-|infile} {-|outfile}\n";
long databytes, hbytes;
struct header_t *h;
register FILE *fp1, *fp2, *outputfp = stdout;
int n;


prog = strrchr(argv[0],'/');
prog = (prog == CNULL) ? argv[0] : (prog + 1);

if (argc != 3) {
	(void) fprintf(stderr,usage,prog);
	exit(ERROR_EXIT_STATUS);
}

if (strcmp(argv[1],"-") == 0)
	fp1 = stdin;
else {
	fp1 = fopen(argv[1],"r");
	if (fp1 == FPNULL) {
		(void) fprintf(stderr,"%s: Could not open %s\n",prog,argv[1]);
		exit(ERROR_EXIT_STATUS);
	}
}

if (strcmp(argv[2],"-") == 0) {
	fp2 = stdout;
	outputfp = stderr;
} else {
	fp2 = fopen(argv[2],"w");
	if (fp2 == FPNULL) {
		(void) fprintf(stderr,"%s: Could not open %s\n",prog,argv[2]);
		exit(ERROR_EXIT_STATUS);
	}
}

h = sp_create_header();
if (h == HDRNULL) {
	(void) fprintf(stderr,"%s: Could not create header %s\n",prog);
	exit(ERROR_EXIT_STATUS);
}

/* Here is how one could add fields to the raw file */
/*
{
double r = 205.111;
long l = 27L;

n = sp_add_field(h,"field1",T_INTEGER,(char *) &l);
if (n>=0) n = sp_add_field(h,"field2",T_REAL,(char *) &r);
if (n>=0) n = sp_add_field(h,"field3",T_STRING,"foobar");
if (n < 0) {
	(void) fprintf(stderr,"%s: error adding fields to header\n");
	exit(ERROR_EXIT_STATUS);
}
}
*/

n = sp_write_header(fp2,h,&hbytes,&databytes);
if (n < 0) {
	(void) fprintf(stderr,"%s: sp_write_header failed\n",prog);
	exit(ERROR_EXIT_STATUS);
}

(void) fprintf(outputfp,"%s: %s --> %s (header=%ld bytes, data=%ld bytes)\n",
		prog,argv[1],argv[2],hbytes,databytes);

if (sp_fpcopy(fp1,fp2) < 0) {
	(void) fprintf(stderr,"%s: error copying samples\n",prog);
	exit(ERROR_EXIT_STATUS);
}

exit(0);
}
