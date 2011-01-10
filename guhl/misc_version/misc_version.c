/*
    Copyright (C) 2010  Guhl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include "gopt.h"
#include <time.h>
#include <string.h>

#define INFILE "/dev/block/mmcblk0p17"
#define OUTFILE "/dev/block/mmcblk0p17"
#define BACKUPFILE "/sdcard/part17backup-%lu.bin"

//#define INFILE "/opt/install/g2/my_dump/p17/mmcblk0p17.img"
//#define OUTFILE "/opt/install/g2/my_dump/p17/mmcblk0p17-new.img"
//#define BACKUPFILE "/opt/install/g2/my_dump/p17/part17backup-%lu.bin"

#define VERSION_A	0
#define VERSION_B	2

int main(int argc, const char **argv) {

	int cid = 0, set_version = 0, help = 0;
	const char* s_set_version;
    const char* s_cid;

	if (argc>1) {

		void *options= gopt_sort( & argc, argv, gopt_start(
		  gopt_option( 'h', 0, gopt_shorts( 'h', '?' ), gopt_longs( "help", "HELP" )),
		  gopt_option( 'v', 0, gopt_shorts( 'v' ), gopt_longs( "version" )),
		  gopt_option( 'c', GOPT_ARG, gopt_shorts('c'), gopt_longs("cid")),
		  gopt_option( 's', GOPT_ARG, gopt_shorts( 's' ), gopt_longs( "set_version" ))));

		if( gopt( options, 'h' ) ){
			help = 1;
		}

		if( gopt( options, 'v' ) ){
			fprintf( stdout, "misc_version version: %d.%d\n",VERSION_A,VERSION_B);
			exit (0);
		}

		if(gopt_arg(options, 'c', &s_cid))
		{
		    // if -c or --cid was specified, check s_cid
		    size_t size;
		    size = strlen(s_cid);
		    if(size != 8)
		    {
			printf("Error: CID must be a 8 character string. Length of specified string: %d\n", (int)size);
			exit(1);
		    }
		    else
		    {
			cid = 1;
			printf("--cid set. CID will be changed to: %s\n", s_cid);
		    }
		}

		if( gopt_arg(options, 's', &s_set_version)){
			// if -a or --set_version was specified, check s_set_version
			size_t size;
			size = strlen(s_set_version);
			if (size!=10){
				fprintf( stderr, "Error: VERSION must be a 10 character string. Length of specified string: %d\n",(int)size);
				exit (1);
			} else {
				set_version = 1;
				fprintf( stdout, "--set_version set. VERSION will be changed to: %s\n",s_set_version);
			}
		}

	} else {
		help = 1;
	}

	if (help!=0){
		//if any of the help options was specified
		fprintf( stdout, "misc_version usage:\n" );
		fprintf( stdout, "misc_version [-h|-?|--help] [-v|--version] [-s|--set_version <VERSION>]\n" );
		fprintf( stdout, "\t-h | -? | --help: display this message\n" );
		fprintf( stdout, "\t-v | --version: display program version\n" );
		fprintf( stdout, "\t-c | --cid <CID>: set the CID in misc to the 8-char long CID\n");
		fprintf( stdout, "\t-s | --set_version <VERSION>:  set the version in misc to the 10-char long VERSION\n" );
		exit(0);
	}

    char *backupFile;
    time_t ourTime;

    ourTime = time(0);
    backupFile = malloc(snprintf(0, 0, BACKUPFILE, ourTime) + 1);
    sprintf(backupFile, BACKUPFILE, ourTime);

	FILE *fdin, *fdout;
	char ch;

    printf("Patching and backing up partition 17...\n");
	fdin = fopen(INFILE, "rb");
	if (fdin == NULL){
		printf("Error opening input file.\n");
		return -1;
	}

	fdout = fopen(backupFile, "wb");
	if (fdout == NULL){
		printf("Error opening backup file.\n");
		return -1;
	}

//  create a copy of the partition
	while(!feof(fdin)) {
	    ch = fgetc(fdin);
	    if(ferror(fdin)) {
	      printf("Error reading input file.\n");
	      exit(1);
	    }
	    if(!feof(fdin)) fputc(ch, fdout);
	    if(ferror(fdout)) {
	      printf("Error writing backup file.\n");
	      exit(1);
	    }
	}
	if(fclose(fdin)==EOF) {
		printf("Error closing input file.\n");
		exit(1);
	}

	if(fclose(fdout)==EOF) {
		printf("Error closing backup file.\n");
		exit(1);
	}

//  copy back and patch
	long j;

	fdin = fopen(backupFile, "rb");
	if (fdin == NULL){
		printf("Error opening backup file.\n");
		return -1;
	}

	fdout = fopen(OUTFILE, "wb");
	if (fdout == NULL){
		printf("Error opening output file.\n");
		return -1;
	}

	j = 0;

	while(!feof(fdin)) {
	    ch = fgetc(fdin);
	    if(ferror(fdin)) {
	      printf("Error reading backup file.\n");
	      exit(1);
	    }
		// CID
		if ((j>=0x0 && j<=0x7)&& (cid!=0)) {
			ch = s_cid[j];
		}
		// VERSION
		if ((j>=0xa0 && j<=0xa9)&& (set_version!=0)) {
			ch = s_set_version[j-0xa0];
		}
		if(!feof(fdin)) fputc(ch, fdout);
		if(ferror(fdout)) {
		  printf("Error writing output file.\n");
		  exit(1);
		}
		j++;
	}
	if(fclose(fdin)==EOF) {
		printf("Error closing backup file.\n");
		exit(1);
	}

	if(fclose(fdout)==EOF) {
		printf("Error closing output file.\n");
		exit(1);
	}

	return 0;
}
