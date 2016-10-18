#include <stdio.h>
#include <sp/sphere.h>
#include <string.h>

struct header_t *global_header;
/*
 *
 *  set_data_mode
 *
 *
 */

int sp_set_data_mode(sp,mode)
SP_FILE *sp;
char *mode;
{
    char *mode_str, *mstr, *proc_name="sp_set_data_mode";
    int ulaw=FALSE, raw=FALSE, shorten=FALSE, shortpack=FALSE, wavpack=FALSE;
    int pcm_1=FALSE, pcm_2=FALSE;
    int sbf_01=FALSE, sbf_10=FALSE, sbf_1=FALSE, sbf_n=FALSE, sbf_orig=FALSE;
    int se_set=0, sbf_set=0;
    enum SP_sample_encoding     new_encoding, old_encoding = SP_se_null;
    enum SP_waveform_comp       new_compress, old_compress = SP_wc_null;
    enum SP_sample_byte_fmt     new_sbf     , old_sbf      = SP_sbf_null;

    if (sp_verbose > 10) fprintf(stdout,"Proc %s:\n",proc_name);
    if (sp_verbose > 15) fprintf(stdout,"Proc %s: mode string '%s'\n",proc_name,mode);

    if (sp == SPNULL) return_err(proc_name,100,100,"Null SPFILE");
    if (mode == CNULL) return_err(proc_name,101,101,"Null mode string");

    /* Set up the old file status.  */
    switch (sp->open_mode){
      case SP_mode_read:
	if (sp_verbose > 15) fprintf(stdout,"Proc %s: read filename %s\n",proc_name,
				    sp->read_spifr->status->external_filename);
	if (sp->read_spifr->status->read_occured_flag)
	    return_err(proc_name,1000,1000,"Call executed after READ occured\n");
	old_encoding = sp->read_spifr->status->file_encoding ;
	old_compress = sp->read_spifr->status->file_compress ;
	old_sbf      = sp->read_spifr->status->file_sbf ;

	/* set up the default operation modes */
	new_compress = SP_wc_none;
	new_sbf =  get_natural_sbf(sp->read_spifr->status->file_sample_n_bytes);
	new_encoding = old_encoding;

	break;
      case SP_mode_update:
      case SP_mode_write:
	if (sp_verbose > 15) fprintf(stdout,"Proc %s: write/update filename %s\n",proc_name,
				    sp->write_spifr->status->external_filename);
	if (sp->write_spifr->status->write_occured_flag)
	    return_err(proc_name,1001,1001,"Call executed after WRITE occured\n");
	old_encoding = sp->write_spifr->status->user_encoding ;
	old_compress = sp->write_spifr->status->user_compress ;
	old_sbf      = sp->write_spifr->status->user_sbf ;

	/* set up the default operation modes */
	new_compress = old_compress;
	new_sbf = sp->write_spifr->status->natural_sbf;
	new_encoding = old_encoding;

	break;
      default:
	return_err(proc_name,106,106,"Unknown file open mode in SPFILE structure");
    }

    if (sp_verbose > 15){
	fprintf(stdout,"Proc %s: Before mode parsing:    old_encoding %d   new_encoding %d\n",
		proc_name,old_encoding,new_encoding);
	fprintf(stdout,"Proc %s: Before mode parsing:    old_compress %d   new_compress %d\n",
		proc_name,old_compress,new_compress);
	fprintf(stdout,"Proc %s: Before mode parsing:    old_sbf      %d   new_sbf      %d\n",
		proc_name,old_sbf       ,new_sbf);
    }

    /* the tokenization method is data destructive , so */
    /* make a local copy */
    mode_str=mtrf_strdup(mode);

    /* begin parsing each token */
    mstr = strtok(mode_str,":");
    while (mstr != CNULL){
	if (sp_verbose > 15) fprintf(stdout,"Proc %s: token found = %s\n",proc_name,mstr);
	if ((strsame(mstr,"SE-PCM")) || (strsame(mstr,"SE-PCM-2"))) {
	    new_encoding = SP_se_pcm2;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-PCM-1")) {
	    new_encoding = SP_se_pcm1;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-RAW")) {
	    new_encoding = SP_se_raw;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-ULAW")) {
	    new_encoding = SP_se_ulaw;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-SHORTEN")){
	    new_encoding = old_encoding;
	    new_compress = SP_wc_shorten;
	    se_set++;
	}
	else if (strsame(mstr,"SE-WAVPACK")){
	    new_encoding = old_encoding;
	    new_compress = SP_wc_wavpack;
	    se_set++;
	}
	else if (strsame(mstr,"SE-SHORTPACK")){
	    new_encoding = old_encoding;
	    new_compress = SP_wc_shortpack;
	    se_set++;
	}
	else if (strsame(mstr,"SE-ORIG")){
	    new_encoding = old_encoding;
	    new_compress = old_compress;
	    se_set++;
	}
	else if (strsame(mstr,"SBF-01")) {
	    new_sbf = SP_sbf_01;
	    sbf_set++;
	}
	else if (strsame(mstr,"SBF-10")) {
	    new_sbf = SP_sbf_10;
	    sbf_set++;
	}

	else if (strsame(mstr,"SBF-1")) {
	    new_sbf = SP_sbf_1;
	    sbf_set++;
	}
	else if (strsame(mstr,"SBF-N")) {
	    if (old_sbf == SP_sbf_1)
		new_sbf = old_sbf;
	    else
		new_sbf = SP_sbf_N;
	    sbf_set++;
	}
	else if (strsame(mstr,"SBF-ORIG"))   {
	    new_sbf = old_sbf;
	    sbf_set++;
	}
	else {
	    mtrf_free(mode_str);
	    return_err(proc_name,102,102,rsprintf("Illegal token '%s' in mode string '%s'",
						  mstr,mode));
	}
	mstr = strtok(CNULL,",");
    }
    mtrf_free(mode_str);
    if (se_set > 1)
	return_err(proc_name,103,103,
		   rsprintf("Too many sample_encoding options used in mode string '%s'",mode));
    if (sbf_set > 1)
	return_err(proc_name,104,104,
		   rsprintf("Too many sample_byte_format options used in mode string '%s'",mode));

    if (sp_verbose > 15){
	if (sp_verbose > 16) sp_file_dump(sp,stdout);
	fprintf(stdout,"Proc %s:    old_encoding %d   new_encoding %d\n",
		proc_name,old_encoding,new_encoding);
	fprintf(stdout,"Proc %s:    old_compress %d   new_compress %d\n",
		proc_name,old_compress,new_compress);
	fprintf(stdout,"Proc %s:    old_sbf      %d   new_sbf      %d\n",
		proc_name,old_sbf       ,new_sbf);
    }

    /* Test the supported file conversions dependent on the file's open mode */
    switch (sp->open_mode){
      case SP_mode_read:
	switch (old_encoding) {
	  case SP_se_pcm2:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_pcm2))
		;
	    else
		return_err(proc_name,108,108,"Illegal read transformation from PCM2");
	    break;
	  case SP_se_pcm1:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_pcm2) || (new_encoding == SP_se_pcm1))
		;
	    else
		return_err(proc_name,109,109,"Illegal read transformation from PCM1");
	    break;
	  case SP_se_ulaw:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_pcm2) ||	(new_encoding == SP_se_pcm1))
		;
	    else
		return_err(proc_name,110,110,"Illegal read transformation from ULAW");
	    break;
	  case SP_se_raw:
	    if (new_encoding == SP_se_raw)
		;
	    else
		return_err(proc_name,111,111,"Illegal read transformation from RAW");
	    break;
	  default:
	    return_err(proc_name,112,112,"Internal function error for file opened for reading");
	}
	switch (old_compress){
	  case SP_wc_shortpack:
	  case SP_wc_wavpack:
	  case SP_wc_shorten:
	    if (new_compress != SP_wc_none)
		return_err(proc_name,113,113,rsprintf("Illegal read transformation to %s",
						      "compress a compressed file"));
	    if (new_encoding == SP_se_null)
		return_err(proc_name,114,114,"Internal function error for file opened for read");
	    break;
	  case SP_wc_none:
	    if (new_compress != SP_wc_none)
		return_err(proc_name,115,115,"Unable to read an uncompressed file as compressed");
	    break;
	  default:
	    return_err(proc_name,115,115,"Internal function error for file opened for read");
	}
	switch (old_sbf){
	  case SP_sbf_10:
	  case SP_sbf_01:
	  case SP_sbf_N:
	    if (new_sbf == SP_sbf_1)
		if ((new_encoding == SP_se_pcm1) || (new_encoding == SP_se_ulaw))
		    ;
		else
		    return_err(proc_name,116,116,"Unable to convert to a one-byte per sample format");
	    break;
	  case SP_sbf_1:
	    if (new_sbf != SP_sbf_1)
		if (new_encoding == SP_se_pcm2)
		    ;
		else
		    return_err(proc_name,117,117,"Unable to convert to a 2-byte per sample format");
	    break;
	  default:
	    return_err(proc_name,118,118,"Internal function error for file opened for read");
	    break;
	}
	break;
	/**********************************************************************/
	/**              Write and Update Mode Checking                       */
      case SP_mode_update:
      case SP_mode_write:
	switch (old_encoding) {
	  case SP_se_pcm2:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_pcm1) ||
		(new_encoding == SP_se_pcm2))
		;
	    else
		return_err(proc_name,119,119,"Illegal write transformation from PCM2");
	    break;
	  case SP_se_pcm1:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_pcm1) ||
		(new_encoding == SP_se_pcm2) ||	(new_encoding == SP_se_pcm1))
		;
	    else
		return_err(proc_name,120,120,"Illegal write transformation from PCM1");
	    break;
	  case SP_se_ulaw:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_pcm1) ||
	        (new_encoding == SP_se_pcm2))
		;
	    else
		return_err(proc_name,121,121,"Illegal write transformation from ULAW");
	    break;
	  case SP_se_raw:
	    if (new_encoding == SP_se_raw)
		;
	    else
		return_err(proc_name,122,122,"Illegal write transformation from RAW");
	    break;
	  default:
	    return_err(proc_name,123,123,"Internal function error for file opened for writing");
	}
	switch (old_compress){
	  case SP_wc_shortpack:
	  case SP_wc_wavpack:
	  case SP_wc_shorten:
	    if (new_compress != SP_wc_none)
		return_err(proc_name,124,124,rsprintf("Illegal write transformation to %s",
						      "compress a compressed file"));
	    break;
	  case SP_wc_none:
	    break;
	  default:
	    return_err(proc_name,125,125,"Internal function error for file opened for writing");
	}
	switch (old_sbf){
	  case SP_sbf_10:
	  case SP_sbf_01:
	  case SP_sbf_N:
	    if (new_sbf == SP_sbf_1)
		if ((new_encoding == SP_se_pcm1) || (new_encoding == SP_se_ulaw))
		    ;
		else
		    return_err(proc_name,126,126,"Unable to convert to a one-byte per sample format");
	    break;
	  case SP_sbf_1:
	    if (new_sbf != SP_sbf_1)
		if (new_encoding == SP_se_pcm2)
		    ;
		else
		    return_err(proc_name,127,127,"Unable to convert to a 2-byte per sample format");
	    break;
	  default:
	    return_err(proc_name,128,128,"Internal function error for file opened for read");
	}
	break;
      default:
	return_err(proc_name,129,129,"Unknown file open mode in SPFILE structure");
    }

    /* convert the natural byte order to the proper value */
    if ((new_encoding == SP_se_pcm2) && (new_sbf == SP_sbf_N))
	if ((sp->open_mode == SP_mode_write) || (sp->open_mode == SP_mode_update))
	    new_sbf = sp->write_spifr->status->natural_sbf;
	else if (sp->open_mode == SP_mode_read)
	    new_sbf = sp->read_spifr->status->natural_sbf;   
    else if ((new_encoding != SP_se_pcm2) && (new_sbf == SP_sbf_N))
	new_sbf = SP_sbf_1;

    /* now update the hidden header: file_header for write, user_header for read */
    /* and update the status structure variables                                 */
    {   struct header_t *hdr_to_modify;
	long lint;
	char new_sample_coding[100];
       
	if (sp->open_mode == SP_mode_read){
	    if (sp_verbose > 15) fprintf(stdout,"Proc %s: modifying the user's header \n",proc_name);
	    hdr_to_modify = sp->read_spifr->header;
	}
	else {
	    if (sp_verbose > 15) fprintf(stdout,"Proc %s: modifying the file's header \n",proc_name);
	    hdr_to_modify = sp->write_spifr->status->file_header;
	}

	global_header = sp->read_spifr->status->file_header;
	/* set the sample_n_bytes field */
	if (new_encoding == SP_se_pcm2){
	    lint = 2;
	    if (h_set_field(hdr_to_modify, SAMPLE_N_BYTES_FIELD, T_INTEGER, &lint))
		return_err(proc_name,150,150,"Unable to update sample_n_bytes field\n");
	} else {
	    lint = 1;
	    if (h_set_field(hdr_to_modify, SAMPLE_N_BYTES_FIELD, T_INTEGER, &lint))
		return_err(proc_name,151,151,"Unable to update sample_n_bytes field\n");
	}

	/* set the sample byte format */
	switch (new_sbf) {
	  case SP_sbf_01:
	    if (sp_verbose > 15) fprintf(stdout,"Proc %s: new sample_byte_format 01\n",proc_name);
	    if (h_set_field(hdr_to_modify, "sample_byte_format", T_STRING, "01"))
		return_err(proc_name,153,153,"Unable to update sample_byte_format field to '01'\n");
	    break;
	  case SP_sbf_10:
	    if (sp_verbose > 15) fprintf(stdout,"Proc %s: new sample_byte_format 10\n",proc_name);
	    if (h_set_field(hdr_to_modify, "sample_byte_format", T_STRING, "10"))
		return_err(proc_name,154,155,"Unable to update sample_byte_format field to '10'\n");
	    break;
	  case SP_sbf_1:
	    if (sp_verbose > 15) fprintf(stdout,"Proc %s: new sample_byte_format 1\n",proc_name);
	    if (h_set_field(hdr_to_modify, "sample_byte_format", T_STRING, "1"))
		return_err(proc_name,156,156,"Unable to update sample_byte_format field to '1'\n");
	    break;
	  default:
	    return_err(proc_name,157,157,"Internal function error");
	}
	    
	/* set the sample encoding field */
	*new_sample_coding = '\0';
	switch (new_encoding){
	  case SP_se_pcm1:
	  case SP_se_pcm2: strcat(new_sample_coding,"pcm"); break;
	  case SP_se_ulaw: strcat(new_sample_coding,"ulaw"); break;
	  case SP_se_raw: strcat(new_sample_coding,"raw"); break;
	  default: return_err(proc_name,158,158,"Internal Function Error");
	}
	switch (new_compress){
	  case SP_wc_shorten:
	    strcat(new_sample_coding,rsprintf(",embedded-shorten-v%1d.%s",FORMAT_VERSION,
					      BUGFIX_RELEASE));
/*	    strcat(new_sample_coding,rsprintf(",embedded-shorten-v%1d.%s",FORMAT_VERSION,
					      "1"));*/
	    break;
	  case SP_wc_wavpack:
	    strcat(new_sample_coding,rsprintf(",embedded-%s", WAVPACK_MAGIC));
	    break;
	  case SP_wc_shortpack:
	    strcat(new_sample_coding,",embedded-shortpack-vIDONTKNOW");
	    break;
	}
	if (h_set_field(hdr_to_modify, SAMPLE_CODING_FIELD, T_STRING, new_sample_coding))
	    return_err(proc_name,160,160,rsprintf("Unable to update sample_coding field to '%s'\n",
						  new_sample_coding));
	if (sp_verbose > 15) fprintf(stdout,"Proc %s: new sample coding %s\n",proc_name,new_sample_coding);

	/* setup the new values */
	if (sp->open_mode == SP_mode_read){
	    sp->read_spifr->status->user_encoding = new_encoding ;
	    sp->read_spifr->status->user_compress = new_compress ;
	    sp->read_spifr->status->user_sbf      = new_sbf      ;
	} else {
	    sp->write_spifr->status->file_encoding = new_encoding ;
	    sp->write_spifr->status->file_compress = new_compress ;
	    sp->write_spifr->status->file_sbf      = new_sbf      ;
	}
    }

    if (sp_verbose > 11) fprintf(stdout,"Proc %s: Exit\n",proc_name);

/* make a warning if the default mode is used */
    
    if (sp->open_mode == SP_mode_read)
	sp->read_spifr->status->set_data_mode_occured_flag = TRUE;
    else
	sp->read_spifr->status->set_data_mode_occured_flag = TRUE;

    return_success(proc_name,0,0,"ok");

}

