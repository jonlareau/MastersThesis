#include <stdio.h>
#include <stdlib.h>
#include <sp/sphere.h>
#include <math.h>


int sp_h_get_field(sp_file, field, ftype, value)
SP_FILE *sp_file;
char *field;
void **value;
int ftype;
{
    char *proc_name="sp_h_get_field";
    int type, size;
    int n;
    int v;

    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if ( sp_file == SPNULL )
	return_err(proc_name,100,100,"Null SPFILE");
    if ( field == CNULL )	
	return_err(proc_name,102,102,"Null header field requested");
    if ( value == (void **)NULL )
	return_err(proc_name,103,103,"Null value pointer");
    
    if (sp_file->open_mode == SP_mode_read){
	return_child(proc_name,int,h_get_field(sp_file->read_spifr->header,field,ftype,value));
    }
    else  /* write or update */
	return_child(proc_name,int,h_get_field(sp_file->write_spifr->header,field,ftype,value));
}

int h_get_field(header, field, ftype, value)
struct header_t *header;
char *field;
void **value;
int ftype;
{
    char *proc_name="h_get_field";
    int type, size;
    int n;
    int v;

    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if ( header == HDRNULL )
	return_err(proc_name,101,101,"Null header");
    if ( field == CNULL )	
	return_err(proc_name,102,102,"Null header field requested");
    if ( value == (void **)NULL )
	return_err(proc_name,103,103,"Null value pointer");
    if ( ftype != T_STRING && ftype != T_REAL && ftype != T_INTEGER)
	return_err(proc_name,104,104,"Illegal field type");

    switch (ftype){ 
      case T_INTEGER:{
	  int *ivalue;
	  ivalue = (int *)value;
	  n = sp_get_field( header, field, &type, &size );
	  if ( n < 0 )
	      return_err(proc_name,104,104,"Non-Existing INTEGER field");
	  switch ( type ) {
	    case T_INTEGER:
	      n = sp_get_data( header, field, (char *) &v, &size );
	      if ( n < 0 )
		  return_err(proc_name,107,107,"Unable to get INTEGER Field");
	      *ivalue = v;
	      return_success(proc_name,0,0,"ok");
	    case T_STRING:
	      return_err(proc_name,108,108,"Illegal INTEGER access of a STRING Field");
	    case T_REAL:
	      return_err(proc_name,109,109,"Illegal INTEGER access of a REAL Field");
	  }
	  return_success(proc_name,0,0,"ok");
      }
      case T_STRING: {
	  char *buf;

	  n = sp_get_field( header, field, &type, &size );
	  if ( n < 0 )
	      return_err(proc_name,110,110,"Non-Existing STRING field");
	  switch ( type ) {
	    case T_INTEGER:
	      return_err(proc_name,111,111,"Illegal STRING access of an INTEGER Field");
	    case T_REAL:
	      return_err(proc_name,112,112,"Illegal STRING access of a REAL Field");
	    case T_STRING:
	      buf = (char *)mtrf_malloc(    size + 1 );
	      if ( buf == CNULL )
		  return_err(proc_name,113,113,"Unable to malloc char buffer");
	      n = sp_get_data( header, field, buf, &size );
	      buf[size] = (char )0;
	      if ( n < 0 ) {
		  mtrf_free( buf );
		  return_err(proc_name,114,114,"Unable to get STRING Field");
	      }
	      *value = buf ;
	      return_success(proc_name,0,0,"ok");
	  }
      }
      case T_REAL:{
	  double fv, *fvalue;
	  fvalue = (double *)value;
	  n = sp_get_field( header, field, &type, &size );
	  if ( n < 0 )
	      return_err(proc_name,115,115,"Non-Existing REAL field");
	  switch ( type ) {
	    case T_INTEGER:
	      return_err(proc_name,116,116,"Illegal REAL access of an INTEGER Field");
	    case T_REAL:
	      n = sp_get_data( header, field, (char *) &fv, &size );
	      if ( n < 0 )
		  return 0;
	      *fvalue = (double)fv;
	      return_success(proc_name,0,0,"ok");
	    case T_STRING:
	      return_err(proc_name,117,117,"Illegal INTEGER access of a STRING Field");
	  }
	  return_success(proc_name,0,0,"ok");
      }
      default:
	return_err(proc_name,120,120,"Unknown header field type");
    }
}

int sp_h_set_field(sp_file, field, ftype, value)
SP_FILE *sp_file;
char *field;
void *value;
int ftype;
{
    char *proc_name="sp_h_set_field";
    SPIFR *spifr;
    int type, size;
    int n;
    int v;
    int ret;
    enum SP_sample_byte_fmt new_sbf = SP_sbf_null;
    enum SP_waveform_comp new_compress = SP_wc_null;
    enum SP_sample_encoding new_encoding = SP_se_null;

    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if ( sp_file == SPNULL )
	return_err(proc_name,100,100,"Null SPFILE");
    if ( field == CNULL )	
	return_err(proc_name,102,102,"Null header field requested");
    if ( value == (char *)NULL )
	return_err(proc_name,103,103,"Null value pointer");
    if ( ftype != T_STRING && ftype != T_REAL && ftype != T_INTEGER)
	return_err(proc_name,104,104,"Illegal field type");

    /**********************************************************/
    /*  Pre-check the field types of standard fields          */
    
    if (strsame(field,SAMPLE_BF_FIELD)) {
	if (ftype != T_STRING)
	    return_err(proc_name,112,112,
		       rsprintf("Illegal field type for the '%s' field not T_STRING",field));
	if (parse_sample_byte_format((char *)value,&new_sbf) != 0)
	    return_err(proc_name,105,105,
		       rsprintf("Illegal value '%s' for '%s' field",value,field));
    }
    if (strsame(field,SAMPLE_N_BYTES_FIELD)){
	if (ftype != T_INTEGER)
	    return_err(proc_name,113,113,
		       rsprintf("Illegal field type for the '%s' field not T_INTEGER",field));
	if (*((long *)value) < 1)
	    return_err(proc_name,106,106,
		       rsprintf("Illegal value %d for '%s' field",*((long *)value),field));
    }
    if (strsame(field,SAMPLE_CODING_FIELD)){
	if (ftype != T_STRING)
	    return_err(proc_name,114,114,
		       rsprintf("Illegal field type for the '%s' field not T_STRING",field));
	if ((sp_file->open_mode == SP_mode_write) ||
	    (sp_file->open_mode == SP_mode_update))
	    spifr = sp_file->write_spifr;
	else
	    spifr = sp_file->read_spifr;
	if (parse_sample_coding((char *)value,spifr->status->user_sample_n_bytes,
				&new_encoding, &new_compress) != 0){
	    sp_print_return_status(stdout);
	    return_err(proc_name,107,107,
		       rsprintf("Illegal value '%s' for '%s' field",value,field));
	}
    }
    if (strsame(field,SAMPLE_COUNT_FIELD) || 
	strsame(field,CHANNEL_COUNT_FIELD) ||
	strsame(field,SAMPLE_RATE_FIELD) || 
	strsame(field,SAMPLE_CHECKSUM_FIELD)) {
	if (ftype != T_INTEGER)
	    return_err(proc_name,115,115,
		       rsprintf("Illegal field type for the '%s' field not T_INTEGER",field));
    }

   
    if ((sp_file->open_mode == SP_mode_write) ||
	(sp_file->open_mode == SP_mode_update)){
	
	spifr = sp_file->write_spifr;

	/* do some consitency checking on X fields to be able to catch */
	/* errors                                                      */
	if (new_sbf != SP_sbf_null) {
	    if (((new_sbf == SP_sbf_01) || (new_sbf == SP_sbf_10)) &&
		((spifr->status->user_sample_n_bytes == 2) ||
		 (spifr->status->user_sample_n_bytes == 0)))
		;
	    else if ((new_sbf == SP_sbf_1) &&
		     ((spifr->status->user_sample_n_bytes == 1) ||
		      (spifr->status->user_sample_n_bytes == 0)))
		;
	    else
		return_err(proc_name,200,200,
			   "Illegal sample_n_bytes field for a 2-byte sample_byte_format");
	}

	if (h_set_field(spifr->header,field,ftype,value) >= 100){
	    print_return_status(stdout);
	    return_err(proc_name,108,108,
		       rsprintf("Unable to set field '%s' in the user's header\n",field));
	}
	if (h_set_field(spifr->status->file_header,field,ftype,value) >= 100)
	    return_err(proc_name,109,109,
		       rsprintf("Unable to set field '%s' in the files's header\n",field));
	/* check for special fields which control the waveform definitions */
	
	if (strsame(field,"sample_n_bytes")) {
	    spifr->status->user_sample_n_bytes = (int)*((long *)value);
	    spifr->status->file_sample_n_bytes = (int)*((long *)value);
	    if (spifr->status->set_data_mode_occured_flag) 
		return_warn(proc_name,1,1,"Field 'sample_n_bytes' set after set_data_mode occured\n");	
	}
	if (strsame(field,"sample_byte_format")){
	    spifr->status->user_sbf = new_sbf;
	    spifr->status->file_sbf = new_sbf;
	    if (spifr->status->set_data_mode_occured_flag) 
		return_warn(proc_name,2,2,
			    "Field 'sample_byte_format' set after set_data_mode occured\n");
	}
	if (strsame(field,"sample_checksum")){
	    spifr->status->file_checksum = (int)*((long *)value);
	    if (spifr->status->set_data_mode_occured_flag) 
		return_warn(proc_name,2,2,
			    "Field 'sample_checksum' set after set_data_mode occured\n");
	}
	if (strsame(field,SAMPLE_CODING_FIELD)){
	    spifr->status->user_compress = spifr->status->file_compress = new_compress;
	    spifr->status->user_encoding = spifr->status->file_encoding = new_encoding;
	    if (spifr->status->set_data_mode_occured_flag) 
		return_warn(proc_name,3,3,"Field 'sample_coding' set after set_data_mode occured\n");
	}
	if (strsame(field,"sample_count")){
	    spifr->status->user_sample_count = *((long *)value);
	    spifr->status->file_sample_count = *((long *)value);
	}
	if (strsame(field,"channel_count")){
	    spifr->status->user_channel_count = *((long *)value);
	    spifr->status->file_channel_count = *((long *)value);
	}
	if (strsame(field,"sample_rate")){
	    spifr->status->user_sample_rate = *((long *)value);
	    spifr->status->file_sample_rate = *((long *)value);
	}
	if (spifr->status->write_occured_flag)
	    return_warn(proc_name,4,4,"Call executed after WRITE occured\n");
    }
    else{
	spifr = sp_file->read_spifr;
	if ((strsame(field,"sample_n_bytes")) ||
	    (strsame(field,"sample_byte_format")) ||
	    (strsame(field,SAMPLE_CODING_FIELD)))
	    return_err(proc_name,111,111,
		       rsprintf("Field '%s' should be set using the 'set_data_mode' function",
				field));
	if ((strsame(field,"sample_count")) ||
	    (strsame(field,"channel_count")) ||
	    (strsame(field,"sample_rate")))
	    return_err(proc_name,112,112,
		       rsprintf("Field '%s' should not be set on a file opened for reading",
				field));
	    
	if (h_set_field(spifr->header,field,ftype,value) >= 100)
	    return_err(proc_name,110,110,
		       rsprintf("Unable to set field '%s' in the SPFILE's header\n",field));
	if (spifr->status->read_occured_flag)
	    return_warn(proc_name,5,5,"Call executed after READ occured\n");
    }
    return_success(proc_name,0,0,"ok");
}

int h_set_field(header, field, ftype, value)
struct header_t *header;
char *field;
void *value;
int ftype;
{
    char *proc_name="h_set_field";
    int type, size;
    int n;
    int v;
    
   
    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if (sp_verbose > 30) { printf("Proc %s: before set\n",proc_name); sp_print_lines(header,stdout); }
    if ( header == HDRNULL )
	return_err(proc_name,101,101,"Null header in SPFILE");
    if ( field == CNULL )	
	return_err(proc_name,102,102,"Null header field requested");
    if ( value == (char *)NULL )
	return_err(proc_name,103,103,"Null value pointer");
    if ( ftype != T_STRING && ftype != T_REAL && ftype != T_INTEGER)
	return_err(proc_name,104,104,"Illegal field type");

    n = sp_get_field( header, field, &type, &size );
    
    if ( n < 0 ){ /* add the field to the header */
	if (sp_add_field(header,field,ftype,value) < 0)
	    return_err(proc_name,105,105,"Unable to add field");
    }
    else 
        if (sp_change_field(header,field,ftype,value) < 0)
	    return_err(proc_name,106,106,"Unable to change existing field");
    if (sp_verbose > 30) { printf("Proc %s: After set\n",proc_name); sp_print_lines(header,stdout); }
    return_success(proc_name,0,0,"ok");
}

/*
 *  sp_h_delete_field
 *
 */
int sp_h_delete_field(sp_file, field)
SP_FILE *sp_file;
char *field;
{
    char *proc_name="sp_h_delete_field";
    int type, size;
    SPIFR *spifr;
    int n;
    int v;

    
    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if ( sp_file == SPNULL )
	return_err(proc_name,100,100,"Null SPFILE");
    if ( field == CNULL )	
	return_err(proc_name,102,102,"Null header field requested");

    if ((sp_file->open_mode == SP_mode_write) ||
	(sp_file->open_mode == SP_mode_update))
	spifr = sp_file->write_spifr;
    else
	spifr = sp_file->read_spifr;
       
    if ((sp_file->open_mode == SP_mode_write) || (sp_file->open_mode == SP_mode_update)) {
	if (h_delete_field(sp_file->write_spifr->header,field) < 0)
	    return_err(proc_name,105,105,
		       rsprintf("Deletion of field '%s' in the user's header failed",field));
	if (h_delete_field(sp_file->write_spifr->status->file_header,field) < 0)
	    return_err(proc_name,106,106,
		       rsprintf("Deletion of field '%s' in the hidden header failed",field));
    }
    if ((sp_file->open_mode == SP_mode_read) || (sp_file->open_mode == SP_mode_update)) {
	if (h_delete_field(sp_file->read_spifr->header,field) < 0)
	    return_err(proc_name,107,107,
		       rsprintf("Deletion of field '%s' in the user's header failed",field));
	if (h_delete_field(sp_file->read_spifr->status->file_header,field) < 0)
	    return_err(proc_name,108,108,
		       rsprintf("Deletion of field '%s' in the hidden header failed",field));
    }

    return_success(proc_name,0,0,"ok");
}

int h_delete_field(header, field)
struct header_t *header;
char *field;
{
    int type, size;
    int n;
    int v;
    char *proc_name="h_delete_field";
    
    if (sp_verbose > 10) printf("Proc %s:\n",proc_name);
    if ( header == HDRNULL )
	return_err(proc_name,101,101,"Null header in SPFILE");
    if ( field == CNULL )	
	return_err(proc_name,102,102,"Null header field requested");
    
    n = sp_get_field( header, field, &type, &size );
    if ( n < 0 )
	return_warn(proc_name,1,1,rsprintf("Header field '%s' does not exist",field));
    if (sp_delete_field(header,field) < 0)
	return_err(proc_name,104,104,rsprintf("Deletion of field '%s' failed",field));
    return_success(proc_name,0,0,"ok");	
}

