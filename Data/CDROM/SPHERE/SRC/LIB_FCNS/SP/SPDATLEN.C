#ifndef lint
  static char rcsid[] = "$Header: /home/beldar/stan/sphere/RCS/spdatlen.c,v 1.3 1993/03/25 00:20:51 stan Exp stan $";
#endif

/* LINTLIBRARY */

#include <stdio.h>
#include <stdlib.h>

#include <sp/sphere.h>



sp_data_length( prog, file, h )
	char * prog;
	char * file;
	struct header_t * h;
{
	long channel_count;
	long sample_count;
	long sample_n_bytes;
	long length;
	char * field;
	int n;


	field = "channel_count";
	n = sp_integer( h, field, &channel_count );
	if ( n < 0 ) {
		fprintf( stderr, "%s: %s: no integer field \"%s\"\n",
				prog, file, field );
		return -1;
	}

	field = "sample_count";
	n = sp_integer( h, field, &sample_count );
	if ( n < 0 ) {
		fprintf( stderr, "%s: %s: no integer field \"%s\"\n",
				prog, file, field );
		return -1;
	}

	field = "sample_n_bytes";
	n = sp_integer( h, field, &sample_n_bytes );
	if ( n < 0 ) {
		fprintf( stderr, "%s: %s: no integer field \"%s\"\n",
				prog, file, field );
		return -1;
	}

	length = channel_count * sample_count * sample_n_bytes;
	return (int) length;
}
