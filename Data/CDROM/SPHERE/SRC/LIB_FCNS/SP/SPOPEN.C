#include <stdio.h>
#include <sp/sphere.h>
#include <string.h>

#define READMODE "r"
#define UPDATEMODE "r+"
#define WRITEMODE "w"

/*
 *  sp_open
 *
 */
SP_FILE *sp_open(filename, mode)
char *filename;
char *mode;
{
    SP_FILE *tsp;
    char *errmsg, *proc_name="sp_open", *fopen_mode;
    enum SP_file_open_mode current_mode;

    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if (filename == CNULL) 
	return_err(proc_name,101,SPNULL,"Null filename string");
    if (mode == CNULL) 
	return_err(proc_name,101,SPNULL,"Null file mode string");

    if ((tsp=sp_alloc_and_init_sphere_t()) == SPNULL)
        return_err(proc_name,102,SPNULL,"Unable to malloc SPFILE memory");

    /* set the file open mode in the status structure */
    if (strsame(mode,"r"))
	current_mode = tsp->open_mode = SP_mode_read;
    else if (strsame(mode,"w"))
	current_mode = tsp->open_mode = SP_mode_write;
    else if (strsame(mode,"rv")){
	current_mode = tsp->open_mode = SP_mode_read;
	tsp->read_spifr->status->extra_checksum_verify = TRUE;
    }
    else if (strsame(mode,"wv")) {
	current_mode = tsp->open_mode = SP_mode_write;
	tsp->write_spifr->status->extra_checksum_verify = TRUE;
    }
    else if (strsame(mode,"u")) {
	tsp->open_mode = SP_mode_read;
	current_mode = SP_mode_update;
    }
    else {
	free_sphere_t(tsp);
        return_err(proc_name,103,SPNULL,
		   rsprintf("Illegal SPFILE open mode '%s'",mode));
    }

    /* just open the file, either for reading or writing.  If the      */
    /* mode was SP_mode_writing, and the file exists, change the mode  */ 
    /* to SP_mode_update.                                              */
    switch (tsp->open_mode) {
	case (SP_mode_read): {
	    if (! strsame(filename,"-")) {
		fopen_mode = (current_mode == SP_mode_read) ? READMODE : UPDATEMODE;
		if ((tsp->read_spifr->waveform->sp_fp = fopen(filename,fopen_mode)) == (FILE *)0){
		    free_sphere_t(tsp);
		    return_err(proc_name,111,SPNULL,
			       rsprintf("Unable to open SPHERE file '%s' for reading fopen mode %s",
				    filename,fopen_mode));
		}
		tsp->read_spifr->status->is_disk_file = TRUE;
	    } else {
		tsp->read_spifr->waveform->sp_fp = stdin;
		tsp->read_spifr->status->is_disk_file = FALSE;
	    }
	    tsp->read_spifr->status->external_filename = mtrf_strdup(filename);
	    break;
	}
	case (SP_mode_write):{ 
	    if (! strsame(filename,"-")) {
		/* open the file, truncating if the file exists */
		fopen_mode = (current_mode == SP_mode_write) ? WRITEMODE : UPDATEMODE;
		if ((tsp->write_spifr->waveform->sp_fp = fopen(filename,fopen_mode)) ==
		    (FILE *)0){
		    free_sphere_t(tsp);
		    return_err(proc_name,105,SPNULL,
			       rsprintf("Unable to open SPHERE file '%s' for writing, fopen mode %s",
					filename,fopen_mode));
		}
		tsp->write_spifr->status->is_disk_file = TRUE;
	    } else {
		tsp->write_spifr->waveform->sp_fp = stdout;
		tsp->write_spifr->status->is_disk_file = FALSE;
	    }
	    tsp->write_spifr->status->external_filename = mtrf_strdup(filename);
	    break;
	}
	default: {
	    return_err(proc_name,200,SPNULL,"Internal error");
	}
    }

    /* now that the file is opened, load the header if it exist, */
    /* otherwise alloc an empty header for the user              */
    switch (tsp->open_mode) {
	case (SP_mode_read): {
	    /* read the header */
	    tsp->read_spifr->header = sp_open_header(tsp->read_spifr->waveform->sp_fp,TRUE,&errmsg);
	    if ( tsp->read_spifr->header == HDRNULL ) {
		free_sphere_t(tsp);				
		return_err(proc_name,104,SPNULL,
			   rsprintf("Unable to open SPHERE header of file '%s', %s",
				    filename,errmsg));
	    }
	    /* get the size of the header */
	    if (! strsame(filename,"-")) {
		if ((tsp->read_spifr->waveform->header_data_size = sp_file_header_size(filename)) < 0){
		    free_sphere_t(tsp);
		    return_err(proc_name,110,SPNULL,
			       rsprintf("Unable to get SPHERE header size of file '%s'",filename));
		}
	    } else {
		if ((tsp->read_spifr->waveform->header_data_size = sp_header_size(tsp->read_spifr->header)) < 0){
		    free_sphere_t(tsp);
		    return_err(proc_name,111,SPNULL,
			       rsprintf("Unable to get SPHERE header size of file '%s'",filename));
		}
	    }

            /* duplicate the header for the file interface */
            if ((tsp->read_spifr->status->file_header = sp_dup_header(tsp->read_spifr->header)) == HDRNULL){
		fprintf(stderr,"Error: sp_open_header unable to dup header for ");
		fprintf(stderr,"file '%s'\n",filename);
		free_sphere_t(tsp);		
		return_err(proc_name,106,SPNULL,
			   rsprintf("Unable to duplicate the SPHERE header of file",
				    filename));
	    }
            /* set the default operation settings */
	    if (sp_set_default_operations(tsp) != 0){
		print_return_status(stderr);
		return_err(proc_name,107,SPNULL,
			   rsprintf("Unable to interpret the SPHERE header of file '%s'",
				    filename));
	    }
	    break;	
	}
	case (SP_mode_write):{ 
	    tsp->write_spifr->header = sp_create_header();
	    if ( tsp->write_spifr->header == HDRNULL ) {
		free_sphere_t(tsp);		
		return_err(proc_name,108,SPNULL,
			   rsprintf("Unable to allocate SPHERE header for file '%s'",
				    filename));
	    }
	    tsp->write_spifr->status->file_header = sp_create_header();
	    if ( tsp->write_spifr->status->file_header == HDRNULL ) {
		free_sphere_t(tsp);		
		return_err(proc_name,109,SPNULL,
			   rsprintf("Unable to allocate hidden SPHERE header for file '%s'",
				    filename));
	    }
	}
    }

    /* the file was actually opened for update, so make a temp file, and */
    /* duplicate the read in file. */
    if (current_mode == SP_mode_update){ 
	SP_FILE *tsp2;
	SPIFR *tspifr;
	char *temp_file;
	char data_mode[100];
	temp_file = sptemp(tsp->read_spifr->status->external_filename);
	if (temp_file == CNULL)
	    return_err(proc_name,300,SPNULL,"Unable to create temporary filename");
	if ((tsp2 = sp_open(temp_file,WRITEMODE)) == SPNULL) {
	    free_sphere_t(tsp);		
	    mtrf_free(temp_file);
	    return_err(proc_name,301,SPNULL,rsprintf("Unable to open temporary file",temp_file));
	}
	sp_set_data_mode(tsp,"SE-ORIG:SBF-ORIG");
	/* now copy the header into the update file */
	if (sp_dup(tsp,tsp2) != 0){
	    free_sphere_t(tsp);		
	    free_sphere_t(tsp2);		
	    unlink(temp_file);
	    mtrf_free(temp_file);
	    return_err(proc_name,302,SPNULL,"Unable to duplicate output header");
	}
	*data_mode = '\0';
	/* now set the data mode to match the output format of the read file */
	switch (tsp->read_spifr->status->file_compress){
	  case SP_wc_shorten: strcat(data_mode,"SE-SHORTEN:"); break;
	  case SP_wc_wavpack: strcat(data_mode,"SE-WAVPACK:"); break;
	  case SP_wc_shortpack: strcat(data_mode,"SE-SHORTPACK:"); break;
	}
	switch (tsp->read_spifr->status->file_sbf){
	  case SP_sbf_01: strcat(data_mode,"SBF-01"); break;
	  case SP_sbf_10: strcat(data_mode,"SBF-10"); break;
	  case SP_sbf_1: strcat(data_mode,"SBF-1"); break;
	  case SP_sbf_N: strcat(data_mode,"SBF-N"); break;
	}
	if (sp_set_data_mode(tsp2,data_mode) >= 100){
	    free_sphere_t(tsp);		
	    free_sphere_t(tsp2);		
	    unlink(temp_file);
	    mtrf_free(temp_file);
	    return_err(proc_name,303,SPNULL,
		       rsprintf("Unable to set_data_mode '%s' for update file",data_mode));
	}
	/* now merge the two SPFILE pointers into a single structure */
	/* and free the residual */
	tspifr = tsp->write_spifr;
	tsp->write_spifr = tsp2->write_spifr;
	tsp2->write_spifr = tspifr;
	free_sphere_t(tsp2);
	mtrf_free(temp_file);	
	tsp->open_mode = current_mode;
    }

/*    sp_file_dump(tsp,stdout);*/
    if (sp_verbose > 17) sp_file_dump(tsp,stdout);
    if (sp_verbose > 11) printf("Proc %s: Returning Sphere-file pointer\n",proc_name);
    return_success(proc_name,0,tsp,"ok");
}


sp_file_dump(sp,fp)
SP_FILE *sp;
FILE *fp;
{
    char *enum_str_SP_file_open_mode();

    fprintf(fp,"|==================================================================\n");
    fprintf(fp,"File open mode:      %s\n",
	    enum_str_SP_file_open_mode(sp->open_mode));
    if ((sp->open_mode == SP_mode_read) || (sp->open_mode == SP_mode_update)){
	fprintf(fp,"Read SPIFR:\n");
	spifr_dump(sp->read_spifr,fp);
    }
    if ((sp->open_mode == SP_mode_write) || (sp->open_mode == SP_mode_update)){
	fprintf(fp,"Write SPIFR:\n");
	spifr_dump(sp->write_spifr,fp);
    }
    fprintf(fp,"|==================================================================\n");
}

spifr_dump(spifr,fp)
SPIFR *spifr;
FILE *fp;
{
    char *enum_str_SP_sample_byte_fmt();
    char *enum_str_SP_sample_encoding();
    char *enum_str_SP_waveform_comp();

    fprintf(fp,"|-----------------------------------------------------------------------\n|\n");
    fprintf(fp,"Dump of an SP_FILE structure\n");
    fprintf(fp,"Users file header\n");
    sp_print_lines(spifr->header,fp);
    fprintf(fp,"\n");
    fprintf(fp,"Wave Sructure\n");
    fprintf(fp,"File pointer:     %x\n",spifr->waveform->sp_fp);
    fprintf(fp,"FOB pointer:      %x\n",spifr->waveform->sp_fob);
    fprintf(fp,"Samples Read:     %d\n",spifr->waveform->samples_read);
    fprintf(fp,"Samples written:  %d\n",spifr->waveform->samples_written);
    fprintf(fp,"Checksum:         %d\n",spifr->waveform->checksum);
    fprintf(fp,"Header Data Size: %d\n",spifr->waveform->header_data_size);
    fprintf(fp,"Read Pre-Mat. EOF %d\n",spifr->waveform->read_premature_eof);
    fprintf(fp,"Failed Checksum   %d\n",spifr->waveform->failed_checksum);
    
    fprintf(fp,"\n");

    fprintf(fp,"Status Structure\n");
    fprintf(fp,"External file name:  %s\n",spifr->status->external_filename);
    fprintf(fp,"The File header\n");
    sp_print_lines(spifr->status->file_header,fp);
    fprintf(fp,"Write Occured Flag:  %d\n",spifr->status->write_occured_flag);
    fprintf(fp,"Read Occured Flag:   %d\n",spifr->status->read_occured_flag);
    fprintf(fp,"Field Set Occ. Flag: %d\n",spifr->status->field_set_occured_flag);
    fprintf(fp,"S_D_MODE Occ. Flg:   %d\n",spifr->status->set_data_mode_occured_flag);
    fprintf(fp,"File checksum:       %d\n",spifr->status->file_checksum);   
    fprintf(fp,"Ignore checksum:     %d\n",spifr->status->ignore_checksum);   
    fprintf(fp,"Nat Sample Byte Fmt: %s\n",
	    enum_str_SP_sample_byte_fmt(spifr->status->natural_sbf));
    fprintf(fp,"Extra Checksum Check %d\n",spifr->status->extra_checksum_verify);
    fprintf(fp,"Is Disk File         %d\n",spifr->status->is_disk_file);
    fprintf(fp,"Is Temp File         %d\n",spifr->status->is_temp_file);
    fprintf(fp,"Temp File Name       %s\n",spifr->status->temp_filename);

    fprintf(fp,"                                   USER");
    fprintf(fp,"                    FILE\n");
    fprintf(fp,"Channel count:    %22d  %22d\n",
	    spifr->status->user_channel_count,spifr->status->file_channel_count);
    fprintf(fp,"Sample Count:     %22d  %22d\n",
	    spifr->status->user_sample_count,spifr->status->file_sample_count);
    fprintf(fp,"Sample Rate:      %22d  %22d\n",
	    spifr->status->user_sample_rate, spifr->status->file_sample_rate); 
    fprintf(fp,"Sample N bytes:   %22d  %22d\n",
           spifr->status->user_sample_n_bytes, spifr->status->file_sample_n_bytes);
    fprintf(fp,"Sample Byte Fmt:  %22s  %22s\n",
	    enum_str_SP_sample_byte_fmt(spifr->status->user_sbf),
	    enum_str_SP_sample_byte_fmt(spifr->status->file_sbf));
    fprintf(fp,"File Coding:      %22s  %22s\n",
	    enum_str_SP_sample_encoding(spifr->status->user_encoding),
	    enum_str_SP_sample_encoding(spifr->status->file_encoding));
    fprintf(fp,"File Compress:    %22s  %22s\n",
	    enum_str_SP_waveform_comp(spifr->status->user_compress),
	    enum_str_SP_waveform_comp(spifr->status->file_compress));
    fprintf(fp,"|\n|-----------------------------------------------------------------------\n");

}

char *enum_str_SP_file_open_mode(id)
enum SP_file_open_mode id;
{
    switch (id){
	  case SP_mode_read : return("SP_mode_read");
	  case SP_mode_write : return("SP_mode_write");
	  case SP_mode_update : return("SP_mode_update");
	  case SP_mode_null : return("SP_mode_null");
	  default: return("UNKNOWN");
    }
}

char *enum_str_SP_sample_encoding(id)
enum SP_sample_encoding id;
{
    switch (id){
	case SP_se_pcm2: return("SP_se_pcm2");
	case SP_se_pcm1: return("SP_se_pcm1");
	case SP_se_ulaw: return("SP_se_ulaw");
	case SP_se_raw: return("SP_se_raw");
	case SP_se_null: return("SP_se_null");
	default: return("UNKNOWN");
    }
}

char *enum_str_SP_waveform_comp(id)
enum SP_waveform_comp id;
{
    switch (id){
	case SP_wc_shorten: return("SP_wc_shorten");
	case SP_wc_wavpack: return("SP_wc_wavpack");
	case SP_wc_shortpack: return("SP_wc_shortpack");
	case SP_wc_none: return("SP_wc_none");
	case SP_wc_null: return("SP_wc_null");
	default: return("UNKNOWN");
    }
}
char *enum_str_SP_sample_byte_fmt(id)
enum SP_sample_byte_fmt id;
{
    switch (id){
	  case SP_sbf_01: return("SP_sbf_01");
	  case SP_sbf_10: return("SP_sbf_10");
	  case SP_sbf_1: return("SP_sbf_1");
	  case SP_sbf_N: return("SP_sbf_N");
	  case SP_sbf_orig: return("SP_sbf_orig");
	  case SP_sbf_null: return("SP_sbf_null");
	  default: return("UNKNOWN");
    }
}

sp_set_default_operations(sp)
SP_FILE *sp;
{
    long lint;
    char *str, *proc_name="sp_set_default_operations";
    SPIFR *spifr;
    struct header_t *header;
    int ret;

    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if (sp == SPNULL) return_err(proc_name,100,100,"Null SPFILE pointer");
    if ((sp->open_mode == SP_mode_read) || (sp->open_mode == SP_mode_update))
        spifr = sp->read_spifr;
    else if (sp->open_mode == SP_mode_write)
        spifr = sp->write_spifr;
    else
        return_err(proc_name,100,100,"Unknown File Mode");

    /****************************************************************************/
    /*    The following fields are REQUIRED for Read operations, NO Exceptions  */

    if (sp_h_get_field(sp,SAMPLE_COUNT_FIELD,T_INTEGER,&lint) != 0)
	return_err(proc_name,101,101,rsprintf("Missing '%s' header field",SAMPLE_COUNT_FIELD));
    spifr->status->user_sample_count = spifr->status->file_sample_count = (int)lint;
    if (lint <= 0)
	return_err(proc_name,108,108,rsprintf("Field '%s' value out of range, > 0",lint));

    if (sp_h_get_field(sp,SAMPLE_N_BYTES_FIELD,T_INTEGER,&lint) != 0)
	return_err(proc_name,104,104,rsprintf("Missing '%s' header field",SAMPLE_N_BYTES_FIELD));
    spifr->status->user_sample_n_bytes = spifr->status->file_sample_n_bytes = (int)lint;
    if (lint <= 0)
	return_err(proc_name,108,108,rsprintf("Field '%s' value out of range, > 0",lint));

    if (sp_h_get_field(sp,CHANNEL_COUNT_FIELD,T_INTEGER,&lint) != 0)
	return_err(proc_name,105,105,rsprintf("Missing '%s' header field",CHANNEL_COUNT_FIELD));
    spifr->status->user_channel_count = spifr->status->file_channel_count = (int)lint;
    if (lint <= 0)
	return_err(proc_name,108,108,rsprintf("Field '%s' value out of range, > 0",lint));

    /****************************************************************************/
    /*    The following fields may exist, if they do not, there is a default    */

    /***** NOTE:  only set the file_sbf, Sp_set_data_mode is called to set the  */
    /*****        user_sbf                                                      */
    if ((ret=sp_h_get_field(sp,SAMPLE_BF_FIELD,T_STRING,&str)) != 0){
	if ((spifr->status->file_sbf = get_natural_sbf(spifr->status->user_sample_n_bytes)) == SP_sbf_null)
	    return_err(proc_name,107,107,rsprintf("Unable to read sample sizes of %d bytes",
						  spifr->status->user_sample_n_bytes));
    } else {  /* str holds the sample_byte_format_value */
	ret = parse_sample_byte_format(str,&(spifr->status->file_sbf),spifr->status->user_sample_n_bytes);
	if (ret == 1000) { /* then the file was compressed using change_wav_format */
	    spifr->status->file_sbf = get_natural_sbf(spifr->status->user_sample_n_bytes);
	    spifr->status->file_compress = SP_wc_shortpack;

	    if ((h_set_field(spifr->header,SAMPLE_BF_FIELD,T_STRING,get_natural_byte_order()) != 0) ||
		(h_set_field(spifr->status->file_header,SAMPLE_BF_FIELD,T_STRING,get_natural_byte_order()) != 0)){
		sp_print_return_status(stdout);
		mtrf_free(str);
		return_err(proc_name,110,110,"Unable to re-set sample byte format field for a shortpacked file");
	    }
	    if ((h_set_field(spifr->header,SAMPLE_CODING_FIELD,T_STRING,rsprintf("pcm,embedded-%s",str)) != 0) ||
		(h_set_field(spifr->status->file_header,SAMPLE_CODING_FIELD,T_STRING,rsprintf("pcm,embedded-%s",str)) != 0)){
		sp_print_return_status(stdout);
		mtrf_free(str);
		return_err(proc_name,111,111,"Unable to re-set sample coding field for a shortpacked file");
	    }
	} else if (ret != 0) { /* there really was an error */
	    mtrf_free(str);
	    return_err(proc_name,106,106,"Unable to parse the 'sample_byte_format' field");
	}
	mtrf_free(str);
    }
              /***** field break *****/	    
    if (sp_h_get_field(sp,SAMPLE_CODING_FIELD,T_STRING,&str) != 0)
        str = mtrf_strdup("pcm");  /* the default, since old Corpora are missing this field */
    
    if (parse_sample_coding(str,spifr->status->file_sample_n_bytes,
			    &(spifr->status->file_encoding),
			    &(spifr->status->file_compress)) != 0){
	mtrf_free(str);
	print_return_status(stderr);
	return_err(proc_name,107,107,
		   rsprintf("Unable to parse sample_coding value '%s' header field",str));
    }
    mtrf_free(str);

    /****************************************************************************/
    /*    The following fields are conditionally required.                      */
    ret = sp_h_get_field(sp,SAMPLE_RATE_FIELD,T_INTEGER,&lint);
    switch (spifr->status->file_encoding) {
      case SP_se_pcm1:
      case SP_se_pcm2:
      case SP_se_ulaw:
	if (ret != 0)
	    return_err(proc_name,102,102,rsprintf("Header field '%s' missing, but required for waveform data",
						  SAMPLE_RATE_FIELD));
	spifr->status->user_sample_rate = spifr->status->file_sample_rate = (int)lint;
	break;
      case SP_se_raw:
      default:
	spifr->status->user_sample_rate = spifr->status->file_sample_rate = 0;
    }
	  
    /****************************************************************************/
    /*    The following fields are OPTIONAL, but if they exist, there is a      */
    /*    special purpose for them                                              */

    if (sp_h_get_field(sp,SAMPLE_CHECKSUM_FIELD,T_INTEGER,&lint) == 0)
	spifr->status->file_checksum = lint;
    else {
	spifr->status->ignore_checksum = TRUE;
    }

    /*********************/
    /* Consitency checks */
    /*********************/

    if (spifr->status->file_encoding == SP_se_ulaw &&
	spifr->status->file_sample_n_bytes != 1) 
	return_err(proc_name,120,120,rsprintf("Ulaw encoding requires a 1 byte sample, however the header value is %d\n",
					      spifr->status->file_sample_n_bytes));
    if (spifr->status->file_encoding == SP_se_pcm1 &&
	spifr->status->file_sample_n_bytes != 1) 
	return_err(proc_name,120,120,rsprintf("PCM1 encoding requires a 1 byte sample, however the header value is %d\n",
					      spifr->status->file_sample_n_bytes));
    if (spifr->status->file_encoding == SP_se_pcm1 &&
	spifr->status->file_sample_n_bytes != 2) 
	return_err(proc_name,120,120,rsprintf("PCM2 encoding requires a 2 byte sample, however the header value is %d\n",
					      spifr->status->file_sample_n_bytes));

    /********************************/
    /* set up the default decodings */

    if (sp->open_mode == SP_mode_read)
	if (sp_set_data_mode(sp,"") != 0){
	    print_return_status(stderr);
	    return_err(proc_name,110,110,"Unable to set up default encodings on file opened for read");
	}
    else if (sp->open_mode == SP_mode_update)
	if (sp_set_data_mode(sp,"SE_ORIG:SBF_ORIG") != 0){
	    print_return_status(stderr);
	    return_err(proc_name,111,111,"Unable to set up default encodings on file opened for update");
	}

    if (sp_verbose > 11) printf("Proc %s: Returning 0\n",proc_name);
    return_success(proc_name,0,0,"ok");
}

int parse_sample_byte_format(str,sbf)
char *str;
enum SP_sample_byte_fmt *sbf;
{
    char *proc_name="parse_sample_byte_format";

    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if (str == CNULL) 
	return_err(proc_name,100,100,"Null sample_byte_format_string");
    if (sbf == (enum SP_sample_byte_fmt *)0)
	return_err(proc_name,101,101,"Null sbf pointer");

    if (strsame(str,"01"))
        *sbf = SP_sbf_01;
    if (strsame(str,"10"))
        *sbf = SP_sbf_10;
    if (strsame(str,"1"))
        *sbf = SP_sbf_1;
    if (strstr(str,"shortpack") != CNULL) {
	/* this return value must remain 1000, other functions depend on it */
	return_err(proc_name,1000,1000,
		   rsprintf("Unknown sample_byte_format value '%s' in header",str));
    }
    if (sp_verbose > 11) printf("Proc %s: Returning 0\n",proc_name);
    return_success(proc_name,0,0,"ok");
}    

int parse_sample_coding(str,sample_n_bytes,sample_encoding,wav_compress)
char *str;
int sample_n_bytes;
enum SP_waveform_comp *wav_compress;
enum SP_sample_encoding *sample_encoding;   
{
    int enc_set=FALSE, comp_set=FALSE;
    char *pstr, *str_mem;
    char *proc_name="parse_sample_coding";
    
    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);

    if (str == CNULL)
	return_err(proc_name,101,101,"Null coding string");
    if (sample_n_bytes < 1 || sample_n_bytes > 2)
	return_err(proc_name,102,102,rsprintf("Sample_n_bytes %d out of range",sample_n_bytes));
    if (sample_encoding == (enum SP_sample_encoding *)0)
	return_err(proc_name,103,103,"Null sample encoding pointer");
    if (wav_compress == (enum SP_waveform_comp *)0)
	return_err(proc_name,104,104,"Null waveform compress pointer");

    *wav_compress = SP_wc_null;
    *sample_encoding = SP_se_null;

    if (sp_verbose > 16) fprintf(stderr,"%s: string IS %s\n",proc_name,str);

    /* the algorithm to parse the sample encoding field is : */
    /*    1: get a token before a ',' or NULL */
    /*    2: set a flag to what it matches    */
    /*    3: move past the token              */
    /*    4: loop to (1)                      */
      
    /* make a duplicate copy because strtok is destructive */
    str_mem = mtrf_strdup(str);
    pstr = strtok(str_mem,",");
    while (pstr != CNULL){
	if (sp_verbose > 16) fprintf(stderr,"%s: token found = %s\n",proc_name,pstr);
	if (strsame(pstr,"pcm")){
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc_name,105,105,"Multiple sample encodings in header field");
	    }
	    if (sample_n_bytes == 1)
		*sample_encoding = SP_se_pcm1;
	    else
		*sample_encoding = SP_se_pcm2;
	    enc_set = TRUE;
	}
	else if (strsame(pstr,"ulaw")) {
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc_name,105,105,"Multiple sample encodings in header field");
	    }
	    *sample_encoding = SP_se_ulaw;
	    enc_set = TRUE;
	}
	else if (strsame(pstr,"raw")){
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc_name,105,105,"Multiple sample encodings in header field");
	    }
	    *sample_encoding = SP_se_raw;
	    enc_set = TRUE;
	}
	else if (strstr(pstr,"embedded-shorten-v") != CNULL) {
	    if (comp_set) {
		mtrf_free(str_mem);
		return_err(proc_name,106,106,
			   "Multiple waveform compressions in header field");
	    }
	    *wav_compress = SP_wc_shorten;
	    comp_set = TRUE;
	}
	else if (strstr(pstr,"embedded-wavpack") != CNULL) {
	    if (comp_set){
		mtrf_free(str_mem);
		return_err(proc_name,106,106,
			   "Multiple waveform compressions in header field");
	    }
	    *wav_compress = SP_wc_wavpack;
	    comp_set = TRUE;
	}
	else if (strstr(pstr,"embedded-shortpack-v") != CNULL) {
	    if (comp_set){
		mtrf_free(str_mem);
		return_err(proc_name,106,106,
			   "Multiple waveform compressions in header field");
	    }
	    *wav_compress = SP_wc_shortpack;
	    comp_set = TRUE;
	}
	else {
	    mtrf_free(str_mem);
	    return_err(proc_name,107,107,"Unknown token in sample coding field");
	}
	pstr = strtok(CNULL,",");
    }
    if (*wav_compress == SP_wc_null)
	*wav_compress = SP_wc_none;
    if (*sample_encoding == SP_se_null)
	*sample_encoding = SP_se_pcm2;
    mtrf_free(str_mem);
    if (sp_verbose > 11) printf("Proc %s: Returning 0\n",proc_name);
    return_success(proc_name,0,0,"ok");
}


