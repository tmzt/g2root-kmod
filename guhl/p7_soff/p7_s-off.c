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

#define INFILE "/dev/block/mmcblk0p7"
#define CPYFILE "/sdcard/mmcblk0p7.ori.img"
#define OUTFILE "/dev/block/mmcblk0p7"

int main(void) {
	FILE *fdin, *fdout;
	char ch;

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
	    if (i==0xa00 || (i>0x80003 && i<0x807fc) || (i>=0x80800 && i<=0x82fff)){
	    	ch = 0x00;
	    } else if ((i>=0x200 && i<=0x207)) {
	    	ch = 0x31;
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
