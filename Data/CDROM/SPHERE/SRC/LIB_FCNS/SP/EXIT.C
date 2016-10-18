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

# include <stdio.h>
# include <setjmp.h>
# include <util/fob.h>
# include <sp/shorten/shrt_sph.h>
# include "shorten.h"

extern int errno;
extern char *sys_errlist[];
extern char *argv0;

jmp_buf	exitenv;
char	*exitmessage;

void basic_exit(exitcode) int exitcode; {
  if(exitmessage == NULL)
    exit(exitcode < 0 ? 0 : exitcode);
  else
    longjmp(exitenv, exitcode);
}

void perror_exit_s(string) char *string; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: failed system call: ", argv0);
    fprintf(stderr, ": ");
    perror(string);
  }
  else {
    sprintf(exitmessage, string);
    strcat(exitmessage, ": ");
    strcat(exitmessage, sys_errlist[errno]);
    strcat(exitmessage, "\n");
  }

  basic_exit(errno);
}

void perror_exit_ss(string0, string1) char *string0, *string1; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: failed system call: ", argv0);
    fprintf(stderr, string0, string1);
    fprintf(stderr, ": ");
    perror(NULL);
  }
  else {
    sprintf(exitmessage, string0, string1);
    strcat(exitmessage, ": ");
    strcat(exitmessage, sys_errlist[errno]);
    strcat(exitmessage, "\n");
  }

  basic_exit(errno);
}

void perror_exit_sd(string, value) char *string; int value; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: failed system call: ", argv0);
    fprintf(stderr, string, value);
    fprintf(stderr, ": ");
    perror(NULL);
  }
  else {
    sprintf(exitmessage, string, value);
    strcat(exitmessage, ": ");
    strcat(exitmessage, sys_errlist[errno]);
    strcat(exitmessage, "\n");
  }

  basic_exit(errno);
}

void usage_exit_sss(exitcode, string0, string1, string2) int exitcode;
     char *string0, *string1, *string2; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: ", argv0);
    fprintf(stderr, string0, string1, string2);
  }
  else
    sprintf(exitmessage, string0, string1, string2);

  usage_exit(exitcode);
}

void usage_exit_ss(exitcode, string0, string1) int exitcode; char *string0;
     char *string1; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: ", argv0);
    fprintf(stderr, string0, string1);
  }
  else
    sprintf(exitmessage, string0, string1);

  usage_exit(exitcode);
}

void usage_exit_sd(exitcode, string, value) int exitcode; char *string;
     int value; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: ", argv0);
    fprintf(stderr, string, value);
  }
  else
    sprintf(exitmessage, string, value);

  usage_exit(exitcode);
}

void usage_exit_s(exitcode, string) int exitcode; char *string; {
  if(exitmessage == NULL)
    fprintf(stderr, "%s: %s", argv0, string);
  else
    strcpy(exitmessage, string);

  usage_exit(exitcode);
}

void usage_exit(exitcode) int exitcode; {
  if(exitmessage == NULL) {
    FILE *stream;

    if(exitcode > 0) stream = stderr;
    else stream = stdout;

    fprintf(stream, "%s: version %d.%s\n",argv0,FORMAT_VERSION,BUGFIX_RELEASE);
    fprintf(stream, "usage: %s [-hx] [-a #byte] [-b #sample] [-c #channel] [-d #discard]\n\t[-m #block] [-p #delay] [-q #bit] [-r #bit] [-t filetype]\n\t[input file] [output file]\n", argv0);
    fprintf(stream, "\t-a %d\tbytes to copy verbatim to align file\n", DEFAULT_NSKIP);
    fprintf(stream, "\t-b %d\tblock size\n", DEFAULT_BLOCK_SIZE); 
    fprintf(stream, "\t-c %d\tnumber of channels\n", DEFAULT_NCHAN); 
    fprintf(stream, "\t-d %d\tbytes to discard before compression or decompression\n", DEFAULT_NDISCARD); 
    fprintf(stream, "\t-h\thelp (this message)\n");
    fprintf(stream, "\t-m %d\tnumber of past block for mean estimation\n",
	    DEFAULT_NMEAN);
    fprintf(stream, "\t-p %d\tmaximum LPC predictor order (0 == fast polynomial predictor)\n", DEFAULT_MAXNLPC);
    fprintf(stream, "\t-q %d\tacceptable number quantisation error in bits\n", DEFAULT_QUANTERROR);
    fprintf(stream, "\t-r %d\tmaximum number of coded bits per sample\n", DEFAULT_MAXBITRATE);
    fprintf(stream, "\t-t s16\tfiletype {au,s8,u8,s16,u16,s16x,u16x,s16hl,u16hl,s16lh,u16lh}\n");
    fprintf(stream, "\t-v %d\tformat version number\n", FORMAT_VERSION);
    fprintf(stream, "\t-x\textract (all other options except -d are ignored)\n");
    exit(exitcode < 0 ? 0 : exitcode);
  }
  else
    longjmp(exitenv, exitcode);
}

void update_exit_sd(exitcode, string, value) int exitcode; char *string;
     int value; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: ", argv0);
    fprintf(stderr, string, value);
  }
  else
    sprintf(exitmessage, string, value);

  update_exit(exitcode);
}

void update_exit_s(exitcode, string) int exitcode; char *string; {
  if(exitmessage == NULL)
    fprintf(stderr, "%s: %s", argv0, string);
  else
    strcpy(exitmessage, string);

  update_exit(exitcode);
}

void update_exit(exitcode) int exitcode; {
  if(exitmessage == NULL) {
    fprintf(stderr, "%s: version %d.%s\n",argv0,FORMAT_VERSION,BUGFIX_RELEASE);
    fprintf(stderr, "%s: a fatal problem has occured\n", argv0);
    fprintf(stderr, "%s: please report this to ajr@eng.cam.ac.uk\n", argv0);
    exit(exitcode < 0 ? 0 : exitcode);
  }
  else
    longjmp(exitenv, exitcode);
}
