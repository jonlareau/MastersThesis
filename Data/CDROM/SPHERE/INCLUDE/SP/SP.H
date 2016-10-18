/* File: sp.h */

/* Global variables */

extern int sp_verbose;

extern char *std_fields[];



/* User library functions */

struct header_t *sp_open_header();
struct header_t *sp_create_header();
int sp_close_header();

FILE *sp_get_fp();
int sp_set_fp();

int sp_get_hbytes();
int sp_get_nfields();
int sp_get_fieldnames();

int sp_print_lines();
int sp_format_lines();
int sp_fpcopy();

int sp_get_field();
int sp_get_data();
int sp_get_type();
int sp_get_size();

int sp_clear_fields();
int sp_add_field();
int sp_delete_field();
int sp_change_field();

int sp_is_std();

int sp_set_dealloc();
int sp_get_dealloc();

long sp_header_size();

SP_FILE *sp_open();
int sp_close();
SP_FILE *sp_alloc_and_init_sphere_t();

struct header_t *sp_dup_header();

unsigned short sp_compute_short_checksum();

char * sptemp();
char * sptemp_dirfile();

int sp_pass_checksum();
int sp_h_get_field();
int sp_h_set_field();
int sp_h_delete_field();
int h_get_field();
int h_set_field();
int h_delete_field();
int sp_set_data_mode();
int sp_read_data();
int sp_write_data();

/*****/
char *rsprintf();
enum SP_sample_byte_fmt get_natural_sbf();
char *get_natural_byte_order();

SP_CHECKSUM sp_compute_short_checksum();
SP_CHECKSUM sp_add_checksum();

