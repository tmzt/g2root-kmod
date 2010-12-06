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

#ifndef SOFF_VERIFY_H_
#define SOFF_VERIFY_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MODEMDEVICE "/dev/ttyUSB2"
#define BAUDRATE B38400

int fd_radio;
struct termios oldtio,newtio;

char *squeeze(char *str);
int verify_init_device();
int verify_init_modem();
int verify_set_verbose();
int verify_secu_flag();
void verify_cid();
int verify_simlock();
int verify_close_device();

#endif /* SOFF_VERIFY_H_ */
