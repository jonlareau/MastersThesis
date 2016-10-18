#include <stdio.h>
#include <sp/sphere.h>
#include <string.h>

/*
 *  This function just computes the checksum of the file pointed to by 
 *  an FOB structure.
 */
int fob_short_checksum(f, checksum, swap_bytes, compute_checksum, add_checksum, success)
FOB *f;
int swap_bytes;
SP_CHECKSUM *checksum, (*compute_checksum)(), (*add_checksum)();
{
    long cur_fp_pos;
    short buff[1024];
    int len, ss;
    SP_CHECKSUM chks;

    chks = *checksum = 0;
    ss = sizeof(short);
    if (fob_is_fp(f)){
	cur_fp_pos = ftell(f->fp);
	while ((len=fread(buff,ss,1024,f->fp)) > 0){
	    chks = (*compute_checksum)(buff, len, swap_bytes);
	    *checksum = (*add_checksum)(*checksum,chks);
	}
	if (ferror(f->fp)){
	    fseek(f->fp,cur_fp_pos,SEEK_SET);
	    return(-1);
	}
	fseek(f->fp,cur_fp_pos,SEEK_SET);
	clearerr(f->fp);
	return(0);
    } else {
        *checksum = (*compute_checksum)(f->buf,f->length/2, swap_bytes);
	success=0;
	return(0);
    }
}

/*
 *
 *  sp_read_data()
 *
 */

int sp_read_data(buffer,sample_size,num_sample,sp)
void *buffer;
size_t sample_size;
size_t num_sample;
SP_FILE *sp;
{
    char *proc_name="sp_read_data";
    SP_CHECKSUM checksum;
    int ret;
    
    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if (buffer == (void *)0) 
	return_err(proc_name,100,0,"Null memory buffer"); 
    if (sp == SPNULL)
	return_err(proc_name,101,0,"Null SPFILE structure");
    if (sp->open_mode != SP_mode_read) 
	return_err(proc_name,104,104,"Read on a file not opened for read");
    if (sample_size != sp->read_spifr->status->user_sample_n_bytes) 
	return_err(proc_name,102,0,
		   rsprintf("Sample size %d does not match the expected size %d",
			    sample_size, sp->read_spifr->status->user_sample_n_bytes));
    if (num_sample < 0)
	return_err(proc_name,103,0,
		   rsprintf("Negative sample count %d",num_sample));

    if (sp->read_spifr->waveform->failed_checksum)
	return_err(proc_name,1001,0,"Pre-Read Checksum Test Failed");


    if (sp->read_spifr->status->read_occured_flag == FALSE) { /* set up the FoB Structure for reading */
	sp->read_spifr->status->read_occured_flag = TRUE;

	if ((sp->read_spifr->status->user_compress == SP_wc_none) &&
	    (sp->read_spifr->status->file_compress != SP_wc_none)){
	    int decomp_into_memory=TRUE;
	    int wav_bytes=0;
	    FOB *fob_in, *fob_out;
	    char *buff;
	    int blen;
	    wav_bytes = sp->read_spifr->status->user_sample_count * 
   		        sp->read_spifr->status->user_channel_count * 
			sp->read_spifr->status->user_sample_n_bytes;

	    /* the file must be decompressed, Question: Should it be done in memory?   */
	    if (wav_bytes > MAX_INTERNAL_WAVFORM)
		decomp_into_memory = FALSE;

	    /* The file needs to be de_compressed into memory !!!! */
	    /* 1. make an FOB struct for the uncompressed waveform to be read into and */
	    /*    the original file */
	    /* 2. allocate memory for the entire waveform */
	    /* 3. decompress the file */
	    /* 4. Clean up the FOB struct for the file, moving the fp from the FOB */
	    /* 5. reset the uncompressed FOB struct to the beginning of the memory */
		
	    
	    if (decomp_into_memory) {
		if (sp_verbose > 15) printf("Proc %s: Pre-buffering compressed data into memory\n",proc_name);
		if (fob_create2(sp->read_spifr->waveform->sp_fp, FPNULL, &fob_in, &fob_out) < 0)
		    return_err(proc_name,200,0,"Unable to setup for decompression");
		blen = sp->read_spifr->status->file_channel_count * sp->read_spifr->status->file_sample_count * 
		    sp->read_spifr->status->file_sample_n_bytes;
		if ((buff=mtrf_malloc(blen)) == CNULL){
		    fob_destroy(fob_in);
		    fob_destroy(fob_out);
		    return_err(proc_name,201,0,"Unable to malloc memory to decompress into");
		}
		fob_bufinit(fob_out, buff, blen);
	    } else { /* decompress into a disk file */
		FILE *temp_fp;
		if (sp_verbose > 15) printf("Proc %s: Pre-buffering compressed data into a temporary file\n",proc_name);
		sp->read_spifr->status->temp_filename = sptemp_dirfile();
		if (sp->read_spifr->status->temp_filename == CNULL)
		    return_err(proc_name,400,0,"Unable to create usable temporary file");
		if (sp_verbose > 15) printf("Proc %s: Attempting to read a big file %d bytes long, using temp file %s\n",proc_name,
					   wav_bytes,sp->read_spifr->status->temp_filename);
		if ((temp_fp=fopen(sp->read_spifr->status->temp_filename,TRUNCATE_UPDATEMODE)) == FPNULL) 
		    return_err(proc_name,401,0,
			       rsprintf("Unable to open temporary file %s",sp->read_spifr->status->temp_filename));
		if (fob_create2(sp->read_spifr->waveform->sp_fp, temp_fp, &fob_in, &fob_out) < 0)
		    return_err(proc_name,402,0,"Unable to setup for decompression");

		/* the FILE pointer will be closed after completion of the decompression */
		/* directly from the fob_in FOB pointer.  this is the bug which Francis  */
		/* caught.                                                               */

                /* Note:  Do NOT set the waveform file to FPNULL here (see bugfix below.)  ***PSI***  24-Sep-1993 */
		/* sp->read_spifr->waveform->sp_fp = FPNULL; */
		sp->read_spifr->status->is_temp_file = TRUE;
	    }
	    switch (sp->read_spifr->status->file_compress){
	      case SP_wc_shorten:
		if (sp_verbose > 15) printf("Proc %s: Executing Shorten Decompression\n",proc_name);
		if (shorten_uncompress(fob_in, fob_out) < 0){
		    fob_destroy(fob_in);
		    fob_destroy(fob_out);
		    return_err(proc_name,202,0,"Shorten Decompression Failed");
		}
		break;
	      case SP_wc_wavpack:
		if (sp_verbose > 15) printf("Proc %s: Executing Wavpack Decompression\n",proc_name);
		if (wavpack_unpack(fob_in, fob_out) < 0){
		    fob_destroy(fob_in);
		    fob_destroy(fob_out);
		    return_err(proc_name,203,0,"Wavpack Decompression Failed");
		}
		break;
	      case SP_wc_shortpack:
		if (sp_verbose > 15) printf("Proc %s: Executing Shortpack Decompression\n",proc_name);
		if (shortpack_uncompress(fob_in, fob_out, sp->read_spifr->status->file_header) < 0){
		    fob_destroy(fob_in);
		    fob_destroy(fob_out);
		    return_err(proc_name,203,0,"Shortpack Decompression Failed");
		}
		break;
	      default:
		return_err(proc_name,205,0,"Unable to decompress the requested format\n");
	    }
	    fob_rewind(fob_out);
            /* If a temporary file is being used, close the waveform file BEFORE setting it to FPNULL.  ***PSI***  24-Sep-1993 */

            if (sp->read_spifr->status->is_temp_file) {
                if (sp_verbose > 15)
		    printf("Proc %s: Closing waveform file \"%s\" (%d)\n", proc_name, sp->read_spifr->status->external_filename,
			   fileno(sp->read_spifr->waveform->sp_fp));

                if (fclose(sp->read_spifr->waveform->sp_fp))
		    return_err(proc_name, 403, 0, rsprintf("Unable to close waveform file \"%s\" (%d)",
							   sp->read_spifr->status->external_filename,
							   fileno(sp->read_spifr->waveform->sp_fp)));
                sp->read_spifr->waveform->sp_fp = FPNULL;
	    }

            /**** End SRI bugfix. ****/

	    if (! decomp_into_memory) { /* Close the original file pointer to the SPHERE file */
		fclose(fob_in->fp);
	    }

	    fob_destroy(fob_in);	    
	    
	    sp->read_spifr->waveform->sp_fob = fob_out;
	}
	else {
	    /* The following code assumes that no pre-bufferring is needed */

	    if ((sp->read_spifr->waveform->sp_fob = fob_create(sp->read_spifr->waveform->sp_fp)) == FOBPNULL)
		return_err(proc_name,300,0,"Unable to allocate a FOB (File or Buffer) structure");
	    sp->read_spifr->waveform->sp_fp = FPNULL;
	}

	/****************************************************/
	/**            INVARIANT ASSERTION:                **/
	/** The data is now in it's natural (decomp) form  **/
	/****************************************************/

	/************ Set up the file conversions ***********/
	/* Set up byte format the conversions */
	if (sp->read_spifr->status->user_sbf != sp->read_spifr->status->file_sbf) 
	    if (((sp->read_spifr->status->user_sbf == SP_sbf_01) &&
		 (sp->read_spifr->status->file_sbf == SP_sbf_10)) ||
		((sp->read_spifr->status->user_sbf == SP_sbf_10) &&
		 (sp->read_spifr->status->file_sbf == SP_sbf_01)))
		fob_read_byte_swap(sp->read_spifr->waveform->sp_fob);

	if (sp->read_spifr->status->user_encoding != sp->read_spifr->status->file_encoding)
	    return_err(proc_name,400,0,"Unable to convert sample types ... for now\n");

	/* pre-verify the waveform data */
	if (sp->read_spifr->status->extra_checksum_verify){
	    /* if the samples are in memory, compute the checksum from there. */
	    /* if not, read in the file, a block at a time and compute the    */
	    /* checksum.                                                      */
	    switch (sp->read_spifr->status->file_encoding){
	      case SP_se_pcm2:
		if (fob_short_checksum(sp->read_spifr->waveform->sp_fob, &checksum,
				       sp->read_spifr->status->file_sbf != sp->read_spifr->status->natural_sbf,
				       sp_compute_short_checksum,
				       sp_add_checksum) < 0){
		    return_err(proc_name,501,0,"Unable to Pre-Verify Checksum");
		}
		if (checksum != sp->read_spifr->status->file_checksum){
		    sp->read_spifr->waveform->failed_checksum = TRUE;
		    return_err(proc_name,1001,0,"Pre-Read Checksum Test Failed");
		}
		break;
	      default:
		break;
	    }
	}
    }

    if (sp_verbose > 15) printf("Proc %s: current file position %d\n",proc_name,
				fob_ftell(sp->read_spifr->waveform->sp_fob));

    /* READ THE DATA INTO THE BUFFER */
    ret = fob_fread(buffer, sample_size, num_sample, sp->read_spifr->waveform->sp_fob);
    if (ret < 0)
	return_err(proc_name,105,0,"Unable to read data");

    /* Perform the checksum computation */
    switch (sp->read_spifr->status->user_encoding){
      case SP_se_pcm2:
	if (sp->read_spifr->status->natural_sbf == sp->read_spifr->status->user_sbf)
	    checksum = sp_compute_short_checksum(buffer, ret, FALSE);
	else
	    checksum = sp_compute_short_checksum(buffer, ret, TRUE);
	
	sp->read_spifr->waveform->checksum = 
	    sp_add_checksum(sp->read_spifr->waveform->checksum,checksum);
	sp->read_spifr->waveform->samples_read += ret;

	if (sp_eof(sp))
	    if (sp->read_spifr->waveform->samples_read != sp->read_spifr->status->user_sample_count){
		sp->read_spifr->waveform->read_premature_eof = TRUE;
		return_err(proc_name,500,0,"Premature End-of_File");
	    }
	if (sp->read_spifr->waveform->samples_read == sp->read_spifr->status->user_sample_count){
	    if ((! sp->read_spifr->status->ignore_checksum) &&
		(sp->read_spifr->waveform->checksum != sp->read_spifr->status->file_checksum)){
		sp->read_spifr->waveform->failed_checksum = TRUE;	    
		return_err(proc_name,1000,0,"Checksum Test Failed");
	    }

	}
	break;
      default:
	break;
    }

    if (sp_verbose > 11) printf("Proc %s: Requested %d, %d byte samples, got %d\n",proc_name,num_sample,sample_size,ret);
    return_success(proc_name,0,ret,"ok");


}
