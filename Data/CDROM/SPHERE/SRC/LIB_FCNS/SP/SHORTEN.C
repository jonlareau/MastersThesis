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

/* SAL ---
   Inserted (unsigned long) before first argument in all calls to
   uvar_put and ulong_put.
*/

# include <math.h>
# include <stdio.h>
# include <setjmp.h>
# include <util/fob.h>
# include <util/min.h>
# include <util/hsgetopt.h>
# include "shorten.h"


#ifdef fread
#       undef fread
#endif
#define fread(a,b,c,d)          fob_fread((a),(b),(c),(d))
 
#ifdef fwrite
#       undef fwrite
#endif
#define fwrite(a,b,c,d)         fob_fwrite((a),(b),(c),(d))
 
#ifdef putc
#       undef putc
#endif
#define putc(a,b)               fob_putc((a),(b))

#ifdef getc
#       undef getc
#endif
#define getc(a)                 fob_getc((a))


#ifdef unix
char *readmode = "r";
char *writemode = "w";
#else
char *readmode = "rb";
char *writemode = "wb";
#endif

char	*argv0 = "shorten";

# define ALPHA0 (3.0 / 2.0)
# define ALPHA1 M_LN2

/* SAL --- put (unsigned long) into macro */
# define UINT_PUT(val, nbit, file) \
  if(version == 0) uvar_put((unsigned long) val, nbit, file); \
  else ulong_put((unsigned long) val, file)

# define UINT_GET(nbit, file) \
  ((version == 0) ? uvar_get(nbit, file) : ulong_get(file))

/* SAL --- put (unsigned long) into macro */
# define VAR_PUT(val, nbit, file) \
  if(version == 0) var_put((unsigned long) val, nbit - 1, file); \
  else var_put((unsigned long) val, nbit, file)

int init_offset(offset, nchan, nblock, ftype) long **offset; int nchan,
       nblock, ftype; {
  long mean = 0;
  int  chan, i;

  /* initialise offset */
  switch(ftype) {
  case TYPE_AU:
  case TYPE_S8:
  case TYPE_S16HL:
  case TYPE_S16LH:
    mean = 0;
    break;
  case TYPE_U8:
    mean = 0x80;
    break;
  case TYPE_U16HL:
  case TYPE_U16LH:
    mean = 0x8000;
    break;
  default:
    update_exit_sd(1, "unknown file type: %d\n", ftype);
  }

  for(chan = 0; chan < nchan; chan++)
    for(i = 0; i < nblock; i++)
      offset[chan][i] = mean;

  return(mean);
}

int shorten(stdi, stdo, argc, argv) FOB *stdi, *stdo; int argc; char **argv; {
  static char* copyright = "Copyright (C) 1992,1993 Tony Robinson\n";
  long 	**buffer, *buffer1, **offset;
  long	default_offset;
  int   version = FORMAT_VERSION, extract = 0, bitshift = 0;
  int   hiloint = 1, hilo = !(*((char*) &hiloint));
  int   ftype = hilo ? TYPE_S16HL : TYPE_S16LH;
  char  *magic = MAGIC, *filenamei = NULL, *filenameo = NULL;
  char	*tmpfilename = NULL, *minusstr = "-", *filesuffix = ".shn";
  FOB  *filei, *fileo;
  int	 blocksize = DEFAULT_BLOCK_SIZE, nchan = DEFAULT_NCHAN;
  int	 i, chan, nwrap, nskip = DEFAULT_NSKIP, ndiscard = DEFAULT_NDISCARD;
  int	*qlpc = NULL, maxnlpc = DEFAULT_MAXNLPC, nmean = DEFAULT_NMEAN;
  int	maxresn = DEFAULT_MAXBITRATE, quanterror = DEFAULT_QUANTERROR;
  int	nfilename;
  extern char *optarg;
  extern int   optind;

  /* this block just processes the command line arguments */
  { int c;

    hs_resetopt();
/*  while((c = hs_getopt(argc, argv, "a:b:c:d:hm:p:r:t:xv:")) != -1) */
    while((c = hs_getopt(argc, argv, "a:b:c:d:hm:p:t:xv:")) != -1)
      switch(c) {
      case 'a':
	if((nskip = atoi(hs_optarg)) < 0) usage_exit(1);
	break;
      case 'b':
	if((blocksize = atoi(hs_optarg)) <= 0) usage_exit(1);
	break;
      case 'c':
	if((nchan = atoi(hs_optarg)) <= 0) usage_exit(1);
	break;
      case 'd':
	if((ndiscard = atoi(hs_optarg)) < 0) usage_exit(1);
	break;
      case 'h':
	usage_exit(-1);
	break;
      case 'm':
	if((nmean = atoi(hs_optarg)) < 0) usage_exit(1);
	break;
      case 'p':
	maxnlpc = atoi(hs_optarg);
	if(maxnlpc < 0 || maxnlpc > MAX_LPC_ORDER) usage_exit(1);
	break;
      case 'q':
	if((quanterror = atoi(hs_optarg)) < 0) usage_exit(1);
	break;
      case 'r':
	if((maxresn = atoi(hs_optarg)) < 0) usage_exit(1);
	break;
      case 't':
	if     (!strcmp(hs_optarg, "au"))	ftype = TYPE_AU;
	else if(!strcmp(hs_optarg, "s8"))	ftype = TYPE_S8;
	else if(!strcmp(hs_optarg, "u8"))	ftype = TYPE_U8;
	else if(!strcmp(hs_optarg, "s16")) ftype = hilo ? TYPE_S16HL : TYPE_S16LH;
	else if(!strcmp(hs_optarg, "u16")) ftype = hilo ? TYPE_U16HL : TYPE_U16LH;
	else if(!strcmp(hs_optarg, "s16x"))ftype = hilo ? TYPE_S16LH : TYPE_S16HL;
	else if(!strcmp(hs_optarg, "u16x"))ftype = hilo ? TYPE_U16LH : TYPE_U16HL;
	else if(!strcmp(hs_optarg, "s16hl"))ftype = TYPE_S16HL;
	else if(!strcmp(hs_optarg, "u16hl"))ftype = TYPE_U16HL;
	else if(!strcmp(hs_optarg, "s16lh"))ftype = TYPE_S16LH;
	else if(!strcmp(hs_optarg, "u16lh"))ftype = TYPE_U16LH;
	else usage_exit(1);
	break;
      case 'v':
	version = atoi(hs_optarg);
	if(version < 0 || version > FORMAT_VERSION + 1) usage_exit(1);
	break;
      case 'x':
	extract = 1;
	break;
      case '?':
	usage_exit(1);
	break;
      }
  }

  if(maxnlpc >= blocksize)
    usage_exit_s(1, "the predictor order must be less than the block size\n");

  /* this chunk just sets up the input and output files */
#ifdef  STANDALONE
  nfilename = argc - hs_optind;
  switch(nfilename) {
  case 0:
    filenamei = minusstr;
    filenameo = minusstr;
    break;
  case 1: {
    int oldfilelen, suffixlen, maxlen;

    filenamei  = argv[argc - 1];
    oldfilelen = strlen(filenamei);
    suffixlen  = strlen(filesuffix);
    maxlen     = oldfilelen + suffixlen;
    tmpfilename = pmalloc((ulong) (maxlen + 1));
    strcpy(tmpfilename, filenamei);
    
    if(extract) {
      int newfilelen = oldfilelen - suffixlen;
      if(strcmp(filenamei + newfilelen, filesuffix))
	usage_exit_sss(1,"file name does not end in %s: %s\n", filesuffix,
		       filenamei);
      tmpfilename[newfilelen] = '\0';
    }
    else
      strcat(tmpfilename, filesuffix);

    filenameo = tmpfilename;
    break;
  }
  case 2:
    filenamei = argv[argc - 2];
    filenameo = argv[argc - 1];
    break;
  default:
    usage_exit(1);
  }

  if(strcmp(filenamei, minusstr)) {
    if((filei = fopen(filenamei, readmode)) == NULL)
      usage_exit_ss(1, "can't open: %s\n", filenamei);
  }	
  else filei = stdi;

  if(strcmp(filenameo, minusstr)) {
    if((fileo = fopen(filenameo, writemode)) == NULL)
      usage_exit_ss(1, "can't open: %s\n", filenameo);
  }
  else fileo = stdo;
#else

  fileo = stdo;
  filei = stdi;
#endif

  /* discard header on input file - can't rely on fseek() here */
  if(ndiscard != 0) {
    char discardbuf[BUFSIZ];

    for(i = 0; i < ndiscard / BUFSIZ; i++)
      if(fread(discardbuf, BUFSIZ, 1, filei) != 1)
	usage_exit_s(1, "EOF on input when discarding header\n");

    if(ndiscard % BUFSIZ != 0)
      if(fread(discardbuf, ndiscard % BUFSIZ, 1, filei) != 1)
	usage_exit_s(1, "EOF on input when discarding header\n");
  }

  if(!extract) {
    float alpha;
    int nread;

    nwrap = MAX(NWRAP, maxnlpc);

    /* grab some space for the input buffers */
    buffer  = long2d((ulong) nchan, (ulong) (blocksize + nwrap));
    buffer1 = (long*) pmalloc((ulong) (blocksize * sizeof(*buffer1)));
    offset  = long2d((ulong) nchan, (ulong) nmean);
    
    for(chan = 0; chan < nchan; chan++) {
      for(i = 0; i < nwrap; i++) buffer[chan][i] = 0;
      buffer[chan] += nwrap;
    }

    if(maxnlpc > 0)
      qlpc = (int*) pmalloc((ulong) (maxnlpc * sizeof(*qlpc)));
    
    default_offset = init_offset(offset, nchan, nmean, ftype);

    /* write magic number */
    if(fwrite(magic, strlen(magic), 1, fileo) != 1)
      usage_exit_s(1, "could not write the magic number\n");

    /* write version number */
    if(putc(version, fileo) == EOF)
      usage_exit_s(1, "EOF when writing version number\n");

    /* initialise the variable length mode */
    var_put_init(fileo);

    /* put file type and number of channels */
    UINT_PUT(ftype, TYPESIZE, fileo);
    UINT_PUT(nchan, CHANSIZE, fileo);

    /* put blocksize if version > 0 */
    if(version == 0) {
      alpha = ALPHA0;
      if(blocksize != DEFAULT_BLOCK_SIZE) {
	uvar_put((ulong) FN_BLOCKSIZE, FNSIZE, fileo);
	UINT_PUT(blocksize, (int) (log((double) DEFAULT_BLOCK_SIZE) / M_LN2),
		 fileo);
      }
    }
    else {
      alpha = ALPHA1;
      UINT_PUT(blocksize, (int) (log((double) DEFAULT_BLOCK_SIZE) / M_LN2),
	       fileo);
      UINT_PUT(maxnlpc, LPCQSIZE, fileo);
      UINT_PUT(nmean, 0, fileo);
      UINT_PUT(nskip, NSKIPSIZE, fileo);
      for(i = 0; i < nskip; i++) {
	int byte = getc(filei);
	if(byte == EOF)
	  usage_exit_s(1, "EOF when reading header\n");
	uvar_put((ulong) byte, XBYTESIZE, fileo);
      }
    }

    while((nread = fread_type(buffer, ftype, nchan, blocksize, filei)) != 0) {
      /* put blocksize if changed */
      if(nread != blocksize) {
	uvar_put((ulong) FN_BLOCKSIZE, FNSIZE, fileo);
	UINT_PUT(nread, (int) (log((double) blocksize) / M_LN2), fileo);
	blocksize = nread;
      }

      for(chan = 0; chan < nchan; chan++) {
	long coffset, *cbuffer = buffer[chan];
	long sum, fnd;
	int  resn;

	/* test for excessive and exploitable quantisation, and exploit!! */
	{ int newbitshift = find_bitshift(cbuffer, blocksize, ftype);
	  if(newbitshift != bitshift) {
	    uvar_put((ulong) FN_BITSHIFT, FNSIZE, fileo);
	    uvar_put((ulong) newbitshift, BITSHIFTSIZE, fileo);
	    bitshift = newbitshift;
	  }
	}
	  
	/* deduct mean if appropriate */
	if(nmean == 0) coffset = default_offset;
	else {
	  sum = 0;
	  for(i = 0; i < nmean; i++) sum += offset[chan][i];
	  coffset = sum / nmean;
	  
	  sum = 0;
	  for(i = 0; i < blocksize; i++)
	    sum += cbuffer[i];

	  for(i = 1; i < nmean; i++)
	    offset[chan][i - 1] = offset[chan][i];
	  offset[chan][nmean - 1] = sum / blocksize;
	}
	if(coffset != 0)
	  for(i = -nwrap; i < blocksize; i++)
	    cbuffer[i] -= coffset;

	/* find the best model */
	if(maxnlpc == 0) {
	  long sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, last0, last1, last2;

	  last2 = (last1 = (last0 = cbuffer[-1]) - cbuffer[-2]) - (cbuffer[-2]
								 -cbuffer[-3]);
	  for(i = 0; i < blocksize; i++) {
	    long diff0, diff1, diff2, diff3;

	    sum0 += abs(diff0 = cbuffer[i]);
	    sum1 += abs(diff1 = diff0 - last0);
	    sum2 += abs(diff2 = diff1 - last1);
	    sum3 += abs(diff3 = diff2 - last2);

	    last0 = diff0;
	    last1 = diff1;
	    last2 = diff2;
	  }

	  if(sum0 < MIN(MIN(sum1, sum2), sum3)) {
	    sum = sum0;
	    fnd = FN_DIFF0;
	  }
	  else if(sum1 < MIN(sum2, sum3)) {
	    sum = sum1;
	    fnd = FN_DIFF1;
	  }
	  else if(sum2 < sum3) {
	    sum = sum2;
	    fnd = FN_DIFF2;
	  }
	  else {
	    sum = sum3;
	    fnd = FN_DIFF3;
	  }

	  if(alpha * sum < blocksize) resn = 0;
	  else resn = log(alpha * sum / (double) blocksize) / M_LN2 + 0.5;

	  if(resn > maxresn) {
	    bitshift = resn - maxresn;
	    for(i = 0; i < blocksize; i++) cbuffer[i] >>= bitshift;
	    resn = maxresn;
	  }
	    
	  uvar_put(fnd, FNSIZE, fileo);
	  uvar_put((ulong) resn, ENERGYSIZE, fileo);
	  
	  switch(fnd) {
	  case FN_DIFF0:
	    for(i = 0; i < blocksize; i++)
	      VAR_PUT(cbuffer[i], resn, fileo);
	    break;
	  case FN_DIFF1:
	    for(i = 0; i < blocksize; i++)
	      VAR_PUT(cbuffer[i] - cbuffer[i - 1], resn, fileo);
	    break;
	  case FN_DIFF2:
	    for(i = 0; i < blocksize; i++)
	      VAR_PUT(cbuffer[i] - 2 * cbuffer[i - 1] + cbuffer[i - 2],
		      resn, fileo);
	    break;
	  case FN_DIFF3:
	    for(i = 0; i < blocksize; i++)
	      VAR_PUT(cbuffer[i] - 3 * (cbuffer[i - 1] - cbuffer[i - 2]) -
		      cbuffer[i - 3], resn, fileo);
	    break;
	  }
	}
	else { /* maxnlpc > 0 so do lpc analysis */
	  int nlpc = wav2lpc(cbuffer, blocksize, qlpc, maxnlpc, &resn);

	  if(resn > maxresn) {
	    bitshift = resn - maxresn;
	    for(i = 0; i < blocksize; i++) cbuffer[i] >>= bitshift;
	    resn = maxresn;
	  }

	  uvar_put((ulong) FN_QLPC, FNSIZE, fileo);
	  uvar_put((ulong) resn, ENERGYSIZE, fileo);
	  uvar_put((ulong) nlpc, LPCQSIZE, fileo);
	  for(i = 0; i < nlpc; i++)
	    var_put((long) qlpc[i], LPCQUANT, fileo);

	  /* use the quantised LPC coefficients to generate the residual */
	  for(i = 0; i < blocksize; i++) {
	    int j;
	    long sum = 0;

	    for(j = 0; j < nlpc; j++)
	      sum += qlpc[j] * cbuffer[i - j - 1];
	    var_put(cbuffer[i] - (sum >> LPCQUANT), resn, fileo);
	  }
	}
	
	/* do the wrap */
	for(i = -nwrap; i < 0; i++)
	  cbuffer[i] = cbuffer[i + blocksize] + coffset;
      }
    }

    /* wind up the variable frame rate file */
    uvar_put((ulong) FN_QUIT, FNSIZE, fileo);
    var_put_quit(fileo);

    /* and free the space used */
    free((char*) buffer);
    free((char*) buffer1);
    free((char*) offset);
    if(maxnlpc > 0)
      free((char*) qlpc);
  }
  else {
    /***********************/
    /* EXTRACT starts here */
    /***********************/

    int i, cmd;

    /* read magic number */
    for(i = 0; i < strlen(magic); i++)
      if(getc(filei) != magic[i])
	usage_exit_s(1, "Bad magic number\n");

    /* get and check version number */
    if((version = getc(filei)) == EOF)
      update_exit_s(1, "EOF when reading version number\n");
    if(version > FORMAT_VERSION)
      update_exit_sd(1, "can't decode version %d\n", version);

    /* initialise the variable length mode */
    var_get_init(filei);

    /* get file type and set up appropriately, ignoring command line state */
    ftype = UINT_GET(TYPESIZE, filei);
    nchan = UINT_GET(CHANSIZE, filei);

    /* get blocksize if version > 0 */
    if(version > 0) {
      blocksize = UINT_GET((int) (log((double) DEFAULT_BLOCK_SIZE) / M_LN2),
			   filei);
      maxnlpc = UINT_GET(LPCQSIZE, filei);
      nmean = UINT_GET(0, fileo);
      nskip = UINT_GET(NSKIPSIZE, filei);
      for(i = 0; i < nskip; i++) {
	int byte = uvar_get(XBYTESIZE, filei);
	if(putc(byte, fileo) == EOF)
	  usage_exit_s(1, "EOF when writing header\n");
      }
    }
    else
      blocksize = DEFAULT_BLOCK_SIZE;
    nwrap = MAX(NWRAP, maxnlpc);

    /* grab some space for the input buffer */
    buffer  = long2d((ulong) nchan, (ulong) (blocksize + nwrap));
    offset  = long2d((ulong) nchan, (ulong) nmean);

    for(chan = 0; chan < nchan; chan++) {
      for(i = 0; i < nwrap; i++) buffer[chan][i] = 0;
      buffer[chan] += nwrap;
    }

    if(maxnlpc > 0)
      qlpc = (int*) pmalloc((ulong) (maxnlpc * sizeof(*qlpc)));
    
    default_offset = init_offset(offset, nchan, nmean, ftype);

    /* get commands from file and execute them */
    chan = 0;
    while((cmd = uvar_get(FNSIZE, filei)) != FN_QUIT)
      switch(cmd) {
      case FN_DIFF0:
      case FN_DIFF1:
      case FN_DIFF2:
      case FN_DIFF3:
      case FN_QLPC:
  	{ int resn = uvar_get(ENERGYSIZE, filei);
	  long coffset, *cbuffer = buffer[chan];
	  int nlpc, j;

	  /* this is a hack as version 0 differed in definition of var_get() */
	  if(version == 0) resn--;

	  /* find offset */
	  if(nmean == 0) coffset = default_offset;
	  else {
	    long sum = 0;
	    for(i = 0; i < nmean; i++) sum += offset[chan][i];
	    coffset = sum / nmean;
	  }
	  
	  switch(cmd) {
	  case FN_DIFF0:
	    for(i = 0; i < blocksize; i++)
	      cbuffer[i] = var_get(resn, filei) + coffset;
	    break;
	  case FN_DIFF1:
	    for(i = 0; i < blocksize; i++)
	      cbuffer[i] = var_get(resn, filei) + cbuffer[i - 1];
	    break;
	  case FN_DIFF2:
	    for(i = 0; i < blocksize; i++)
	      cbuffer[i] = var_get(resn, filei) + (2 * cbuffer[i - 1] -
						   cbuffer[i - 2]);
	    break;
	  case FN_DIFF3:
	    for(i = 0; i < blocksize; i++)
	      cbuffer[i] = var_get(resn, filei) + 3 * (cbuffer[i - 1] -
					     cbuffer[i - 2]) + cbuffer[i - 3];
	    break;
	   case FN_QLPC:
	    nlpc = uvar_get(LPCQSIZE, filei);
	    for(i = 0; i < nlpc; i++)
	      qlpc[i] = var_get(LPCQUANT, filei);
	    for(i = 0; i < nlpc; i++)
	      cbuffer[i - nlpc] -= coffset;
	    for(i = 0; i < blocksize; i++) {
	      long sum = 0;
	      for(j = 0; j < nlpc; j++)
		sum += qlpc[j] * cbuffer[i - j - 1];
	      cbuffer[i] = var_get(resn, filei) + (sum >> LPCQUANT);
	    }
	    if(coffset != 0)
	      for(i = 0; i < blocksize; i++)
		cbuffer[i] += coffset;
	    break;
	  }

	  /* store mean value if appropriate */
	  if(nmean > 0) {
	    long sum = 0;

	    for(i = 1; i < nmean; i++)
	      offset[chan][i - 1] = offset[chan][i];
	  
	    for(i = 0; i < blocksize; i++)
	      sum += cbuffer[i];

	    offset[chan][nmean - 1] = sum / blocksize;
	  }

	  /* do the wrap */
	  for(i = -nwrap; i < 0; i++) cbuffer[i] = cbuffer[i + blocksize];

	  fix_bitshift(cbuffer, blocksize, bitshift, ftype);
	  
	  if(chan == nchan - 1)
	    fwrite_type(buffer, ftype, nchan, blocksize, fileo);
	  chan = (chan + 1) % nchan;
	}
	break;
      case FN_BLOCKSIZE:
	blocksize = UINT_GET((int) (log((double) blocksize) / M_LN2), filei);
	break;
      case FN_BITSHIFT:
	bitshift = uvar_get(BITSHIFTSIZE, filei);
	break;
      default:
	update_exit_sd(1, "sanity check fails trying to decode function: %d\n",
		       cmd);
      }
    
    /* wind up */
    var_get_quit(filei);

    free((char*) buffer);
    free((char*) offset);
    if(maxnlpc > 0)
      free((char*) qlpc);
  }    

#ifdef NOT_FOR_EMBEDDED_CODE
  /* close the files */
  if(filei != stdi) fclose(filei);
  if(fileo != stdo) fclose(fileo);
     
  /* make the compressed file look like the original if possible */
  if((filei != stdi) && (fileo != stdo))
    (void) dupfileinfo(filenamei, filenameo);

  if(nfilename == 1)
    if(unlink(filenamei))
      perror_exit_ss("unlink(%s)", filenamei);
#endif

  if(tmpfilename != NULL)
    free(tmpfilename);
  
  /* quit happy */   
  return(0);
}

