#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/h_modify.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* File: h_modify.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <sp/sphere.h>
#include <util/hsgetopt.h>

			/* for temporary files */
#define EXTENSION	".BAK"

int ignore_failure = 0, ov, in_place, verbose = 0;
char *prog;

main(argc,argv)
int argc;
char *argv[];
{
int exit_status = 0;

char *field[MAXFIELDS];
char op[MAXFIELDS];
int nfields = 0;

int i, c, uflag = 0;
char *outfile = CNULL, *dir = CNULL;
extern char opspec[], ops[];


prog = strrchr(argv[0],'/');
prog = (prog == CNULL) ? argv[0] : (prog + 1);

while ( (c = hs_getopt(argc,argv,opspec)) != -1 )

  switch (c) {
	case 'D':
		if (hs_optarg == CNULL)
			usage();
		dir = hs_optarg;
		break;

	case 'f':
		ignore_failure = 1;
		break;

	case 'o':
		if (hs_optarg == CNULL)
			usage();
		outfile = hs_optarg;
		break;

	case 'u':
		uflag = 1;
		break;

	case 'v':
		verbose = 1;
		break;

	default:
		if (hs_optarg == CNULL)
			usage();
		if (strchr(ops,c) == CNULL)
			usage();
		field[nfields] = hs_optarg;
		op[nfields] = (char) c;
		nfields++;
		break;
  }

if (nfields == 0) {
	(void) fprintf(stderr,"%s: Error -- no fields specified\n",prog);
	exit(1);
}

if (outfile != CNULL) {
	if (dir != CNULL)
		usage();
	if (hs_optind + 1 != argc)
		usage();
}

if (hs_optind >= argc) {
	(void) fprintf(stderr,"%s: Error -- no files specified\n",prog);
	exit(ERROR_EXIT_STATUS);
}

in_place = (outfile == CNULL) && (dir == CNULL);
if (uflag && in_place) {
	(void) fprintf(stderr,
		"%s: Error -- cannot unlink if editing in-place\n",
		prog);
	exit(ERROR_EXIT_STATUS);
}
if (verbose)
	(void) printf("Editing in-place\n");

for (i=hs_optind; i < argc; i++) {

	static char ofile[MAXPATHLEN];

	if (outfile != CNULL)
		(void) strcpy(ofile,outfile);
	else {
		if (dir == CNULL) {
			(void) strcpy(ofile,argv[i]);
			(void) strcat(ofile,EXTENSION);
		} else {
			char *base;

			(void) strcpy(ofile,dir);
			(void) strcat(ofile,"/");
			base = strrchr(argv[i],'/');
			base = (base == CNULL) ? argv[i] : (base + 1);
			(void) strcat(ofile,base);
		}
	}

	if (verbose)
		(void) printf("\nEditing %s\n",argv[i]);
	if (wav_edit(argv[i],ofile,nfields,field,op) < 0) {
		exit_status = ERROR_EXIT_STATUS;
		if (verbose)
			(void) printf("\tContinuing\n");
		continue;
	}

	if (in_place) {
		if (ov) {
			if (verbose)
				(void) printf("\tOverwriting header for %s\n",
					argv[i]);
			if (sp_overwrite_header(ofile,argv[i],ov) < 0) {
				(void) fprintf(stderr,
				   "%s: %s: Error -- cannot overwrite header\n",
				   prog,argv[i]);
				exit_status = ERROR_EXIT_STATUS;
			} else if (unlink(ofile) < 0) {
				perror(ofile);
				exit_status = ERROR_EXIT_STATUS;
			}
		} else {
			if (verbose)
				(void) printf("\tRenaming %s to %s\n",
					ofile,argv[i]);
			if (rename(ofile,argv[i]) < 0) {
				perror(argv[i]);
				exit_status = ERROR_EXIT_STATUS;
			}
		}
		continue;
	}

	if (verbose)
		(void) printf("\tCreated %s\n",ofile);
	if (uflag && (unlink(argv[i]) < 0)) {
		perror(argv[i]);
		exit_status = ERROR_EXIT_STATUS;
	}
}

exit(exit_status);
}

/*****************************************************************************/

usage()
{
int multi;
static char fn[]   = "fieldname";
static char fnv[]  = "fieldname=value";
static char use1[] = "Usage: %s [-uvf] [-D dir] -%s %s ... file ...\n";
static char use2[] = "   or: %s [-uvf] [-o outfile] -%s %s ... file\n";

extern char ops[];


multi = strlen(ops) > 1;
(void) fprintf(stderr, use1, prog, (!multi)?ops:"opchar", multi?fnv:fn );
(void) fprintf(stderr, use2, prog, (!multi)?ops:"opchar", multi?fnv:fn );
if (multi)
	(void) fprintf(stderr, "Opchar is any of %s\n", ops);
exit(ERROR_EXIT_STATUS);
}
