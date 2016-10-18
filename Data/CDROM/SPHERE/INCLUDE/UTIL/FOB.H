typedef struct fob_t {
	FILE * fp;		/* if not NULL, use stdio.h functions */
        int read_byte_swap;     /* if true, swap after read */
        int write_byte_swap;    /* if true, swap after write */
	int length;		/* #bytes read or written */
	char * buf;		/* allocated buffer pointer */
	char * buf_swap;        /* if the write operation requries byte*/
	                        /* swapping, it's done here */
	int bufsize;		/* allocated buffer size */
	char * pos;		/* current position in buffer */
} FOB;


#define FOB_BUF_GRAN	8192
#define FOBPNULL	( (FOB *) NULL )

extern int fob_fread();
extern int fob_fwrite();
extern int fob_ferror();
extern int fob_putc();
extern int fob_getc();
extern int fob_getw();
extern FOB * fob_create();
extern int fob_destroy();
extern int fob_create2();
extern int fob_destroy2();

/*
 * lseek & access args
 *
 * SEEK_* have to track L_* in sys/file.h & SEEK_* in 5include/stdio.h
 * ?_OK have to track ?_OK in sys/file.h
 */
#ifndef SEEK_SET
#define SEEK_SET        0       /* Set file pointer to "offset" */
#define SEEK_CUR        1       /* Set file pointer to current plus "offset" */
#define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif
