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

#include "soff_verify.h"

int verify_init_device(){
	fd_radio = 0;
	fd_radio = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
	if (fd_radio <0) {
		fprintf( stderr, "Error: could not open modem device: %s\n",MODEMDEVICE);
		return -1;
	}
    tcgetattr(fd_radio,&oldtio); 			// save current serial port settings
    memset(&newtio, 0x00, sizeof(newtio)); // clear struct for new port settings

    //get the current options
    tcgetattr(fd_radio, &newtio);

	// set raw input, 1 second timeout
    newtio.c_cflag     |= (CLOCAL | CREAD);
    newtio.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG );
    newtio.c_oflag     &= ~OPOST;
    newtio.c_cc[VMIN]  = 0;
    newtio.c_cc[VTIME] = 10;

    // clean the modem line and activate the settings for the port
    tcflush(fd_radio, TCIFLUSH);
    tcsetattr(fd_radio,TCSANOW,&newtio);

    return 0;
}

int verify_init_modem() {
    char buffer[255];  // Input buffer
    char *bufptr;      // Current char in buffer
    int  nbytes;       // Number of bytes read
    int  tries;        // Number of tries so far

    for (tries = 0; tries < 3; tries ++) {
    	// send an AT command followed by a CR
    	if (write(fd_radio, "AT\r", 3) < 3)
    		continue;
    	// read characters into our string buffer until we get a CR or NL
    	bufptr = buffer;
		while ((nbytes = read(fd_radio, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
			bufptr += nbytes;
			if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
				break;
		}
		// nul terminate the string and see if we got an OK response */
		*bufptr = '\0';
		if ((strncmp(buffer, "0", 1) == 0) || (strncmp(buffer, "\r\nOK", 4) == 0))
			return (0);
	}
    return (-1);
}

int verify_set_verbose() {
    char buffer[255];  // Input buffer
    char *bufptr;      // Current char in buffer
    int  nbytes;       // Number of bytes read
    int  tries;        // Number of tries so far

    for (tries = 0; tries < 3; tries ++) {
    	// send an ATV1 command followed by a CR
    	if (write(fd_radio, "ATV1\r", 5) < 5)
    		continue;
    	// read characters into our string buffer until we get a CR or NL
    	bufptr = buffer;
		while ((nbytes = read(fd_radio, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
			bufptr += nbytes;
			if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
				break;
		}
		// nul terminate the string and see if we got an OK response */
		*bufptr = '\0';
		if (strncmp(buffer, "\r\nOK", 4) == 0)
			return (0);
	}
    return (-1);
}

int verify_secu_flag() {
    char buffer[255];  // Input buffer
    char *bufptr;      // Current char in buffer
    int  nbytes;       // Number of bytes read
    int  tries;        // Number of tries so far

    for (tries = 0; tries < 3; tries ++) {
    	// send an ATV1 command followed by a CR
    	if (write(fd_radio, "AT@SIMLOCK?AA\r", 15) < 15)
    		continue;
    	// read characters into our string buffer until we get a CR or NL
    	bufptr = buffer;
		while ((nbytes = read(fd_radio, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
			bufptr += nbytes;
			if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
				break;
		}
		// nul terminate the string and see if we got an OK response */
		*bufptr = '\0';
		fprintf( stdout, "gfree verify_secu_flag returned: %s\n",buffer);
		if (strncmp(buffer, "\r\n@secu_flag: 0", 15) == 0)
			return (0);
	}
    return (-1);
}

void verify_cid() {
    char buffer[255];  // Input buffer
    char *bufptr;      // Current char in buffer
    int  nbytes;       // Number of bytes read
    int  tries;        // Number of tries so far

    for (tries = 0; tries < 3; tries ++) {
    	// send an ATV1 command followed by a CR
    	if (write(fd_radio, "AT@CID?\r", 8) < 8)
    		continue;
    	// read characters into our string buffer until we get a CR or NL
    	bufptr = buffer;
		while ((nbytes = read(fd_radio, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
			bufptr += nbytes;
			if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
				break;
		}
		// nul terminate the string and see if we got an OK response */
		*bufptr = '\0';
		fprintf( stdout, "gfree verify_cid returned: %s\n",buffer);
		return;
	}
}

int verify_simlock() {
    char buffer[255];  // Input buffer
    char *bufptr;      // Current char in buffer
    int  nbytes;       // Number of bytes read
    int  tries;        // Number of tries so far

    for (tries = 0; tries < 3; tries ++) {
    	// send an ATV1 command followed by a CR
    	if (write(fd_radio, "AT@SIMLOCK?40\r", 15) < 15)
    		continue;
    	// read characters into our string buffer until we get a CR or NL
    	bufptr = buffer;
		while ((nbytes = read(fd_radio, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
			bufptr += nbytes;
			if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
				break;
		}
		// nul terminate the string and see if we got an OK response */
		*bufptr = '\0';
		fprintf( stdout, "gfree verify_simlock returned: %s\n",buffer);
		if (strncmp(buffer, "\r\n@SIMLOCK= 00", 14) == 0)
			return (0);
	}
    return (-1);
}

int verify_close_device() {
	// restore the old port settings
	tcsetattr(fd_radio,TCSANOW,&oldtio);
	close(fd_radio);
	return 0;
}
