#include <stdio.h>
#include <sp/sphere.h>

/*
 *  memory allocation/deallocation routines for the sphere_t 
 *  structure.
 *
 *  Returns:  a SP_FILE pointer upon success
 *            SPNULL upon failure
 */
SP_FILE *sp_alloc_and_init_sphere_t()
{
    SP_FILE *tsp;
    SPIFR *alloc_SPIFR();

    if (sp_verbose > 10) printf("Proc sp_alloc_and_init_sphere_t:\n");
    /* alloc sphere_t structure */
    if ((tsp=(SP_FILE *)mtrf_malloc(sizeof(SP_FILE))) == SPNULL)
	return(SPNULL);

    tsp->open_mode = SP_mode_null;

    if ((tsp->read_spifr = alloc_SPIFR()) == SPIFRNULL){
	mtrf_free(tsp);
	return(SPNULL);
    }
    if ((tsp->write_spifr = alloc_SPIFR()) == SPIFRNULL){
	free_SPIFR(tsp->read_spifr);
	mtrf_free(tsp);
	return(SPNULL);
    }
    return(tsp);
}


SPIFR *alloc_SPIFR()
{
    SPIFR *tspifr;

    if ((tspifr=(SPIFR *)mtrf_malloc(sizeof(SPIFR))) == SPIFRNULL)
	return(SPIFRNULL);
    
    /* init the sphere_t structure */
    if ((tspifr->status=(struct spfile_status_t *)
	                mtrf_malloc(sizeof(struct spfile_status_t))) == 
        (struct spfile_status_t *)0){
	mtrf_free(tspifr);
	return(SPIFRNULL);
    }
    if ((tspifr->waveform=(struct waveform_t *)
	                mtrf_malloc(sizeof(struct waveform_t))) == 
	(struct waveform_t *)0){
	mtrf_free(tspifr->status);
	mtrf_free(tspifr);
	return(SPIFRNULL);
    }
    tspifr->header = HDRNULL;

    /* init the waveform structure */
    tspifr->waveform->sp_fp = (FILE *)0;
    tspifr->waveform->sp_fob = FOBPNULL;
    tspifr->waveform->samples_read = 0;
    tspifr->waveform->samples_written = 0;
    tspifr->waveform->checksum = 0;
    tspifr->waveform->header_data_size = -1;
    tspifr->waveform->read_premature_eof = FALSE;
    tspifr->waveform->failed_checksum = FALSE;

    /* init the status structure */
    tspifr->status->external_filename = CNULL;
    tspifr->status->file_header = HDRNULL;
    tspifr->status->extra_checksum_verify = FALSE;
    tspifr->status->is_disk_file = FALSE;
    tspifr->status->is_temp_file = FALSE;
    tspifr->status->temp_filename = CNULL;

    tspifr->status->user_channel_count = 0;
    tspifr->status->user_sample_count = 0;
    tspifr->status->user_sample_rate = 0; 
    tspifr->status->user_sample_n_bytes = 0;

    tspifr->status->file_channel_count = 0;
    tspifr->status->file_sample_count = 0;
    tspifr->status->file_sample_rate = 0; 
    tspifr->status->file_sample_n_bytes = 0;

    tspifr->status->file_checksum = (-1);
    tspifr->status->ignore_checksum = FALSE;

    tspifr->status->file_header = HDRNULL;
    tspifr->status->user_encoding = tspifr->status->file_encoding = SP_se_null;
    tspifr->status->user_compress = tspifr->status->file_compress = SP_wc_null;
    tspifr->status->user_sbf = tspifr->status->file_sbf = SP_sbf_null;
    tspifr->status->natural_sbf = get_natural_sbf(2);
    tspifr->status->write_occured_flag = FALSE;
    tspifr->status->read_occured_flag = FALSE;
    tspifr->status->field_set_occured_flag = FALSE;
    tspifr->status->set_data_mode_occured_flag = FALSE;

    return(tspifr);    
}


int free_SPIFR(spifr)
SPIFR *spifr;
{
    int fail=0;
    FILE *fp=FPNULL;

    /* free the waveform structure */
    if (spifr->waveform->sp_fob != FOBPNULL){
	if (spifr->waveform->sp_fob->fp != FPNULL){
	    fp = spifr->waveform->sp_fob->fp;
	    fflush(fp);
	    if ((fp != stdin) && (fp != stdout)) fclose(fp);
	}
	fob_destroy(spifr->waveform->sp_fob);
    }

    if ((fp != FPNULL) && (fp == spifr->waveform->sp_fp))
	;
    else
	if (spifr->waveform->sp_fp != FPNULL){
	    fp = spifr->waveform->sp_fp;
	    fflush(fp);
	    if ((fp != stdin) && (fp != stdout)) fclose(fp);
	}

    if (spifr->status->is_temp_file && (spifr->status->temp_filename != CNULL))
	unlink(spifr->status->temp_filename);

    /* free the status structure */
    if (spifr->status->external_filename != CNULL)
	mtrf_free(spifr->status->external_filename);
    if (spifr->status->file_header != HDRNULL)
	sp_close_header(spifr->status->file_header);
    if (spifr->status->temp_filename != CNULL)
	mtrf_free(spifr->status->temp_filename);


    /* the sphere_t data */
    if (spifr->header != HDRNULL)
	sp_close_header(spifr->header);
    if (spifr->waveform != (struct waveform_t *)0)
	mtrf_free(spifr->waveform);
    if (spifr->status != (struct spfile_status_t *)0)
	mtrf_free(spifr->status);
    mtrf_free(spifr);
    return(0);
}


/*
 * Free all memory associated with a SP_FILE
 * Returns: -1 on failure
 *           0 on success
 */
int free_sphere_t(sp)
SP_FILE *sp;
{
    if (sp->read_spifr != SPIFRNULL)
        free_SPIFR(sp->read_spifr);
    if (sp->write_spifr != SPIFRNULL)
        free_SPIFR(sp->write_spifr);
    mtrf_free(sp);	    
    return(0);
}
