/******************************************************************************

Copyright (C) 1992,1993 Tony Robinson

Permission is granted to use this software for non-commercial purposes.
Explicit permission must be obtained from the author to use this software
for commercial purposes.

This software carries no warranty, expressed or implied.  The user assumes
all risks, known or unknown, direct or indirect, which involve this software
in any way.

Dr Tony Robinson
Cambridge University Engineering Department
Trumpington Street, Cambridge, CB2 1PZ, UK.
ajr@eng.cam.ac.uk     voice: +44-223-332815

******************************************************************************/

# define MAGIC			"ajkg"
# define FORMAT_VERSION		1
# define DEFAULT_BLOCK_SIZE	256
# define DEFAULT_NMEAN		0
# define DEFAULT_MAXNLPC	0
# define MAX_LPC_ORDER		64
# define DEFAULT_NCHAN		1
# define DEFAULT_NSKIP		0
# define DEFAULT_NDISCARD	0
# define DEFAULT_MAXBITRATE	32
# define DEFAULT_QUANTERROR	0

# define CHANSIZE	0
# define ENERGYSIZE	3
# define BITSHIFTSIZE	2
# define NWRAP		3

# define FNSIZE		2
# define FN_DIFF0	0
# define FN_DIFF1	1
# define FN_DIFF2	2
# define FN_DIFF3	3
# define FN_QUIT	4
# define FN_BLOCKSIZE	5
# define FN_BITSHIFT	6
# define FN_QLPC	7

# define ULONGSIZE	2
# define NSKIPSIZE	1
# define LPCQSIZE	2
# define LPCQUANT	5
# define XBYTESIZE	7

# define TYPESIZE	4
# define TYPE_AU	0
# define TYPE_S8	1
# define TYPE_U8	2
# define TYPE_S16HL	3
# define TYPE_U16HL	4
# define TYPE_S16LH	5
# define TYPE_U16LH	6

#ifndef	MIN
# define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef	MAX
# define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef M_LN2
#define	M_LN2	0.69314718055994530942
#endif

/* BUFSIZ must be a multiple of four to contain a whole number of words */
#ifndef BUFSIZ
# define BUFSIZ 1024
#endif

#undef	uchar
#define uchar	unsigned char
#undef	ushort
#define ushort	unsigned short
#undef	ulong
#define ulong	unsigned long

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
typedef signed char	schar;
#define PROTO(ARGS)	ARGS
#else
typedef char		schar;
#define PROTO(ARGS)	()
#endif

 /*******************************************/
/* this should be in string.h or strings.h */
extern int	strcmp		PROTO((const char*, const char*));
extern char*	strcpy		PROTO((char*, const char*));
extern char*	strcat		PROTO((char*, const char*));
extern int	strlen		PROTO((const char*));

 /**************************************/
/* defined in stdlib.h if you have it */
extern void*	malloc		PROTO((unsigned));
extern void	free		PROTO((char*));
extern int	atoi		PROTO((char*));
extern void	swab		PROTO((char*, char*, int));
extern int	fseek		PROTO((FILE*, long, int));

 /***************************/
/* other misc system calls */
extern int	unlink		PROTO((const char*));

 /**********************/
/* defined in fixio.c */
extern int	fread_type	PROTO((long**, int, int, int, FOB*));
extern void	fwrite_type	PROTO((long**, int, int, int, FOB*));
extern int	find_bitshift	PROTO((long*, int, int));
extern void	fix_bitshift	PROTO((long*, int, int, int));


 /**********************/
/* defined in vario.c */
extern void	var_put_init	PROTO((FOB*));
extern void	uvar_put	PROTO((ulong, int, FOB*));
extern void	var_put		PROTO((long, int, FOB*));
extern void	ulong_put	PROTO((ulong, FOB*));
extern void	var_put_quit	PROTO((FOB*));

extern void	var_get_init	PROTO((FOB*));
extern long	uvar_get	PROTO((int, FOB*));
extern long	var_get		PROTO((int, FOB*));
extern ulong	ulong_get	PROTO((FOB*));
extern void	var_get_quit	PROTO((FOB*));

extern int	sizeof_uvar	PROTO((ulong, int));
extern int	sizeof_var	PROTO((long, int));


 /********************/
/* defined in lpc.c */
extern int	wav2lpc		PROTO((long*, int, int*, int, int*));

 /*********************/
/* defined in exit.c */
extern void	perror_exit_s	PROTO((char*));
extern void	perror_exit_ss	PROTO((char*, char*));
extern void	perror_exit_sd	PROTO((char*, int));
extern void	basic_exit	PROTO((int));
extern void	usage_exit	PROTO((int));
extern void	usage_exit_s	PROTO((int, char*));
extern void	usage_exit_sd	PROTO((int, char*, int));
extern void	usage_exit_ss	PROTO((int, char*, char*));
extern void	usage_exit_sss	PROTO((int, char*, char*, char*));
extern void	update_exit	PROTO((int));
extern void	update_exit_s	PROTO((int, char*));
extern void	update_exit_sd	PROTO((int, char*, int));
extern void	exit		PROTO((int));

 /***********************/
/* defined in getopt.c */
extern void	resetopt	PROTO((void));
extern int	getopt		PROTO((int, char**, char*));

 /**********************/
/* defined in array.c */
extern void	*pmalloc	PROTO((ulong));
extern long	**long2d	PROTO((ulong, ulong));

 /****************************/
/* defined in dupfileinfo.c */
extern int	dupfileinfo	PROTO((char*, char*));
