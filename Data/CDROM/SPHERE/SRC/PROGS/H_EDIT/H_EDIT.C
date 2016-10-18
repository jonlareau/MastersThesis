#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/h_edit.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* File: h_edit.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sp/sphere.h>

extern int ignore_failure, in_place, ov;
extern char *prog;

char opspec[] = "D:S:I:R:fo:uv";
char ops[]    = "SIR";


/* ARGSUSED */
int wav_edit(f1,f2,fct,f,fop)
char *f1, *f2, *f[], fop[];
int fct;
{
struct header_t *h;
char *errmsg;
register FILE *fp1, *fp2;
int i, failed;
long ohs, hs, dummy;


if (in_place) {
	ohs = sp_file_header_size(f1);
	if (ohs <= 0) {
		(void) fprintf(stderr,"%s: %s: Bad header\n",prog,f1);
		return -1;
	}
}

fp1 = fopen(f1,"r");
if (fp1 == FPNULL) {
	(void) fprintf(stderr,"%s: %s: Error opening for reading\n",prog,f1);
	return -1;
}

fp2 = fopen(f2,"w");
if (fp2 == FPNULL) {
	(void) fprintf(stderr,"%s: %s: Error opening %s for writing\n",
		prog,f1,f2);
	(void) fclose(fp1);
	return -1;
}

h = sp_open_header(fp1,1,&errmsg);
if (h == HDRNULL) {
	(void) fprintf(stderr,"%s: %s: Error reading header -- %s\n",
		prog,f1,errmsg);
	(void) fclose(fp1);
	(void) fclose(fp2);
	return -1;
}

failed = 0;
for (i=0; i < fct; i++) {
	char *eq, *p;
	long lbuf;
	double dbuf;
	int dummy, t, type;

	eq = p = strchr(f[i],'=');
	if (p == (char *) NULL) {
		(void) fprintf(stderr,"%s: %s: Error in edit specifier %s -- \"=\" required\n",
			prog,f1,f[i]);
		(void) sp_close_header(h);
		(void) fclose(fp1);
		(void) fclose(fp2);
		return -1;
	}

	*p++ = '\0';

	switch (fop[i]) {

	  case 'S':	type = T_STRING;
			break;

	  case 'I':	type = T_INTEGER;
			lbuf = atol(p);
			p = (char *) &lbuf;
			break;

	  case 'R':	type = T_REAL;
			dbuf = atof(p);
			p = (char *) &dbuf;
			break;

	  default:	(void) fprintf(stderr,
				"%s: %s: Error in edit specification -%s\n",
				prog,f1,fop[i]);
			*eq = '=';
			(void) sp_close_header(h);
			(void) fclose(fp1);
			(void) fclose(fp2);
			return -1;
	}


	if (sp_get_field(h,f[i],&t,&dummy) < 0) {
		if (sp_add_field(h,f[i],type,p) < 0) {
			(void) fprintf(stderr,"%s: %s: Error adding field %s\n",
				prog,f1,f[i]);
			failed++;
		}
		*eq = '=';
		continue;
	}

	if (type != t) {
		(void) fprintf(stderr,
			"%s: %s: Error changing field %s -- type mismatch\n",
			prog,f1,f[i]);
		failed++;
		*eq = '=';
		continue;
	}

	if (sp_change_field(h,f[i],type,p) < 0) {
		(void) fprintf(stderr,"%s: %s: Error changing field %s\n",
			prog,f1,f[i]);
		failed++;
	}

	*eq = '=';
}

if (failed && ! ignore_failure) {
	(void) unlink(f2);
	(void) fclose(fp1);
	(void) fclose(fp2);
	(void) sp_close_header(h);
	return -1;
}

if ((sp_write_header(fp2,h,&hs,&dummy) < 0) ||
    (!(ov = (in_place && (ohs == hs))) && (sp_fpcopy(fp1,fp2) < 0))) {
	(void) fprintf(stderr,"%s: %s: Error writing to %s\n",
		prog,f1,f2);
	(void) unlink(f2);
	(void) fclose(fp1);
	(void) fclose(fp2);
	(void) sp_close_header(h);
	return -1;
}

(void) fclose(fp1);
(void) fclose(fp2);

if (sp_close_header(h) < 0) {
	(void) fprintf(stderr,"%s: %s: Warning -- close_header failed\n",
		prog,f1);
	return -1;
}

if (ov)
	ov = hs;

return 0;
}
