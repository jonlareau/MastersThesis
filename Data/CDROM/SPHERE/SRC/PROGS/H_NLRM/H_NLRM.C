#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/h_nlrm.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* File: h_nlrm.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sp/sphere.h>

extern int ignore_failure, in_place, ov;
extern char *prog;

char opspec[] = "D:F:fo:uv";
char ops[]    = "F";


/* ARGSUSED */
int wav_edit(f1,f2,fct,f,fop)
char *f1, *f2, *f[], fop[];
int fct;
{
struct header_t *h;
char *errmsg;
register FILE *fp1, *fp2;
int i;
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

{
register int fc;
register struct field_t **fv;
char *p;

fv = h->fv;
fc = h->fc;
for (i=0; i < fc; i++) {
	if (fv[i]->type != T_STRING)
		continue;
	p = fv[i]->data + (fv[i]->datalen - 1);
	while ((fv[i]->datalen > 0) && (*p == '\n')) {
		*p-- = '\0';
		--fv[i]->datalen;
	}
}
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
