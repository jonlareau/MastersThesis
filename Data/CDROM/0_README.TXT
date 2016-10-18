*************************************************************************
Multi-Language Telephone Speech Corpus Distribution

README.TXT

release January, 1994

Copyright 1994
Center for Spoken Language Understanding
Oregon Graduate Institute of Science & Technology
*************************************************************************


This disc contains the following files and directories:

1) readme.txt - this text file

2) overview.txt - file describing the OGI Multi-language Telephone
	Speech Corpus in detail

3) calls/ - A directory containing the directories for each language,
	which in turn contain the speech files for the respective
	languages.  Therefore, the directory structure for calls/ will
	look like the following:

	english/  french/  hindi/     korean/    spanish/   vietnam/
	farsi/    german/  japanese/  mandarin/  tamil/

	The speech files are organized according to a call number
	system (see 4 below, i.e. data.doc).  For ease of file
	handling, the files are divided in groups of 10 per directory.
	For example, the farsi/ directory contains the directories:

	00/     02/     04/     06/     08/     10/     12/     14/
	01/     03/     05/     07/     09/     11/     13/     15/

	Directory 00 contains calls identified by call numbers 0-9;
	directory 15 contains calls identified by call numbers
	150-159; etc.

4) doc/  - directory containing the following documentation files:
        
	data.doc -- file containing the conventions used for naming
		the data files

        formats.doc -- file containing documentation on the speech
		(.wav) and transcription (.ptlola) file formats.

        header.doc -- file giving details of the NIST SPHERE header
		structure

	ph1_logs.doc -- file describing the contents of the .log files
		created during Phase I of the development process

	ph2_logs.doc -- file describing the contents of the .log2
		files created during Phase II of the development
		process

	mltlngdb.ps -- postscript file containing the article: "The
		OGI Multi-language Telephone Speech Corpus" Y. K.
		Muthusamy, R. A. Cole and B. T. Oshika Proceedings of
		the International Conference on Spoken Language
		Processing, Banff, Alberta, Canada, October 1992.

5) seglola/ - directory containing broad phonetic transcriptions (i.e.
	.seg files --- see overview.txt and doc/formats.doc for more
	information on these files).

6) logs/ - directory containing logfiles of corpus development, Phase
	I, which consisted of preliminary verification, chopping,
	evaluation and broad phonetic transcriptions of each utterance
	  
7) logs2/ - directory containing logfiles of corpus development, Phase
	II, consisting of verification and evaluation of calls by
	native speakers of each individual language

8) trn_test/ - directory containing files describing the training,
	development and test sets used by Yeshwant Muthusamy for his
	Ph.D. Thesis research.

9) sphere/ - directory containing files needed to uncompress the .wav
	files


PLEASE NOTE:
------------

This publication of the OGI Multi-Language Telephone Speech Corpus,
produced on CD-ROM by the Linguistic Data Consortium, contains a few
minor modifications relative to the version distributed on tape by
OGI.  To begin with, directory and file names have been simplified
where necessary to conform to ISO 9660 conventions for file naming.
In addition, we have included the more current SPHERE package (version
2.0) from NIST, and have applied a more effective waveform compression
algorithm (the "shorten" compression method developed by Tony Robinson
of Cambridge University, as implemented in the current release of
SPHERE).  In performing this conversion of the waveform data, we also
supplemented the information in each file's SPHERE header to include
common header fields that were missing from the original files (sample
min & max, sample coding).  Relevant changes to the various log and
documentation files have been made as necessary.  Finally, the tape
distributed by OGI contained numerous log files for which there were
no corresponding speech data, as well as a few empty log files; these
have been eliminated from this release.

