#include <stdio.h>
#include <sp/sphere.h>
#include <string.h>

/*
 *
 *  sp_write_data()
 *
 */

int sp_write_data(buffer,sample_size,num_sample,sp)
void *buffer;
size_t sample_size;
size_t num_sample;
SP_FILE *sp;
{
    char *proc_name="sp_write_data", *str;
    SP_CHECKSUM checksum=0;
    int ret;
    long lint, header_size, data_bytes;
    
    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if (buffer == (void *)0) 
	return_err(proc_name,100,0,"Null memory buffer"); 
    if (sp == SPNULL)
	return_err(proc_name,101,0,"Null SPFILE structure");
    if (sp->open_mode == SP_mode_read)
	return_err(proc_name,104,0,"Unable to write data to a file opened for reading");
    if (sample_size != sp->write_spifr->status->user_sample_n_bytes) 
	return_err(proc_name,102,0,
		   rsprintf("Sample size %d does not match the expected size %d",
			    sample_size, sp->write_spifr->status->user_sample_n_bytes));
    if (num_sample < 0)
	return_err(proc_name,103,0,
		   rsprintf("Negative sample count %d",num_sample));

    if (sp->write_spifr->status->write_occured_flag == FALSE) { /* set up the FoB Structure for reading */
	/******************************************************/
	/*             Check for required fields              */

	/* the sample_n_bytes is required, if it is not set, there is an error */
	if (h_get_field(sp->write_spifr->status->file_header, SAMPLE_N_BYTES_FIELD, T_INTEGER, &lint) != 0)
	    return_err(proc_name,150,0,rsprintf("Header field '%s' is missing",SAMPLE_N_BYTES_FIELD));
	
	/* if 'channel_count' is not in the header, there is an error */
	if (h_get_field(sp->write_spifr->status->file_header, CHANNEL_COUNT_FIELD, T_INTEGER, &lint) != 0)
	    return_err(proc_name,151,0,rsprintf("Header field '%s' is missing",CHANNEL_COUNT_FIELD));

	if (! sp->write_spifr->status->is_disk_file) 
	    if (h_get_field(sp->write_spifr->status->file_header, SAMPLE_COUNT_FIELD, T_INTEGER, &lint) != 0)
		return_err(proc_name,151,0,rsprintf("Header field '%s' is missing for stream file",SAMPLE_COUNT_FIELD));
	
	/* if the sample coding field is not 'raw' the sample_rate field must exist */
        /* if the field is missing, it is assumed to be a pcm file                  */
	ret=h_get_field(sp->write_spifr->status->file_header, SAMPLE_CODING_FIELD, T_STRING, &str);
	if (((ret == 0) && (!strsame(str,"raw"))) || (ret != 0))
	    if (h_get_field(sp->write_spifr->status->file_header, SAMPLE_RATE_FIELD, T_INTEGER, &lint) != 0) {
		if (ret == 0) mtrf_free(str);
		return_err(proc_name,151,0,rsprintf("Header field '%s' is missing from wave type file",SAMPLE_RATE_FIELD));
	    }
	if (ret == 0) mtrf_free(str);

	/* if the following fields are missing from the header, default values need to */
	/* be assumed in the SP_FILE status structure                                  */
	if (h_get_field(sp->write_spifr->status->file_header, SAMPLE_BF_FIELD, T_STRING, &str) == 0)
	    mtrf_free(str);
	else {
	    sp->write_spifr->status->user_sbf = sp->write_spifr->status->file_sbf =
		get_natural_sbf(sp->write_spifr->status->file_sample_n_bytes) ;
	}


	/* only add data to the header if the file is not a stream */
	if (sp->write_spifr->status->is_disk_file) {
	    /* if 'sample_count' is not in the header, add it to take up space */
	    /* for later correction                                            */
	    if (h_get_field(sp->write_spifr->status->file_header, SAMPLE_COUNT_FIELD, T_INTEGER, &lint) != 0){
		lint=(-1);

		sp->write_spifr->status->file_sample_count=lint;
		sp_h_set_field(sp, SAMPLE_COUNT_FIELD, T_INTEGER, &lint);
	    }

	    /* if 'sample_checksum' is not in the header, add it with a fake checksum */
	    if (h_get_field(sp->write_spifr->status->file_header, "sample_checksum", T_INTEGER, &lint) != 0){
		lint=999999999;
		sp->write_spifr->status->file_checksum=lint;
		sp_h_set_field(sp, "sample_checksum", T_INTEGER, &lint);
	    }
	}

	/* Flush the header to the file pointer */
	if (sp_write_header(sp->write_spifr->waveform->sp_fp,sp->write_spifr->status->file_header,
			    &header_size,&(sp->write_spifr->waveform->header_data_size)) < 0)
	    return_err(proc_name,200,0,"Unable to write header to file");

	if ((sp->write_spifr->status->user_compress == SP_wc_none) &&
	    (sp->write_spifr->status->file_compress != SP_wc_none)){
	    char *buff;
	    int blen;

	    /* The file needs to be written into a temporary place, and then compressed*/
	    /* if The expected waveform size is bigger that MAX_INTERNAL_WAVEFORM, use */
	    /* a temporary file. otherwise:                                            */
	    /* 1. make an MEMORY FOB struct for the uncompressed file.                 */
	    /* 2. allocate the memory for the FOB struct,                              */
	    /* 3. install the memory into the FOB struct                               */
	    
	    if (sp->write_spifr->status->file_sample_count < 0)
		/* allocate a minimal size for the waveform */
		blen = MAX_INTERNAL_WAVFORM;
	    else
		blen = sp->write_spifr->status->file_channel_count * sp->write_spifr->status->file_sample_count * 
		       sp->write_spifr->status->file_sample_n_bytes;

	    if (blen < MAX_INTERNAL_WAVFORM){
		if ((sp->write_spifr->waveform->sp_fob = fob_create(FPNULL)) == FOBPNULL)
		    return_err(proc_name,300,0,"Unable to allocate a FOB (File or Buffer) structure");
		if ((buff = mtrf_malloc(blen)) == CNULL){
		    fob_destroy(sp->write_spifr->waveform->sp_fob);
		    return_err(proc_name,301,0,"Unable to malloc buffer for waveform data");
		}
		fob_bufinit(sp->write_spifr->waveform->sp_fob, buff, blen);
	    } else {
		FILE *temp_fp;
		sp->write_spifr->status->temp_filename = sptemp_dirfile();
		if (sp->write_spifr->status->temp_filename == CNULL)
		    return_err(proc_name,301,0,"Unable to create usable temporary file");
		if (sp_verbose > 15) printf("Proc %s: Attempting to write a big file %d bytes long, using temp file %s\n",proc_name,
					   blen,sp->write_spifr->status->temp_filename);
		if ((temp_fp=fopen(sp->write_spifr->status->temp_filename,TRUNCATE_UPDATEMODE)) == FPNULL) 
		    return_err(proc_name,302,0,
			       rsprintf("Unable to open temporary file %s",sp->write_spifr->status->temp_filename));
		if ((sp->write_spifr->waveform->sp_fob = fob_create(temp_fp)) == FOBPNULL)
		    return_err(proc_name,303,0,"Unable to allocate a FOB (File or Buffer) structure");
		sp->write_spifr->status->is_temp_file = TRUE;
	    }
	}
	else {
	    /* This assumes that no pre-buffering is required */
/*	    printf("Setting up for un-buffered IO \n");*/
	    if ((sp->write_spifr->waveform->sp_fob = fob_create(sp->write_spifr->waveform->sp_fp)) == FOBPNULL)
		return_err(proc_name,300,0,"Unable to allocate a FOB (File or Buffer) structure");
	    sp->write_spifr->waveform->sp_fp = FPNULL;
	}

	/* Set up byte format the conversions */
	if (sp->write_spifr->status->user_sbf != sp->write_spifr->status->file_sbf) 
	    if (((sp->write_spifr->status->user_sbf == SP_sbf_01) &&
		 (sp->write_spifr->status->file_sbf == SP_sbf_10)) ||
		((sp->write_spifr->status->user_sbf == SP_sbf_10) &&
		 (sp->write_spifr->status->file_sbf == SP_sbf_01)))
		fob_write_byte_swap(sp->write_spifr->waveform->sp_fob);

	if (sp->write_spifr->status->user_encoding != sp->write_spifr->status->user_encoding)
	    return_err(proc_name,400,0,"Unable to convert sample types ... for now\n");

	sp->write_spifr->status->write_occured_flag = TRUE;
    }


    /* WRITE THE DATA INTO FOB */
    ret = fob_fwrite(buffer, sample_size, num_sample, sp->write_spifr->waveform->sp_fob);
    if (ret < 0)
	return_err(proc_name,301,0,"Unable to write data");
    /* Perform the checksum computation */
    switch (sp->write_spifr->status->user_encoding){
      case SP_se_pcm2:
	if (sp->write_spifr->status->natural_sbf == sp->write_spifr->status->user_sbf)
	    checksum = sp_compute_short_checksum(buffer, ret, FALSE);
	else
	    checksum = sp_compute_short_checksum(buffer, ret, TRUE);

	sp->write_spifr->waveform->checksum = sp_add_checksum(sp->write_spifr->waveform->checksum,checksum);
	sp->write_spifr->waveform->samples_written += ret;
	break;
      default:
	return_err(proc_name,302,0,"Internal error. user encoding has not been set");
    }

    if (sp_verbose > 11) printf("Proc %s: Requested %d, %d byte samples, written %d\n",proc_name,num_sample,sample_size,ret);
    return_success(proc_name,0,ret,"ok");
}
