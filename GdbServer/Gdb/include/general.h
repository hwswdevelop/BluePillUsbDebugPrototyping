/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GENERAL_H
#define __GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE
#if !defined(__USE_MINGW_ANSI_STDIO)
# define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <sys/types.h>

#include "platform.h"
#include "platform_support.h"

	extern uint32_t delay_cnt;

	enum BMP_DEBUG {
		BMP_DEBUG_NONE = 0,
		BMP_DEBUG_INFO = 1,
		BMP_DEBUG_GDB = 2,
		BMP_DEBUG_TARGET = 4,
		BMP_DEBUG_PROBE = 8,
		BMP_DEBUG_WIRE = 0x10,
		BMP_DEBUG_MAX = 0x20,
		BMP_DEBUG_STDOUT = 0x8000,
	};

#define FREQ_FIXED 0xffffffff

#if PC_HOSTED == 0
	/* For BMP debug output on a firmware BMP platform, using
	 * BMP PC-Hosted is the preferred way. Printing DEBUG_WARN
	 * and DEBUG_INFO is kept for comptibiluty.
	 */
# if defined(ENABLE_DEBUG)
#  define DEBUG_WARN printf
#  define DEBUG_INFO printf
# else
#  define DEBUG_WARN(...)
#  define DEBUG_INFO(...)
# endif
# define DEBUG_GDB(...)
# define DEBUG_TARGET(...)
# define DEBUG_PROBE(...)
# define DEBUG_WIRE(...)
# define DEBUG_GDB_WIRE(...)
#else
# include <stdarg.h>
	extern int cl_debuglevel;

#if 0
#define DEBUG_WARN printf
#define	DEBUG_INFO printf
#define	DEBUG_GDB printf
#define	DEBUG_GDB_WIRE printf
#define	DEBUG_TARGET printf
#define	DEBUG_PROBE printf
#define	DEBUG_WIRE printf
#else
#  define DEBUG_WARN(...)
#  define DEBUG_INFO printf
#  define DEBUG_WARN(...)
#  define DEBUG_INFO(...)
# define DEBUG_GDB printf
# define DEBUG_TARGET printf
# define DEBUG_PROBE(...)
# define DEBUG_WIRE(...)
# define DEBUG_GDB_WIRE(...)

#endif

/*
	static inline void DEBUG_WARN(const char *format, ...)
	{
		return;
	}
	

	static inline void DEBUG_INFO(const char* format, ...)
	{
		return;
	}

	static inline void DEBUG_GDB(const char* format, ...)
	{
		return;
	}

	static inline void DEBUG_GDB_WIRE(const char* format, ...)
	{
		return;
	}

	static inline void DEBUG_TARGET(const char* format, ...)
	{
		return;
	}

	static inline void DEBUG_PROBE(const char* format, ...)
	{
		return;
	}

	static inline void DEBUG_WIRE(const char* format, ...)
	{
		return;
	}
*/
#endif

#define ALIGN(x, n) (((x) + (n) - 1) & ~((n) - 1))
#undef MIN
#define MIN(x, y)  (((x) < (y)) ? (x) : (y))
#undef MAX
#define MAX(x, y)  (((x) > (y)) ? (x) : (y))


#ifdef __cplusplus
}
#endif


#endif

