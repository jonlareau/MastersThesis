#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/h_encode.c,v 1.7 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* File: w_encode.c */

/************************************************/
/* This program encodes data in a NIST file     */
/* in place (i.e. the filename is not changed). */
/************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sp/sphere.h>
char usage[] = "Usage:  %s [-dmvf] -t [ wavpack | shorten | ulaw ] { filein | - } { fileout | - }\n        %s [-dmvi] -t [ wavpack | shorten | ulaw ] file1 file2 ...\n";
char * prog;


main( argc, argv )
int argc;
char *argv[];
{
    struct stat fileinfo;
    int status = 0;
    int type;
    int in_place = 0;
    int force_overwrite = 0;
    int dump_compression = 0;
    int maintain_sbf = 0;
    char format_conversion[256];
    int c;
    char *format_conversion_type=CNULL;

    format_conversion[0] = '\0';

    prog = strrchr( argv[0], '/' );
    prog = ( prog == CNULL ) ? argv[0] : ( prog + 1 );


    while (( c=hs_getopt( argc, argv, "dmvift:" )) != -1 )
	switch ( c ) {
	  case 'v':
	    sp_verbose++;
	    break;
	  case 'i':
	    in_place=1;
	    break;
	  case 'f':
	    force_overwrite=1;
	    break;
	  case 'd':
	    dump_compression=1;
	    break;
	  case 'm':
	    maintain_sbf=1;
	    break;
	  case 't':
	    if (strsame(hs_optarg,"wavpack"))
		format_conversion_type = "wavpack";
	    else  if (strsame(hs_optarg,"shorten"))
		format_conversion_type = "shorten";
	    else  if (strsame(hs_optarg,"ulaw"))
		format_conversion_type = "ulaw";
	    else{
		fprintf(stderr,"Error: unknown output type option '%s'\n",hs_optarg);
		fprintf(stderr,usage,prog,prog);
		exit(-1);
	    }
	    break;
	  default:
	    fprintf(stderr,"Error: unknown flag -%c\n",c);
	    fprintf(stderr,usage,prog,prog);
	    exit(-1);
	}

    if (format_conversion_type == CNULL){
	fprintf(stderr,"Output conversion type required\n");
	fprintf(stderr,usage,prog,prog);
	exit(-1);
    }

    if (strsame(format_conversion_type,"wavpack"))
	strcat(format_conversion,"SE-WAVPACK:");
    else  if (strsame(format_conversion_type,"shorten"))
	strcat(format_conversion,"SE-SHORTEN:");
    else  if (strsame(format_conversion_type,"ulaw"))
	strcat(format_conversion,"SE-ULAW:");
    else {
	fprintf(stderr,"Internal error\nContact Jon Fiscus");
	exit(-1);
    }
    if (maintain_sbf == 1)
	strcat(format_conversion,"SBF-ORIG");
    else
	strcat(format_conversion,"SBF-N");

    if (in_place == 0){
	char *filein, *fileout;

	if (argc - hs_optind != 2){
	    fprintf(stderr,"Error: Requires 2 filename arguements\n");
	    fprintf(stderr,usage,prog,prog);
	    exit(-1);
	}

	filein=argv[hs_optind];
	fileout=argv[hs_optind+1];
	if (force_overwrite == 0){
	    if (stat(fileout,&fileinfo) == 0) {
		fprintf(stderr,"Unable to overwrite output file %s.  Use -f to override\n",fileout);
		fprintf(stderr,usage,prog,prog);
		exit(-1);
	    }
	}

	hs_resetopt();
	if (convert_file(filein,fileout,format_conversion) != 0){
	    exit(-1);
	}
    } else {
	int op, baseop;
	if (argc - hs_optind < 1){
	    fprintf(stderr,"Error: Requires at least one filename\n");
	    fprintf(stderr,usage,prog,prog);
	    exit(-1);
	}
	baseop = hs_optind;
	hs_resetopt();
	for (op=baseop; op<argc; op++)
	    if (do_update(argv[op],format_conversion) != 0)
		exit(-1);
    }
}


convert_file(filein,fileout,format_conversion)
char *filein, *fileout, *format_conversion;
{
    SP_FILE *sp_in, *sp_out;
    
    if ((sp_in=sp_open(filein,"r")) == SPNULL){
	fprintf(stderr,usage,prog,prog);
	sp_print_return_status(stderr);
	fprintf(stderr,"Unable to open file '%s' to update\n",(strsame(filein,"-") ? "stdin" : filein ));
	return(100);
    }
    if ((sp_out=sp_open(fileout,"w")) == SPNULL){
	fprintf(stderr,usage,prog,prog);
	sp_print_return_status(stderr);
	sp_close(sp_in);
	fprintf(stderr,"Unable to open file '%s' to update\n",(strsame(fileout,"-") ? "stdout" : fileout ));
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }

    if (sp_dup(sp_in,sp_out) != 0){
	fprintf(stderr,"Unable to duplicate the input file\n");
	sp_print_return_status(stdout);
	sp_close(sp_in); sp_close(sp_out);
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }

    if (sp_set_data_mode(sp_out,format_conversion) != 0){
	sp_print_return_status(stderr);
	fprintf(stderr,"Unable to set data mode to '%s'\n",format_conversion);
	sp_close(sp_in); sp_close(sp_out);
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }
    
    { char *buff;
      int tot=0;
      int ns, nc, nspb;
      int samples_read, samples_written;
      ns = sp_in->read_spifr->status->user_sample_count;
      nc = sp_in->read_spifr->status->user_channel_count;
      nspb = sp_in->read_spifr->status->user_sample_n_bytes;
      if ((buff=mtrf_malloc(nc * nspb * 4096)) == CNULL) {
	  sp_close(sp_in); sp_close(sp_out);
	  if (! strsame(fileout,"-")) unlink(fileout);
	  return(100);
      }
      do {
	  samples_read = sp_read_data(buff,nspb,4096,sp_in);
	  if (samples_read > 0) {
	      samples_written = sp_write_data(buff,nspb,samples_read,sp_out);			    
	      if (samples_written != samples_read){
		  sp_close(sp_in); sp_close(sp_out);
		  if (! strsame(fileout,"-")) unlink(fileout);
		  mtrf_free(buff);
		  return(100);
	      }
	  } else { 
	      if (sp_error(sp_in) >= 100) { /* a checksum error occured, close the sp and */
		  sp_print_return_status(stdout);
		  sp_close(sp_in); sp_close(sp_out);
		  if (! strsame(fileout,"-")) unlink(fileout);
		  mtrf_free(buff);
		  return(100);
	      }
	  }
      } while (samples_read > 0);
      mtrf_free(buff);
      sp_close(sp_in);
      sp_close(sp_out);
  }
}

int do_update(filein,format_conversion)
char *filein ,*format_conversion;
{
    SP_FILE *sp;

    if ((sp=sp_open(filein,"u")) == SPNULL){
	fprintf(stderr,usage,prog,prog);
	sp_print_return_status(stderr);
	fprintf(stderr,"Unable to open file '%s' to update\n",filein);
	return(100);
    }
    if (sp_set_data_mode(sp,format_conversion) != 0){
	fprintf(stderr,usage,prog,prog);
	sp_print_return_status(stderr);
	fprintf(stderr,"Unable to set data mode to '%s'\n",format_conversion);
	sp_close(sp);
	return(100);
    }
    if (sp_close(sp) != 0){
	fprintf(stderr,usage,prog,prog);
	sp_print_return_status(stderr);
	fprintf(stderr,"File Close failed\n");
	return(100);
    }
    return(0);
}
