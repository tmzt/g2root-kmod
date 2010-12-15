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
#include "soff_verify.h"

#define VERSION_A	0
#define VERSION_B	01

int main() {
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
