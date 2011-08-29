/*
    Copyright (C) 2011  Guhl
    This work is very much based on the module wpthis by scotty2

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

#ifndef GFMOD_H_
#define GFMOD_H_

#define MOD_RET_OK       -ENOSYS
#define MOD_RET_FAILINIT -ENOTEMPTY
#define MOD_RET_FAILWP   -ELOOP
#define MOD_RET_FAIL     -ENOMSG
#define MOD_RET_NONEED   -EXFULL

#define GFMOD_DODEBUG 1
#if GFMOD_DODEBUG
#define ONDEBUG(x) x
#else
#define ONDEBUG(x)
#endif

#define dmesg printk("gfmod: "); printk

#define get_or_die(target, source)													\
    target = source;																\
    if(!target)																		\
    {																				\
	dmesg("%s = %s == null.\n", #source, #target);									\
	goto out;																		\
    }																				\
    else if(IS_ERR(target))															\
    {																				\
	dmesg("%s gave us an error for %s: %lu\n", #source, #target, PTR_ERR(target));	\
	goto out;																		\
    }





#define print_dev(device) \
    dmesg("%s: 0x%.8x (%s)\n", #device, (unsigned int)device, dev_name(device))

#define print_clock(clock) \
    dmesg("%s: 0x%.8x (%lu)\n", #clock, (unsigned int)clock, clk_get_rate(clock));

#define assert(eval)                               \
    if(!(eval))                                    \
    {                                              \
	dmesg("assertion failed! !(%s)\n", #eval); \
	goto out;                                  \
    }


#endif /* GFMOD_H_ */
