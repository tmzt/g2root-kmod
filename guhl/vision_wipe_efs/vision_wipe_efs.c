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
#include <time.h>
#include <string.h>

#define VERSION_A	0
#define VERSION_B	1

#define EFSFILE1 "/dev/block/mmcblk0p13"
#define EFSFILE2 "/dev/block/mmcblk0p14"
#define BACKUPFILE1 "/sdcard/part13backup-%lu.bin"
#define BACKUPFILE2 "/sdcard/part14backup-%lu.bin"

int wipePartition(char* pPartition, int size);
int backupPartition(char* pPartition, char* pBackupFile);

int main(int argc, const char **argv) {
    char *backupFile;
    time_t ourTime;

    ourTime = time(0);
    backupFile = malloc(snprintf(0, 0, BACKUPFILE1, ourTime) + 1);
    sprintf(backupFile, BACKUPFILE1, ourTime);
	if (backupPartition(EFSFILE1,backupFile)!=0)
		exit(1);
    sprintf(backupFile, BACKUPFILE2, ourTime);
	if (backupPartition(EFSFILE2,backupFile)!=0)
		exit(2);
	if (wipePartition(EFSFILE1, 0x300000)!=0)
		exit(3);
	if (wipePartition(EFSFILE2, 0x300000)!=0)
		exit(4);
    printf("Successfully wiped partitions 13 and 14.");
	exit(0);	
}

int wipePartition(char* pPartition, int size){
    FILE *fdout;
    
    printf("Wiping partition %s...\n", pPartition);
	fdout = fopen(pPartition, "wb");
	if (fdout == NULL){
		printf("Error opening output file.\n");
		return -1;
	}

    char ch = 0x00;
    int i = 0;
	while (i<size){
		fputc(ch, fdout);
		if(ferror(fdout)) {
		  printf("Error writing output file.\n");
	      return 1;
		}
		i++;
	}

	if(fclose(fdout)==EOF) {
		printf("Error closing output file.\n");
	    return 1;
	}

	return 0;
}

int backupPartition(char* pPartition, char* pBackupFile){
    FILE *fdin, *fdout;
    char ch;

    printf("Backing up partition %s to %s ...\n", pPartition, pBackupFile);
	fdin = fopen(pPartition, "rb");
	if (fdin == NULL){
		printf("Error opening input file.\n");
		return -1;
	}

	fdout = fopen(pBackupFile, "wb");
	if (fdout == NULL){
		printf("Error opening backup file.\n");
		return -1;
	}

//  create a copy of the partition
	while(!feof(fdin)) {
	    ch = fgetc(fdin);
	    if(ferror(fdin)) {
	      printf("Error reading input file.\n");
	      return 1;
	    }
	    if(!feof(fdin)) fputc(ch, fdout);
	    if(ferror(fdout)) {
	      printf("Error writing backup file.\n");
	      return 1;
	    }
	}
	if(fclose(fdin)==EOF) {
		printf("Error closing input file.\n");
	    return 1;
	}

	if(fclose(fdout)==EOF) {
		printf("Error closing backup file.\n");
	    return 1;
	}
	return 0;
}
