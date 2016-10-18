#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/stdfield.c,v 1.4 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* LINTLIBRARY */

/* File: stdfield.c */

#include <stdio.h>

#include <sp/sphere.h>

char *std_fields[] = {
	"database_id",
	"database_version",
	"utterance_id",
	"channel_count",
	"sample_count",
	"sample_rate",
	"sample_min",
	"sample_max",
	"sample_n_bytes",
	"sample_byte_format",
	"sample_sig_bits",
	CNULL
};
