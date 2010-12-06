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
#include <string.h>
#include "gopt.h"
#include "soff_verify.h"

/*
#define INFILE "/dev/block/mmcblk0p7"
#define CPYFILE "/sdcard/mmcblk0p7.ori.img"
#define OUTFILE "/dev/block/mmcblk0p7"
*/
#define INFILE "/opt/install/g2/my_dump/p7/mmcblk0p7.ori.img"
#define CPYFILE "/opt/install/g2/my_dump/p7/mmcblk0p7.cpy.img"
#define OUTFILE "/opt/install/g2/my_dump/p7/mmcblk0p7.out.img"

#define VERSION_A	0
#define VERSION_B	11

int main(int argc, const char **argv) {
	FILE *fdin, *fdout;
	char ch;
	int cid, secu_flag, sim_unlock, verify = 0;
	const char* s_secu_flag;
	const char* s_cid;

	if (argc>1) {

		void *options= gopt_sort( & argc, argv, gopt_start(
		  gopt_option( 'h', 0, gopt_shorts( 'h', '?' ), gopt_longs( "help", "HELP" )),
		  gopt_option( 'v', 0, gopt_shorts( 'v' ), gopt_longs( "version" )),
		  gopt_option( 's', GOPT_ARG, gopt_shorts( 's' ), gopt_longs( "secu_flag" )),
		  gopt_option( 'c', GOPT_ARG, gopt_shorts( 'c' ), gopt_longs( "cid" )),
		  gopt_option( 'S', 0, gopt_shorts( 'S' ), gopt_longs( "sim_unlock" )),
		  gopt_option( 'V', 0, gopt_shorts( 'V' ), gopt_longs( "verify" ))));
		/*
		* there are possible options to this program, some of which have multiple names:
		*
		* -h -? --help --HELP
		* -v --version
		* -s --secu_flag	(which requires an option argument on|off)
		* -c --cid			(which requires an option 8-character string argument <CID>)
		* -S --sim_unlock
		*/

		if( gopt( options, 'h' ) ){
			//if any of the help options was specified
			fprintf( stdout, "gfree usage:\n" );
			fprintf( stdout, "gfree [-h|-?|--help] [-v|--version] [-s|--secu_flag on|off]\n" );
			fprintf( stdout, "\t-h | -? | --help: display this message\n" );
			fprintf( stdout, "\t-v | --version: display program version\n" );
			fprintf( stdout, "\t-s | --secu_flag on|off: turn secu_flag on or off\n" );
			fprintf( stdout, "\t-c | --cid <CID>: set the CID to the 8-char long CID\n" );
			fprintf( stdout, "\t-S | --sim_unlock: remove the SIMLOCK\n" );
			fprintf( stdout, "\t-V | --verify: verify the CID, secu_flag, SIMLOCK\n" );
			fprintf( stdout, "\n" );
			fprintf( stdout, "calling gfree without arguments is the same as calling it:\n" );
			fprintf( stdout, "\tgfree --secu_flag off --sim_unlock --cid 11111111\n" );
			exit( 0 );
		}

		if( gopt( options, 'v' ) ){
			//if any of the version options was specified
			fprintf( stdout, "gfree version: %d.%d\n",VERSION_A,VERSION_B);
			exit (0);
		}

		if( gopt_arg(options, 's', &s_secu_flag)){
			// if -s or --secu_flag was specified, check s_secu_flag
			if (strcmp(s_secu_flag, "on")==0){
				secu_flag = 1;
				fprintf( stdout, "--secu_flag on set\n");
			} else if (strcmp(s_secu_flag, "off")==0){
				secu_flag = 2;
				fprintf( stdout, "--secu_flag off set\n");
			}
		}

		if( gopt_arg(options, 'c', &s_cid)){
			// if -c or --cid was specified, check s_cid
			size_t size;
			size = strlen(s_cid);
			if (size!=8){
				fprintf( stderr, "Error: CID must be a 8 character string. Length of specified string: %d\n",(int)size);
				exit (1);
			} else {
				cid = 1;
				fprintf( stdout, "--cid set. CID will be changed to: %s\n",s_cid);
			}
		}

		if( gopt( options, 'S' ) ){
			//if any of the sim_unlock options was specified
			sim_unlock = 1;
			fprintf( stdout, "--sim_unlock. SIMLOCK will be removed\n");
		}

		if( gopt( options, 'V' ) ){
			//if any of the sim_unlock options was specified
			verify = 1;
			fprintf( stdout, "--verify. CID, secu_flag, SIMLOCK will be verified\n");
		}

	} else {
		secu_flag = 2;
		fprintf( stdout, "--secu_flag off set\n");
		cid = 1;
		s_cid = "11111111";
		fprintf( stdout, "--cid set. CID will be changed to: %s\n",s_cid);
		sim_unlock = 1;
		fprintf( stdout, "--sim_unlock. SIMLOCK will be removed\n");
	}

	// if verify is set just verify and exit
	if (verify==1){
		if (verify_init_device()!=0){
			fprintf( stderr, "Error: verify could not initialize device!");
			exit (-1);
		}
		if (verify_init_modem()!=0){
			fprintf( stderr, "Error: verify could not initialize radio!");
			exit (-1);
		}
		verify_set_verbose();
		verify_cid();
		verify_secu_flag();
		verify_simlock();
		verify_close_device();
		exit (0);
	}

	fdin = fopen(INFILE, "rb");
	if (fdin == NULL){
		printf("Error opening input file.\n");
		return -1;
	}

	fdout = fopen(CPYFILE, "wb");
	if (fdout == NULL){
		printf("Error opening copy file.\n");
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
	      printf("Error writing copy file.\n");
	      exit(1);
	    }
	}
	if(fclose(fdin)==EOF) {
		printf("Error closing input file.\n");
		exit(1);
	}

	if(fclose(fdout)==EOF) {
		printf("Error closing copy file.\n");
		exit(1);
	}

//  copy back and patch
	long i;

	fdin = fopen(CPYFILE, "rb");
	if (fdin == NULL){
		printf("Error opening copy file.\n");
		return -1;
	}

	fdout = fopen(OUTFILE, "wb");
	if (fdout == NULL){
		printf("Error opening output file.\n");
		return -1;
	}

	i = 0;

	while(!feof(fdin)) {
	    ch = fgetc(fdin);
	    if(ferror(fdin)) {
	      printf("Error reading copy file.\n");
	      exit(1);
	    }
	    // secu_flag
	    if (i==0xa00 && secu_flag!=0) {
	    	ch = secu_flag==2 ? 0x00 : 0x01;
	    }
	    // CID
	    if ((i>=0x200 && i<=0x207)&& (cid!=0)) {
	    	ch = s_cid[i-0x200];
	    }
	    // SIM LOCK
	    if (sim_unlock!=0){
			if ((i>0x80003 && i<0x807fc) || (i>=0x80800 && i<=0x82fff)){
				ch = 0x00;
			} else if (i==0x80000) {
				ch = 0x78;
			} else if (i==0x80001) {
				ch = 0x56;
			} else if (i==0x80002) {
				ch = 0xF3;
			} else if (i==0x80003) {
				ch = 0xC9;
			} else if (i==0x807fc) {
				ch = 0x49;
			} else if (i==0x807fd) {
				ch = 0x53;
			} else if (i==0x807fe) {
				ch = 0xF4;
			} else if (i==0x807ff) {
				ch = 0x7D;
			}
	    }
		if(!feof(fdin)) fputc(ch, fdout);
		if(ferror(fdout)) {
		  printf("Error writing output file.\n");
		  exit(1);
		}
		i++;
	}
	if(fclose(fdin)==EOF) {
		printf("Error closing copy file.\n");
		exit(1);
	}

	if(fclose(fdout)==EOF) {
		printf("Error closing output file.\n");
		exit(1);
	}

	return 0;
}
