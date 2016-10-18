/*   File: spfile.h
     Created: 05/06/93
  */

typedef int SIGNED_2BYTE;
typedef unsigned char UNSIGNED_1BYTE;

enum SP_file_open_mode  { SP_mode_null=100, SP_mode_read, SP_mode_write, 
			      SP_mode_update };
enum SP_sample_byte_fmt { SP_sbf_null=200, SP_sbf_01, SP_sbf_10, SP_sbf_1,
			      SP_sbf_N, SP_sbf_orig};
enum SP_sample_encoding { SP_se_null=300, SP_se_pcm2, SP_se_pcm1,
			      SP_se_ulaw, SP_se_raw};
enum SP_waveform_comp   { SP_wc_null=400, SP_wc_shorten, SP_wc_wavpack,
			      SP_wc_shortpack, SP_wc_none };

struct waveform_t {
    FILE *sp_fp;
    FOB *sp_fob;
    int samples_read;
    int samples_written;
    SP_CHECKSUM checksum;
    long header_data_size;
    int read_premature_eof;
    int failed_checksum;
};


struct spfile_status_t {
    char *external_filename;
    struct header_t *file_header;
    int extra_checksum_verify;
    int is_disk_file;
    int is_temp_file;
    char *temp_filename;

    int user_channel_count, file_channel_count;
    int user_sample_count, file_sample_count;
    int user_sample_rate, file_sample_rate;
    int user_sample_n_bytes, file_sample_n_bytes;
    SP_CHECKSUM file_checksum;
    int ignore_checksum;

    enum SP_sample_encoding    user_encoding, file_encoding;
    enum SP_waveform_comp      user_compress, file_compress;
    enum SP_sample_byte_fmt    user_sbf, file_sbf;
    enum SP_sample_byte_fmt    natural_sbf;

    int write_occured_flag;
    int read_occured_flag;
    int field_set_occured_flag;
    int set_data_mode_occured_flag;
};

typedef struct sphere_internal_file_rep_t {
    struct header_t *header;
    struct waveform_t *waveform;
    struct spfile_status_t *status;
} SPIFR;

typedef struct sphere_t {
    enum SP_file_open_mode open_mode;
    SPIFR *read_spifr;
    SPIFR *write_spifr;
} SP_FILE;

#define SPNULL		((SP_FILE *) NULL)
#define SPIFRNULL	((SPIFR *) NULL)
