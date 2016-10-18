#include <stdio.h>
#include <sp/sphere.h>
#include <util/hsgetopt.h>

#define EXAMPLE1_10_BASE           "ex1_10.wav"
#define EXAMPLE1_01_BASE           "ex1_01.wav"
#define EXAMPLE1_10_WAVPACK_BASE   "ex1_10wp.wav"
#define EXAMPLE1_10_SHORTEN_BASE   "ex1_10st.wav"
#define EXAMPLE1_10_SHORTPACK_BASE "ex1_10sh.wav"
#define EXAMPLE1_01_WAVPACK_BASE   "ex1_01wp.wav"
#define EXAMPLE1_01_SHORTEN_BASE   "ex1_01st.wav"
#define EXAMPLE1_01_SHORTPACK_BASE "ex1_01sh.wav"

#define EXAMPLE2_10_BASE           "ex2_10.wav"
#define EXAMPLE2_10_BASE           "ex2_10.wav"
#define EXAMPLE2_01_BASE           "ex2_01.wav"
#define EXAMPLE2_10_WAVPACK_BASE   "ex2_10wp.wav"
#define EXAMPLE2_10_SHORTEN_BASE   "ex2_10st.wav"
#define EXAMPLE2_01_WAVPACK_BASE   "ex2_01wp.wav"
#define EXAMPLE2_01_SHORTEN_BASE   "ex2_01st.wav"


char EXAMPLE1_10[256], EXAMPLE1_01[256], EXAMPLE1_10_WAVPACK[256];
char EXAMPLE1_10_SHORTEN[256], EXAMPLE1_01_WAVPACK[256], EXAMPLE1_01_SHORTEN[256];
char EXAMPLE1_10_SHORTPACK[256], EXAMPLE1_01_SHORTPACK[256];
char EXAMPLE2_10[256], EXAMPLE2_01[256], EXAMPLE2_10_WAVPACK[256];
char EXAMPLE2_10_SHORTEN[256], EXAMPLE2_01_WAVPACK[256], EXAMPLE2_01_SHORTEN[256];

#define COPY "cp"

main(argc,argv)
int argc;
char **argv;
{
    char *sphere_lib_dir=".";
    int c;


    while (( c=hs_getopt( argc, argv, "d:hvmMe" )) != -1 )
	switch ( c ) {
	  case 'd':
	    sphere_lib_dir = hs_optarg;
	    break;
	  case 'v':
	    sp_verbose ++;
	    break;
	  case 'm':
	    mtrf_set_dealloc(0);
	    mtrf_set_verbose(1);
	    break;
	  case 'M':
	    mtrf_set_dealloc(1);
	    mtrf_set_verbose(1);
	    break;
	  case 'e':	    
	    set_error_util_debug(1);
	    break;
	  case 'h':
	  default:
	    printf("Usage: tsphere -[vmMeh] -d sphere_lib_dir\n");
	    printf("Illegal argument: %c\n",c);
	    exit(100);
	}

    if (hs_optind != argc){
	printf("Usage: tsphere -[v|m] -d sphere_lib_dir\n");
	exit(100);
    }
    hs_resetopt();
	
    sprintf(EXAMPLE1_10, "%s/%s", sphere_lib_dir,EXAMPLE1_10_BASE);
    sprintf(EXAMPLE1_01, "%s/%s", sphere_lib_dir,EXAMPLE1_01_BASE);
    sprintf(EXAMPLE1_10_WAVPACK, "%s/%s", sphere_lib_dir,EXAMPLE1_10_WAVPACK_BASE);
    sprintf(EXAMPLE1_10_SHORTEN, "%s/%s", sphere_lib_dir,EXAMPLE1_10_SHORTEN_BASE);
    sprintf(EXAMPLE1_10_SHORTPACK, "%s/%s", sphere_lib_dir,EXAMPLE1_10_SHORTPACK_BASE);
    sprintf(EXAMPLE1_01_WAVPACK, "%s/%s", sphere_lib_dir,EXAMPLE1_01_WAVPACK_BASE);
    sprintf(EXAMPLE1_01_SHORTEN, "%s/%s", sphere_lib_dir,EXAMPLE1_01_SHORTEN_BASE);
    sprintf(EXAMPLE1_01_SHORTPACK, "%s/%s", sphere_lib_dir,EXAMPLE1_01_SHORTPACK_BASE);

    sprintf(EXAMPLE2_10, "%s/%s", sphere_lib_dir,EXAMPLE2_10_BASE);
    sprintf(EXAMPLE2_10_WAVPACK, "%s/%s", sphere_lib_dir,EXAMPLE2_10_WAVPACK_BASE);
    sprintf(EXAMPLE2_10_SHORTEN, "%s/%s", sphere_lib_dir,EXAMPLE2_10_SHORTEN_BASE);
    sprintf(EXAMPLE2_01, "%s/%s", sphere_lib_dir,EXAMPLE2_01_BASE);
    sprintf(EXAMPLE2_01_WAVPACK, "%s/%s", sphere_lib_dir,EXAMPLE2_01_WAVPACK_BASE);
    sprintf(EXAMPLE2_01_SHORTEN, "%s/%s", sphere_lib_dir,EXAMPLE2_01_SHORTEN_BASE);

    header_test();  
    checksum_pre_post_verification();
    write_required_field_test();
    write_check_adding_fields_test();
    update_test();
    large_file_test();  

    printf("*******  ALL TESTS SUCCESSFULLY COMPLETED *******\n");

    exit(0);
}

large_file_test()

{
    char *large_file_name = "large.wav";
    char *large2_file_name = "large2.wav";
    
    printf("-- Large File Handling:\n");
    printf("---- Building CONTROL data file, %s\n",large2_file_name);
    make_test_file(large2_file_name,160000,"");

    do_large_file_conversion(large_file_name, large2_file_name, "SE-PCM:SBF-01");
    do_large_file_conversion(large_file_name, large2_file_name, "SE-SHORTEN:SBF-10");
    do_large_file_conversion(large_file_name, large2_file_name, "SE-WAVPACK:SBF-01");

    unlink(large2_file_name);
    printf("\n");
}

do_large_file_conversion(modified, control, mode)
char *modified, *control, *mode;
{
    fprintf(stdout,"------ Building Test file '%s' converted by '%s', using control file %s\n",modified,mode,control);

    make_test_file(modified,160000,mode);
    if (diff_waveforms(modified,control) != 0){
	fprintf(stdout,"Successful update failed waveform verification\n");
	exit(-1);
    }
    unlink(modified);
}


make_test_file(name,size,conversion)
char *name, *conversion;
int size;
{
    SP_FILE *spp;
    short *wavbuf;
    long lint;
    long buf_nsamp = 16000;
    long nbyte = 2;
    long srate = 16000;
    int stow, i;
    int samps_written=0, status;

    if ((wavbuf=(short *)mtrf_malloc(sizeof(short)*buf_nsamp)) == (short *)0){
	fprintf(stdout,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }

    for (i=0; i<buf_nsamp; i++)
	wavbuf[i] = 10 * ( i % (16 + (i / 100)));

    spp = sp_open(name, "w");
    if (spp == NULL) {
	fprintf(stdout,"Couldn't open NIST waveform file: %s\n", name);
	sp_print_return_status(stdout);
	exit(-1);
    }

    lint = size;
    sp_h_set_field(spp, "sample_count", T_INTEGER, (void *) &lint);
    sp_h_set_field(spp, "sample_rate", T_INTEGER, (void *) &srate);
    sp_h_set_field(spp, "sample_n_bytes", T_INTEGER, (void *) &nbyte);
    sp_h_set_field(spp, "sample_byte_format", T_STRING, (void *)"10");
    sp_h_set_field(spp, "sample_coding", T_STRING, (void *)"pcm");
    lint = 1; sp_h_set_field(spp, "channel_count", T_INTEGER, (void *)&lint);
    if (! strsame(conversion,"") )
	sp_set_data_mode(spp,conversion);

    while (samps_written < size){
	stow = (samps_written + buf_nsamp) < size ? buf_nsamp : size - samps_written;
	status = sp_write_data((void *)wavbuf, sizeof(short), stow, spp);
	if (status != stow){
	    fprintf(stdout,"Couldn't write NIST waveform file: %s\n", name);
	    sp_print_return_status(stdout);
	    status = sp_error(spp);
	    sp_print_return_status(stdout);
	    sp_close(spp);
	    (void) mtrf_free(wavbuf);
	    exit(-1);
	}	
	samps_written += stow;
    }
    if (sp_close(spp) != 0) {
	printf("SP_close failed\n");
	sp_print_return_status(stdout);
	exit(-1);
    }
    (void) mtrf_free(wavbuf);
}

write_required_field_test()
{
    SP_FILE *spp;
    char *outfilename="outputz.wav";
    short *wavbuf;
    long lint;
    long buf_nsamp = 16000;
    long nbyte = 2;
    long srate = 16000;
    int stow, i;
    int samps_written=0, status;

    printf("-- Required Field Test:\n");

    if ((wavbuf=(short *)mtrf_malloc(sizeof(short)*buf_nsamp)) == (short *)0){
	fprintf(stdout,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }

    for (i=0; i<buf_nsamp; i++)
	wavbuf[i] = 10 * ( i % (16 + (i / 100)));

    spp = sp_open(outfilename, "wv");
    if (spp == NULL) {
	fprintf(stdout,"Couldn't open NIST waveform file: %s\n", outfilename);
	sp_print_return_status(stdout);
	exit(-1);
    }

    for (i=0; i<5; i++){
	switch (i) {
	  case 0: printf("---- Test %d: Missing sample_n_bytes\n",i+1); break;
	  case 1: printf("---- Test %d: Missing channel_count\n",i+1); break;
	  case 2: printf("---- Test %d: sample_encoding 'ulaw' and missing sample_rate\n",i+1); break;
	  case 3: printf("---- Test %d: sample_encoding 'pcm' and missing sample_rate\n",i+1); break;
	  case 4: printf("---- Test %d: All fields present\n",i+1); break;
	}

	lint = buf_nsamp; sp_h_set_field(spp, "sample_count", T_INTEGER, (void *) &lint); 

	if (i > 0) sp_h_set_field(spp, "sample_n_bytes", T_INTEGER, (void *) &nbyte);
	if (i > 1) { lint = 1;  sp_h_set_field(spp, "channel_count", T_INTEGER, (void *) &lint);}
	if (i > 2) { sp_h_set_field(spp, "sample_coding", T_STRING, (void *) "ulaw"); }
	if (i > 3) { sp_h_set_field(spp, "sample_coding", T_STRING, (void *) "pcm"); }
	if (i > 3) { lint = 16000;  sp_h_set_field(spp, "sample_rate", T_INTEGER, (void *) &lint); }

	status = sp_write_data((void *)wavbuf, sizeof(short), buf_nsamp, spp);
	if ((i < 4) && (status > 0)){
	    fprintf(stdout,"************* TEST FAILED ****************\n");
	    sp_print_return_status(stdout);
	}
	else if ((i == 4) && (status != buf_nsamp)) {
	    fprintf(stdout,"************* TEST FAILED ****************\n");
	    sp_print_return_status(stdout);
	}
    }
    if (sp_close(spp) != 0) {
	printf("SP_close failed\n");
	sp_print_return_status(stdout);
	exit(-1);
    }
    (void) mtrf_free(wavbuf);
    unlink(outfilename);
    printf("\n");
}


write_check_adding_fields(comp_file,conversion_str)
char *comp_file, *conversion_str;
{
    SP_FILE *spp;
    char *outfilename = "outputw.wav";
    short *wavbuf;
    int i, status;
    long nsamp = 16000;
    long nbyte = 2;
    long srate = 16000;
    SP_CHECKSUM expected_checksum=61520;
    int expected_sample_size=nsamp;
    long lchecksum=expected_checksum;
    long channels=1;

    printf("------ Data Mode string '%s'\n",conversion_str);
    printf("------   Compared to file '%s'\n",comp_file);

    if ((wavbuf=(short *)mtrf_malloc(sizeof(short)*nsamp)) == (short *)0){
	fprintf(stdout,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }
    for (i=0; i<16000; i++)
	wavbuf[i] = 10 * ( i % (16 + (i / 100)));

    for (i=0; i<4; i++){
	switch (i) {
	  case 0: printf("-------- Test %d: All fields present\n",i+1); break;
	  case 1: printf("-------- Test %d: Missing sample_count\n",i+1); break;
	  case 2: printf("-------- Test %d: Missing sample_checksum\n",i+1); break;
	  case 3: printf("-------- Test %d: Missing sample_count and sample_checksum\n",i+1); break;
	}
	spp = sp_open(outfilename, "wv");
	if (spp == NULL) {
	    fprintf(stdout,"Couldn't open NIST waveform file: %s", outfilename);
	    sp_print_return_status(stdout);
	    exit(-1);
	}

	/* FILL IN ONLY THE REQUIRED FIELDS */
	/*  i = 0  ->  All fields present */
	/*  i = 1  ->  missing sample_count */
	/*  i = 2  ->  missing checksum */
	/*  i = 3  ->  missing sample_count and checksum */
	if ((i == 0) || (i == 2))
	    sp_h_set_field(spp, "sample_count", T_INTEGER, (void *) &nsamp);
	if ((i == 0) || (i == 1))
	    sp_h_set_field(spp, "sample_checksum", T_INTEGER, (void *) &lchecksum);
	sp_h_set_field(spp, "sample_rate", T_INTEGER, (void *) &srate);
	sp_h_set_field(spp, "sample_n_bytes", T_INTEGER, (void *) &nbyte);
	sp_h_set_field(spp, "sample_byte_format", T_STRING, (void *)get_natural_byte_order());
	sp_h_set_field(spp, "sample_coding", T_STRING, (void *)"pcm");
	sp_h_set_field(spp, "channel_count", T_INTEGER, (void *)&channels);

	if (! strsame(conversion_str,"")){
	    if (sp_set_data_mode(spp,conversion_str) != 0){
		printf("Set data mode failed\n");
		sp_print_return_status(stdout);
		exit(-1);
	    }
	}

	status = sp_write_data((void *)wavbuf, sizeof(short), nsamp, spp);
	if (status != nsamp)
	    {
		fprintf(stdout,"Couldn't write NIST waveform file: %s\n", outfilename);
		sp_print_return_status(stdout);
		status = sp_error(spp);
		sp_print_return_status(stdout);
		sp_close(spp);
	    }	
	if (sp_close(spp) != 0) {
	    printf("SP_close failed\n");
	    sp_print_return_status(stdout);
	    exit(-1);
	}
	    

	if (diff_waveforms(outfilename,comp_file) != 0){
	    fprintf(stdout,"Successful write failed waveform verification\n");
	    exit(-1);
	}
	{ int chg, ins, del, fail=0;
	  if (diff_header(outfilename,comp_file,&chg,&ins,&del,0) != 0) {
	      fprintf(stdout,"Successful write failed header verification\n");
	      fail=1;
	  }
	  if ((fail==1) && (chg > 0 || ins > 0 || del > 0)) {
	      diff_header(outfilename,comp_file,&chg,&ins,&del,1);
	      exit(-1);
	  }
        }

	unlink(outfilename);
    }
    (void) mtrf_free(wavbuf);
}

write_check_adding_fields_test()
{
    printf("-- Write Adding Fields Check:\n");
    write_check_adding_fields(EXAMPLE2_10,"SBF-10");
    write_check_adding_fields(EXAMPLE2_01,"SBF-01");
    write_check_adding_fields(EXAMPLE2_10_SHORTEN,"SE-SHORTEN:SBF-10");
    write_check_adding_fields(EXAMPLE2_10_WAVPACK,"SE-WAVPACK:SBF-10");
    write_check_adding_fields(EXAMPLE2_10_SHORTEN,"SE-SHORTEN:SBF-10");
    write_check_adding_fields(EXAMPLE2_01,"SBF-01");
    write_check_adding_fields(EXAMPLE2_01_SHORTEN,"SE-SHORTEN:SBF-01");
    write_check_adding_fields(EXAMPLE2_01_WAVPACK,"SE-WAVPACK:SBF-01");
    write_check_adding_fields(EXAMPLE2_01_SHORTEN,"SE-SHORTEN:SBF-01");
    printf("\n");
}

update_test()
{
    printf("-- Update Tests\n");
    printf("---- Update of header fields\n");
    header_update(EXAMPLE1_10,0);
    header_update(EXAMPLE1_01,0);
    header_update(EXAMPLE1_10_WAVPACK,0);
    header_update(EXAMPLE1_10_SHORTEN,0);
    header_update(EXAMPLE1_01_WAVPACK,0);
    header_update(EXAMPLE1_01_SHORTEN,0);
    printf("---- Update of header fields, expanding the header\n");
    header_update(EXAMPLE1_10,1);
    header_update(EXAMPLE1_01,1);
    header_update(EXAMPLE1_10_WAVPACK,1);
    header_update(EXAMPLE1_10_SHORTEN,1);
    header_update(EXAMPLE1_01_WAVPACK,1);
    header_update(EXAMPLE1_01_SHORTEN,1);

    printf("---- Update the waveform format\n");
    waveform_update(EXAMPLE1_10,"SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_10,"SBF-01",EXAMPLE1_01);
    waveform_update(EXAMPLE1_10,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_10,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);
    waveform_update(EXAMPLE1_10,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_10,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);

    waveform_update(EXAMPLE1_01,"SBF-01",EXAMPLE1_01);
    waveform_update(EXAMPLE1_01,"SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_01,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_01,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);
    waveform_update(EXAMPLE1_01,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_01,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);

    waveform_update(EXAMPLE1_10_SHORTEN,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);
    waveform_update(EXAMPLE1_10_SHORTEN,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);
    waveform_update(EXAMPLE1_10_SHORTEN,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_10_SHORTEN,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_10_SHORTEN,"SE-PCM:SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_10_SHORTEN,"SE-PCM:SBF-01",EXAMPLE1_01);

    waveform_update(EXAMPLE1_10_WAVPACK,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_10_WAVPACK,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_10_WAVPACK,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);
    waveform_update(EXAMPLE1_10_WAVPACK,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);
    waveform_update(EXAMPLE1_10_WAVPACK,"SE-PCM:SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_10_WAVPACK,"SE-PCM:SBF-01",EXAMPLE1_01);


    waveform_update(EXAMPLE1_10_SHORTPACK,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);
    waveform_update(EXAMPLE1_10_SHORTPACK,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);
    waveform_update(EXAMPLE1_10_SHORTPACK,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_10_SHORTPACK,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_10_SHORTPACK,"SE-PCM:SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_10_SHORTPACK,"SE-PCM:SBF-01",EXAMPLE1_01);

    waveform_update(EXAMPLE1_01_SHORTEN,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);
    waveform_update(EXAMPLE1_01_SHORTEN,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);
    waveform_update(EXAMPLE1_01_SHORTEN,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_01_SHORTEN,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_01_SHORTEN,"SE-PCM:SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_01_SHORTEN,"SE-PCM:SBF-01",EXAMPLE1_01);

    waveform_update(EXAMPLE1_01_WAVPACK,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_01_WAVPACK,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_01_WAVPACK,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);
    waveform_update(EXAMPLE1_01_WAVPACK,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);
    waveform_update(EXAMPLE1_01_WAVPACK,"SE-PCM:SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_01_WAVPACK,"SE-PCM:SBF-01",EXAMPLE1_01);

    waveform_update(EXAMPLE1_01_SHORTPACK,"SE-SHORTEN:SBF-01",EXAMPLE1_01_SHORTEN);
    waveform_update(EXAMPLE1_01_SHORTPACK,"SE-SHORTEN:SBF-10",EXAMPLE1_10_SHORTEN);
    waveform_update(EXAMPLE1_01_SHORTPACK,"SE-WAVPACK:SBF-10",EXAMPLE1_10_WAVPACK);
    waveform_update(EXAMPLE1_01_SHORTPACK,"SE-WAVPACK:SBF-01",EXAMPLE1_01_WAVPACK);
    waveform_update(EXAMPLE1_01_SHORTPACK,"SE-PCM:SBF-10",EXAMPLE1_10);
    waveform_update(EXAMPLE1_01_SHORTPACK,"SE-PCM:SBF-01",EXAMPLE1_01);

    printf("\n");
}

header_update(file, expand)
char *file;
int expand;
{
    char *target="speaking_mode";
    char *change_target="database_id";
    char *insert_target="insert_field";
    SP_FILE *sp;
    int chg, ins, del;
    int added_field=0, num_to_add=30, i;

    printf("------ File %s\n",file);
        
    system(rsprintf("%s %s output.wav",COPY,file));
    if ((sp=sp_open("output.wav","u")) == 0){
	printf("Unable to open copy base file '%s' called '%s'\n",file,"output.wav");
	sp_print_return_status(stdout);
        exit(-1);
    }
    if (sp_h_delete_field(sp,target) != 0){
	printf("Can't delete field '%s' in file %s\n",target,"output.wav");
	sp_print_return_status(stdout);
    }
    if (sp_h_set_field(sp,change_target,T_STRING,"foobar") != 0){
	printf("Can't change field '%s' in file %s\n",change_target,"output.wav");
	sp_print_return_status(stdout);
    }
    if (sp_h_set_field(sp,insert_target,T_STRING,"foobar") != 0){
	printf("Can't insert field '%s' in file %s\n",insert_target,"output.wav");
	sp_print_return_status(stdout);
    }
    if (expand){
	for (i=0; i<num_to_add; i++) {
	    if (sp_h_set_field(sp,rsprintf("dummy_field_%d",i),T_STRING,"Dummy field string value") != 0){
		printf("Can't insert field '%s' in file %s\n",rsprintf("dummy_field_%d",i),"output.wav");
		sp_print_return_status(stdout);
	    }
	    added_field ++;
	}
    }
	

    if (sp_close(sp) != 0) {
	printf("      Failed to close\n");
	sp_print_return_status(stdout);	
	exit(-1);
    }

    if (diff_waveforms("output.wav",file) != 0){
	printf("Update passed, but waveforms of files '%s' and '%s' differ\n","output.wav",file);
	exit(-1);
    }	
    if (diff_data("output.wav",file) != 0){
	printf("WARNING: files '%s' and '%s' decompress to the same form,\n","output.wav",file);
	printf("         but are not identical on disk\n");
	exit(-1);
    }
    if (diff_header(file,"output.wav",&chg,&ins,&del,0) != 0){
	printf("Unable to compare headers of file '%s' and '%s'\n","output.wav",file);
	printf("         but are not identical on disk\n");
	exit(-1);
    }
    if (! expand){
	if ((del != 1) || (chg != 1) || (ins != 1)){
	    printf("   There should have been one field deleted, inserted and changed, but the actual status is\n");
	    printf("   Del: %d   Ins: %d   Chg: %d\n",del,ins,chg);
	    diff_header(file,"output.wav",&chg,&ins,&del,1);
	    exit(-1);
	}
    } else {
	if ((del != 1) || (chg != 1) || (ins != 1 + added_field)){
	    printf("   There should have been one field deleted and changed, and %d fields inserted\n",1+added_field);
	    printf("   but the actual status is\n");
	    printf("   Del: %d   Ins: %d   Chg: %d\n",del,ins,chg);
	    diff_header(file,"output.wav",&chg,&ins,&del,1);
	    exit(-1);
	}
    }
	



}

waveform_update(base_file,sdm_mode,compare_file)
char *base_file, *sdm_mode, *compare_file;
{
    SP_FILE *sp;

    printf("------ File: %-13s    converted by: %-18s    compared to file: %s\n",
	   base_file,sdm_mode,compare_file);
    system(rsprintf("%s %s output.wav",COPY,base_file));
    if ((sp=sp_open("output.wav","u")) == 0){
	printf("Unable to open copy base file '%s' called '%s'\n",base_file,"output.wav");
	sp_print_return_status(stdout);
        exit(-1);
    }
    if (sp_set_data_mode(sp,sdm_mode) != 0){
	printf("Set data mode failed\n");
	sp_print_return_status(stdout);
        exit(-1);
    }
    if (sp_close(sp) != 0) {
	printf("      Failed to close\n");
	sp_print_return_status(stdout);
	exit(-1);
    }

    if (diff_waveforms("output.wav",compare_file) != 0){
	printf("Update passed, but waveforms of files '%s' and '%s' differ\n","output.wav",compare_file);
	exit(-1);
    }
	
    if (diff_data("output.wav",compare_file) != 0){
	printf("WARNING: files '%s' and '%s' decompress to the same form,\n","output.wav",compare_file);
	printf("         but are not identical on disk\n");
	exit(-1);
    }
    unlink("output.wav");


}

checksum_pre_post_verification(){
    printf("-- Checksum verification tests\n");
    printf("---- Pre-Read Check\n");
    pre_read_check(EXAMPLE1_10);
    pre_read_check(EXAMPLE1_01);
    pre_read_check(EXAMPLE1_10_WAVPACK);
    pre_read_check(EXAMPLE1_10_SHORTEN);
    pre_read_check(EXAMPLE1_01_WAVPACK);
    pre_read_check(EXAMPLE1_01_SHORTEN);

    printf("---- Post-Write Check\n");
    post_write_check(EXAMPLE1_10);
    post_write_check(EXAMPLE1_01);
    post_write_check(EXAMPLE1_10_WAVPACK);
    post_write_check(EXAMPLE1_01_WAVPACK);
    post_write_check(EXAMPLE1_01_SHORTEN);
    post_write_check(EXAMPLE1_01_SHORTEN);

    printf("\n");
}

pre_read_check(file)
char *file;
{
    SP_FILE *sp, *out_sp;
    short buff[20];

    printf("------ File: %s\n",file);
    if ((sp = sp_open(file,"rv")) == SPNULL) {
	sp_print_return_status(stdout);
        fprintf(stdout,"Error: Unable to open file %s to reading\n",file);
        exit(-1);
    }
    if (sp_read_data(buff,2,10,sp) != 10){
	sp_print_return_status(stdout);
        fprintf(stdout,"Error: Unable to pre-verify checksum in file %s\n",file);
        exit(-1);
    }
    sp_close(sp);
}

post_write_check(file)
char *file;
{
    SP_FILE *sp, *out_sp;
    
    printf("------ File: %s\n",file);
    if ((sp = sp_open(file,"r")) == SPNULL) {
	sp_print_return_status(stdout);
        fprintf(stdout,"Error: Unable open input file %s\n",file);
        exit(-1);
    }

    sp_set_data_mode(sp,"SE-ORIG:SBF-ORIG");

    if ((out_sp = sp_open("output.wav","w")) == SPNULL) {
	sp_print_return_status(stdout);
        fprintf(stdout,"Error: Unable to open SPHERE file %s\n","output.wav");
        exit(-1);
    }

    if (copy_sp_header(sp,out_sp) != 0){
	sp_print_return_status(stdout);
	exit(-1);
    }
    if (sp->read_spifr->status->file_compress != sp->read_spifr->status->user_compress){
	switch (sp->read_spifr->status->file_compress){
	  case SP_wc_shortpack:
	    sp_set_data_mode(out_sp,"SE-SHORTPACK:SBF-ORIG");
	    break;
          case SP_wc_wavpack:
	    sp_set_data_mode(out_sp,"SE-WAVPACK:SBF-ORIG");
	    break;
          case SP_wc_shorten:
	    sp_set_data_mode(out_sp,"SE-SHORTEN:SBF-ORIG");
	    break;
	}
    }

    { short buff[2048];
      int ret;

      do {
          ret=sp_read_data(buff,2,1024,sp);
	  sp_write_data(buff,2,ret,out_sp);
      } while (ret > 0);
    }
/*    sp_file_dump(sp,stdout);
    sp_file_dump(out_sp,stdout);*/
    if (sp_close(sp) != 0) {
	fprintf(stdout,"    Closing of file '%s' opened for read failed\n",file);
	sp_print_return_status(stdout);
	exit(-1);
    }

    if (sp_close(out_sp) != 0) {
	fprintf(stdout,"    Closing of file '%s' opened for write failed\n",file);
	sp_print_return_status(stdout);
	if (sp_error(out_sp) == 100)
	    fprintf(stdout,"    POST-WRITE CHECKSUM VERIFICATION FAILED\n");
	sp_print_return_status(stdout);
	exit(-1);
    }
    if (diff_waveforms(file,"output.wav") != 0){
	fprintf(stdout,"    Vefication passed, but files were not reproduced identically\n");
	exit(-1);
    }
    unlink("output.wav");
}

int copy_sp_header(spin, spout)
SP_FILE *spin, *spout;
{

    char *proc_name="sp_dup";
    struct header_t *h;
    int i, n;
    long lint;
    double real;

    if (spin->open_mode == SP_mode_read)
	h = spin->read_spifr->header;
    else if (spin->open_mode == SP_mode_write)
	h = spin->write_spifr->header;
    else
	return_err(proc_name,100,100,"Unable to dup header opened for update");

    /* just loop through all the names, adding each field */
    for (i=0; i < h->fc ; i++){
	switch (h->fv[i]->type){
	  case T_STRING:
	    if (sp_h_set_field(spout,h->fv[i]->name,h->fv[i]->type,h->fv[i]->data) != 0)
		return_err(proc_name,200,200,rsprintf("Unable to copy STRING field '%s'",h->fv[i]->name));
	    break;
	  case T_INTEGER:
	    lint=atol(h->fv[i]->data);
	    if (sp_h_set_field(spout,h->fv[i]->name,h->fv[i]->type,&lint) != 0)
		return_err(proc_name,200,200,rsprintf("Unable to copy INTEGER field '%s'",h->fv[i]->name));
	    break;
	  case T_REAL:
	    real=atof(h->fv[i]->data);
	    if (sp_h_set_field(spout,h->fv[i]->name,h->fv[i]->type,&real) != 0)
		return_err(proc_name,200,200,rsprintf("Unable to copy REAL field '%s'",h->fv[i]->name));
	    break;
	}
    }

    if (sp_set_default_operations(spout) != 0)
	return_err(proc_name,300,300,"Unable to set default operations duplicated file");
    return_success(proc_name,0,0,"ok");
}

header_test()
{
    SP_FILE *sp;
    char filename[100];
    short *waveform;
    int wave_byte_size, total_samples, rtn;
    char *str;
    long i, lint;
    double real, f;

    printf("-- Write Mode header operations:\n");
    system("rm -f testing.wav");
    if ((sp=sp_open("testing.wav","w")) == SPNULL) {
	printf("   sp_open: Valid write open failed\n");
	sp_print_return_status(stdout);
	exit(-1);
    }

    printf("---- Testing the file header io:\n");
    printf("------ Field creation:\n");
    if (sp_h_set_field(SPNULL,"field1",T_STRING,"char string 1") == 0)
	printf("    sp_h_set_field: Null SPFILE pointer failed\n");
    if (sp_h_set_field(sp,CNULL,T_STRING,"char string 1") == 0)
	printf("    sp_h_set_field: Null field name failed\n");
    if (sp_h_set_field(sp,"field1",4930,"char string 1") == 0)
	printf("    sp_h_set_field: Invalid field type failed\n");
    if (sp_h_set_field(sp,"field1",T_STRING,CNULL) == 0)
	printf("    sp_h_set_field: Null value failed\n");

    if (sp_h_set_field(sp,"field1",T_STRING,"string value1") != 0){
	printf("    sp_h_set_field: valid STRING command failed\n");
	sp_print_return_status(stdout);
    }

    lint=1;
    if (sp_h_set_field(sp,"field2",T_INTEGER,&lint) != 0){
	printf("    sp_h_set_field: valid INTEGER command failed\n");
	sp_print_return_status(stdout);
	exit(-1);
    }
    real=2.0;
    if (sp_h_set_field(sp,"field3",T_REAL,&real) != 0){
	printf("    sp_h_set_field: valid REAL command failed\n");
	sp_print_return_status(stdout);
    }

    printf("------ Field access:\n");
    if (sp_h_get_field(sp,"field1",T_STRING,&str) != 0){
	printf("    sp_h_get_field: valid STRING command failed\n");
	sp_print_return_status(stdout);
    }
    lint=1;
    free(str);
    if (sp_h_get_field(sp,"field2",T_INTEGER,&lint) != 0){
	printf("    sp_h_get_field: valid INTEGER command failed\n");
	sp_print_return_status(stdout);
	exit(-1);
    }
    real=2.0;
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,&real)) != 0){
	printf("    sp_h_get_field: valid REAL command returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
	   
    printf("------ Illegal Field access:\n");
    if ((rtn=sp_h_get_field(SPNULL,"field3",T_REAL,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, SPNULL, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,CNULL,T_REAL,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, Field NULL, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field3",6,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, Bad Type, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,CNULL)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, Null value pointer, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }

    if ((rtn=sp_h_get_field(sp,"field3",T_INTEGER,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command Accessed as an INTEGER returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_STRING,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command Accessed as an STRING returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_STRING,&lint)) < 100){
	printf("    sp_h_get_field: Invalid INTEGER command accessed as a STRING failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_REAL,&lint)) < 100){
	printf("    sp_h_get_field: Invalid INTEGER command accessed as a REAL failed\n");
	sp_print_return_status(stdout);
    }
    if (sp_h_get_field(sp,"field1",T_REAL,&str) < 100){
	printf("    sp_h_get_field: Invalid STRING command accessed as a REAL failed\n");
	sp_print_return_status(stdout);
    }
    if (sp_h_get_field(sp,"field1",T_INTEGER,&str) < 100){
	printf("    sp_h_get_field: Invalid STRING command accessed as a INTEGER failed\n");
	sp_print_return_status(stdout);
   }

    printf("------ Verifying 3 header fields\n");
    if ((sp->write_spifr->header->fc != 3) || (sp->write_spifr->status->file_header->fc != 3)){
	printf("***************************************************\n");
	printf("*  The following header should have three fields  *\n");
	printf("***************************************************\n");
	sp_file_dump(sp,stdout);
	exit(-1);
    }

    printf("------ Illegal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(SPNULL,"field1")) < 100){
	printf("    sp_h_delete_field: Invalid Deletion, SPNULL, failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_delete_field(sp,CNULL)) < 100){
	printf("    sp_h_delete_field: Invalid Deletion, Field name NULL, failed\n");
	sp_print_return_status(stdout);
    }
    if (((rtn=sp_h_delete_field(sp,"field84")) == 0) && (rtn >= 100)){
	printf("    sp_h_delete_field: Invalid Deletion, field already deleted, failed\n");
	sp_print_return_status(stdout);
    }

    printf("------ Legal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(sp,"field1")) != 0){
	printf("    sp_h_delete_field: Valid STRING Deletion failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_delete_field(sp,"field2")) != 0){
	printf("    sp_h_delete_field: Valid INTEGER Deletion failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_delete_field(sp,"field3")) != 0){
	printf("    sp_h_delete_field: Valid REAL Deletion failed\n");
	sp_print_return_status(stdout);
    }
    printf("------ Verifying an empty header\n");
    if ((sp->write_spifr->header->fc != 0) || (sp->write_spifr->status->file_header->fc != 0)){
	printf("***************************************************\n");
	printf("*      The following header should be empty       *\n");
	printf("***************************************************\n");
	sp_file_dump(sp,stdout);
	exit(-1);
    }
    
    sp_close(sp);
    system("rm -f testing.wav");
    printf("\n");
    
    printf("-- Read Mode header operations:\n");
    if ((sp=sp_open(EXAMPLE1_10,"r")) == SPNULL) {
	printf("   sp_open: Valid spopen for read of file '%s' failed\n",EXAMPLE1_10);
	sp_print_return_status(stdout);
	exit(-1);
    }

    printf("---- Testing the file header io:\n");
    printf("------ Field creation:\n");
    if (sp_h_set_field(SPNULL,"field1",T_STRING,"char string 1") == 0)
	printf("    sp_h_set_field: Null SPFILE pointer failed\n");
    if (sp_h_set_field(sp,CNULL,T_STRING,"char string 1") == 0)
	printf("    sp_h_set_field: Null field name failed\n");
    if (sp_h_set_field(sp,"field1",4930,"char string 1") == 0)
	printf("    sp_h_set_field: Invalid field type failed\n");
    if (sp_h_set_field(sp,"field1",T_STRING,CNULL) == 0)
	printf("    sp_h_set_field: Null value failed\n");

    if (sp_h_set_field(sp,"field1",T_STRING,"string value1") != 0){
	printf("    sp_h_set_field: valid STRING command failed\n");
	sp_print_return_status(stdout);
    }
    lint=1;
    if (sp_h_set_field(sp,"field2",T_INTEGER,&lint) != 0){
	printf("    sp_h_set_field: valid INTEGER command failed\n");
	sp_print_return_status(stdout);
	exit(-1);
    }
    real=2.0;
    if (sp_h_set_field(sp,"field3",T_REAL,&real) != 0){
	printf("    sp_h_set_field: valid REAL command failed\n");
	sp_print_return_status(stdout);
    }

    printf("------ Field access:\n");
    if (sp_h_get_field(sp,"field1",T_STRING,&str) != 0){
	printf("    sp_h_get_field: valid STRING command failed\n");
	sp_print_return_status(stdout);
    }
    mtrf_free(str);
    lint=1;
    if (sp_h_get_field(sp,"field2",T_INTEGER,&lint) != 0){
	printf("    sp_h_get_field: valid INTEGER command failed\n");
	sp_print_return_status(stdout);
	exit(-1);
    }
    real=2.0;
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,&real)) != 0){
	printf("    sp_h_get_field: valid REAL command returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
	   
    printf("------ Illegal Field access:\n");
    if ((rtn=sp_h_get_field(SPNULL,"field3",T_REAL,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, SPNULL, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,CNULL,T_REAL,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, Field NULL, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field3",6,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, Bad Type, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,CNULL)) < 100){
	printf("    sp_h_get_field: Invalid REAL command, Null value pointer, returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }

    if ((rtn=sp_h_get_field(sp,"field3",T_INTEGER,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command Accessed as an INTEGER returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_STRING,&real)) < 100){
	printf("    sp_h_get_field: Invalid REAL command Accessed as an STRING returned %d and failed\n",rtn);
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_STRING,&lint)) < 100){
	printf("    sp_h_get_field: Invalid INTEGER command accessed as a STRING failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_REAL,&lint)) < 100){
	printf("    sp_h_get_field: Invalid INTEGER command accessed as a REAL failed\n");
	sp_print_return_status(stdout);
    }
    if (sp_h_get_field(sp,"field1",T_REAL,&str) < 100){
	printf("    sp_h_get_field: Invalid STRING command accessed as a REAL failed\n");
	sp_print_return_status(stdout);
    }
    if (sp_h_get_field(sp,"field1",T_INTEGER,&str) < 100){
	printf("    sp_h_get_field: Invalid STRING command accessed as a INTEGER failed\n");
	sp_print_return_status(stdout);
    }

    printf("------ Illegal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(SPNULL,"field1")) < 100){
	printf("    sp_h_delete_field: Invalid Deletion, SPNULL, failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_delete_field(sp,CNULL)) < 100){
	printf("    sp_h_delete_field: Invalid Deletion, Field name NULL, failed\n");
	sp_print_return_status(stdout);
    }
    if (((rtn=sp_h_delete_field(sp,"field84")) == 0) && (rtn >= 100)){
	printf("    sp_h_delete_field: Invalid Deletion, field already deleted, failed\n");
	sp_print_return_status(stdout);
    }

    printf("------ Legal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(sp,"field1")) != 0){
	printf("    sp_h_delete_field: Valid STRING Deletion failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_delete_field(sp,"field2")) != 0){
	printf("    sp_h_delete_field: Valid INTEGER Deletion failed\n");
	sp_print_return_status(stdout);
    }
    if ((rtn=sp_h_delete_field(sp,"field3")) != 0){
	printf("    sp_h_delete_field: Valid REAL Deletion failed\n");
	sp_print_return_status(stdout);
    }
    printf("\n");

    sp_close(sp);
}

diff_files(file1,file2)
char *file1, *file2;
{

    FILE *fp1, *fp2;
    char c1, c2;
    fp1 = fopen(file1, "r");
    fp2 = fopen(file2, "r");
    do {
	c1 = fgetc(fp1);
	c2 = fgetc(fp2);
	if (c1 != c2){
	    fclose(fp1);
	    fclose(fp2);
	    return(100);
	}
    } while (!feof(fp1));
    if (!feof(fp2)){
	fclose(fp1);
	fclose(fp2);
	return(100);
    }
    fclose(fp1);
    fclose(fp2);
    return(0);
}


diff_data(file1,file2)
char *file1, *file2;
{

    FILE *fp1, *fp2;
    struct header_t *h1, *h2;
    char c1, c2, *errmsg;

    fp1 = fopen(file1, "r");
    fp2 = fopen(file2, "r");
    if ((h1 = sp_open_header(fp1,TRUE,&errmsg)) == HDRNULL){
	printf("diff_data: Unable to open SPHERE header for file %s\n",file2);
	exit(-1);
    }
    if ((h2 = sp_open_header(fp2,TRUE,&errmsg)) == HDRNULL){
	printf("diff_data: Unable to open SPHERE header for file %s\n",file2);
	exit(-1);
    }
    do {
	c1 = fgetc(fp1);
	c2 = fgetc(fp2);
	if (c1 != c2){
	    fclose(fp1);
	    fclose(fp2);
	    return(100);
	}
    } while (!feof(fp1));
    if (!feof(fp2)){
	sp_close_header(h1);
	sp_close_header(h2);
	fclose(fp1);
	fclose(fp2);
	return(100);
    }
    sp_close_header(h1);
    sp_close_header(h2);
    fclose(fp1);
    fclose(fp2);
    return(0);
}

diff_header(file1,file2,chg,ins,del,verbose)
char *file1, *file2;
int *chg, *ins, *del;
int verbose;
{
    FILE *fp1, *fp2;
    struct header_t *h1, *h2;
    char c1, c2, *errmsg;
    int i1, i2, found;

    *chg = *ins = *del = 0;

    fp1 = fopen(file1, "r");
    fp2 = fopen(file2, "r");
    if ((h1 = sp_open_header(fp1,TRUE,&errmsg)) == HDRNULL){
	printf("diff_data: Unable to open SPHERE header for file %s\n",file2);
	exit(-1);
    }
    if ((h2 = sp_open_header(fp2,TRUE,&errmsg)) == HDRNULL){
	printf("diff_data: Unable to open SPHERE header for file %s\n",file2);
	exit(-1);
    }
    for (i1=0 ;i1 < h1->fc; i1++) {
	found=0;
	for (i2=0 ;i2 < h2->fc; i2++) {
	    if (strsame(h1->fv[i1]->name,h2->fv[i2]->name)) {
		found=1;
		if (h1->fv[i1]->type != h2->fv[i2]->type){
		    if (verbose) printf("    Changed field '%s' type %d -> %d\n",
					h1->fv[i1]->name,h1->fv[i1]->type,h2->fv[i2]->type);
		    *chg += 1;
		    continue;
		} else {
		    if (! strsame(h1->fv[i1]->data,h2->fv[i2]->data)){
			if (strsame(h1->fv[i1]->name,SAMPLE_CODING_FIELD) && 
			    (strncmp(h1->fv[i1]->data,h2->fv[i2]->data,20) == 0))
			    ;
			else {
			    if (verbose) printf("    Changed field '%s' value %s -> %s\n",
						h1->fv[i1]->name,h1->fv[i1]->data,h2->fv[i2]->data);
			    *chg += 1;
			}
		    }
		    continue;
		}
	    }
	}
	if (found == 0){
	    if (verbose) printf("    Deleted field %s\n",h1->fv[i1]->name);
	    *del += 1;
	}
    }		

    for (i2=0 ;i2 < h2->fc; i2++) {
	found=0;
	for (i1=0 ;found==0 && i1 < h1->fc; i1++) {
	    if (strsame(h1->fv[i1]->name,h2->fv[i2]->name)) 
		found=1;
	}
	if (found == 0){
	    if (verbose) printf("    Inserted field %s\n",h2->fv[i2]->name);
	    *ins += 1;
	}
    }		

    sp_close_header(h1);
    sp_close_header(h2);

    fclose(fp1);
    fclose(fp2);
    return(0);
}


diff_waveforms(file1,file2)
char *file1, *file2;
{
    SP_FILE *sp1, *sp2;
    short buff1[512], buff2[512];
    int n1, n2, i, samp=0, fail=0;

    if ((sp1 = sp_open(file1, "r")) == SPNULL){
	fail=1;
	fprintf(stdout,"DIFFERENT WAVEFORM: files %s and %s\n",file1,file2);
	fprintf(stdout,"sp_open failed on file %s\n",file1);
	sp_print_return_status(stdout);
    }
	
    if ((sp2 = sp_open(file2, "r")) == SPNULL){
	if (fail == 0)
	    fprintf(stdout,"DIFFERENT WAVEFORM: files %s and %s\n",file1,file2);
	fail=1;
	fprintf(stdout,"sp_open failed on file %s\n",file2);
	sp_print_return_status(stdout);
    }
    if (fail == 1){
	if (sp1 != SPNULL)
	    sp_close(sp1);
	if (sp2 != SPNULL)
	    sp_close(sp2);
	exit(-1);
    }	

    do {

	n1 = sp_read_data((char *)buff1,2,512,sp1);
	n2 = sp_read_data((char *)buff2,2,512,sp2);
	if (n1 != n2){
	    fprintf(stdout,"DIFFERENT WAVEFORM: files %s and %s\n",file1,file2);
	    fprintf(stdout,"   Beginning sample: %d\n",samp);
	    fprintf(stdout,"   %d samples read from %s\n",n1,file1);
	    fprintf(stdout,"   %d samples read from %s\n",n2,file2);
	    sp_close(sp1);
	    sp_close(sp2);
	    return(100);
	}
	for (i=0; i<n1; i++, samp++)
	    if (buff1[i] != buff2[i]){
		fprintf(stdout,"DIFFERENT WAVEFORM: files %s and %s\n",file1,file2);
		fprintf(stdout,"   Sample discrepancy: %d\n",samp);
		sp_close(sp1);
		sp_close(sp2);
		return(100);
	    }	
		
    } while (!sp_eof(sp1));
    if (!sp_eof(sp2)){
	fprintf(stdout,"DIFFERENT WAVEFORM: files %s and %s\n",file1,file2);
	fprintf(stdout,"   file %s not at eof\n",file2);
	sp_close(sp1);
	sp_close(sp2);
	return(100);
    }
    sp_close(sp1);
    sp_close(sp2);
    return(0);
}

update_sphere_file(filein, format_conversion)
char *filein, *format_conversion;
{

    SP_FILE *sp;

    if ((sp=sp_open(filein,"u")) == SPNULL){
	sp_print_return_status(stdout);
	fprintf(stdout,"Unable to open file '%s' to update\n",filein);
	return(100);
    }
    if (sp_set_data_mode(sp,format_conversion) != 0){
	sp_print_return_status(stdout);
	fprintf(stdout,"Unable to set data mode to '%s'\n",format_conversion);
	sp_close(sp);
	return(100);
    }

    if (sp_close(sp) != 0){
	sp_print_return_status(stdout);
	fprintf(stdout,"File Close failed\n");
	return(100);
    }
    return(0);
}
