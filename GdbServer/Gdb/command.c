/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2021 Uwe Bonnes
 *                            (bon@elektron.ikp.physik.tu-darmstadt.de)
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

/* This file implements a basic command interpreter for GDB 'monitor'
 * commands.
 */

#include "general.h"
#include "exception.h"
#include "command.h"
#include "gdb_packet.h"
#include "target.h"
#include "target_internal.h"
#include "morse.h"
#include "version.h"
#include "serialno.h"
#include "Windows.h"

#if 0
#ifdef PLATFORM_HAS_TRACESWO
#	include "traceswo.h"
#endif

typedef bool (*cmd_handler)(target *t, int argc, const char **argv);

static bool cmd_version(target *t, int argc, char **argv);
static bool cmd_help(target *t, int argc, char **argv);

static bool cmd_jtag_scan(target *t, int argc, char **argv);
static bool cmd_swdp_scan(target *t, int argc, char **argv);
static bool cmd_frequency(target *t, int argc, char **argv);
static bool cmd_targets(target *t, int argc, char **argv);
static bool cmd_morse(target *t, int argc, char **argv);
static bool cmd_halt_timeout(target *t, int argc, const char **argv);
static bool cmd_connect_srst(target *t, int argc, const char **argv);
static bool cmd_hard_srst(target *t, int argc, const char **argv);
#ifdef PLATFORM_HAS_POWER_SWITCH
static bool cmd_target_power(target *t, int argc, const char **argv);
#endif
#ifdef PLATFORM_HAS_TRACESWO
static bool cmd_traceswo(target *t, int argc, const char **argv);
#endif
static bool cmd_heapinfo(target *t, int argc, const char **argv);
#if defined(PLATFORM_HAS_DEBUG) && (PC_HOSTED == 0)
static bool cmd_debug_bmp(target *t, int argc, const char **argv);
#endif

const struct command_s cmd_list[] = {
	{"version", (cmd_handler)cmd_version, "Display firmware version info"},
	{"help", (cmd_handler)cmd_help, "Display help for monitor commands"},
	{"jtag_scan", (cmd_handler)cmd_jtag_scan, "Scan JTAG chain for devices" },
	{"swdp_scan", (cmd_handler)cmd_swdp_scan, "Scan SW-DP for devices" },
	{"frequency", (cmd_handler)cmd_frequency, "set minimum high and low times" },
	{"targets", (cmd_handler)cmd_targets, "Display list of available targets" },
	{"morse", (cmd_handler)cmd_morse, "Display morse error message" },
	{"halt_timeout", (cmd_handler)cmd_halt_timeout, "Timeout (ms) to wait until Cortex-M is halted: (Default 2000)" },
	{"connect_srst", (cmd_handler)cmd_connect_srst, "Configure connect under SRST: (enable|disable)" },
	{"hard_srst", (cmd_handler)cmd_hard_srst, "Force a pulse on the hard SRST line - disconnects target" },
#ifdef PLATFORM_HAS_POWER_SWITCH
	{"tpwr", (cmd_handler)cmd_target_power, "Supplies power to the target: (enable|disable)"},
#endif
#ifdef PLATFORM_HAS_TRACESWO
#if defined TRACESWO_PROTOCOL && TRACESWO_PROTOCOL == 2
	{"traceswo", (cmd_handler)cmd_traceswo, "Start trace capture, NRZ mode: (baudrate) (decode channel ...)" },
#else
	{"traceswo", (cmd_handler)cmd_traceswo, "Start trace capture, Manchester mode: (decode channel ...)" },
#endif
#endif
	{"heapinfo", (cmd_handler)cmd_heapinfo, "Set semihosting heapinfo" },
#if defined(PLATFORM_HAS_DEBUG) && (PC_HOSTED == 0)
	{"debug_bmp", (cmd_handler)cmd_debug_bmp, "Output BMP \"debug\" strings to the second vcom: (enable|disable)"},
#endif
	{NULL, NULL, NULL}
};

bool connect_assert_srst;
#if defined(PLATFORM_HAS_DEBUG) && (PC_HOSTED == 0)
bool debug_bmp;
#endif
unsigned cortexm_wait_timeout = 2000; /* Timeout to wait for Cortex to react on halt command. */
#endif

int command_process(target *t, char *cmd)
{
	int argc = 1;
	const char **argv;
	const char *part;

	/* Initial estimate for argc */
	for(char *s = cmd; *s; s++)
		if((*s == ' ') || (*s == '\t')) argc++;

	argv = _alloca(sizeof(const char *) * argc);

	/* Tokenize cmd to find argv */
	argc = 0;
	for (part = strtok(cmd, " \t"); part; part = strtok(NULL, " \t"))
		argv[argc++] = part;
	if (!t)
		return -1;
	return target_command(t, argc, argv);
}
