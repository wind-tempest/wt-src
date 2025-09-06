// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/shell/shell.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "shell.h"

#include <debug/debug.h>
#include <drivers/driver.h>
#include <fs/ext2/ext2.h>
#include <fs/vfs/vfs.h>
#include <kern/framebuf/framebuf.h>
#include <kern/memory/memory.h>
#include <kern/panic/panic.h>
#include <lib/kgeneral/kerrno/kerrno.h>
#include <lib/kstdio/kstdio.h>
#include <lib/kstdlib/kstdlib.h>
#include <lib/kstring/kstring.h>
#include <lib/ktime/ktime.h>
#include <lib/kunistd/kunistd.h>

#define CMD_BUFFER_SIZE 512
#define MAX_HISTORY     128
static char command_history[MAX_HISTORY][CMD_BUFFER_SIZE];
static int  history_count  = 0;
int         input_overflow = 0;

// Command handler function type
typedef void (*command_func_t)(const char *args);

// Forward declarations for handler functions
static void
    cmd_help (const char *args);
static void
    cmd_clear (const char *args);
static void
    cmd_echo (const char *args);
static void
    cmd_poweroff (const char *args);
static void
    cmd_reboot (const char *args);
static void
    cmd_fetch (const char *args);
static void
    cmd_sleep (const char *args);
static void
    cmd_ls (const char *args);
static void
    cmd_cat (const char *args);
static void
    cmd_fsize (const char *args);
static void
    cmd_pwd (const char *args);
static void
    cmd_history (const char *args);
static void
    cmd_test_graphics (const char *args);
static void
    cmd_time (const char *args);
static void
    cmd_panic (const char *args);
static void
    cmd_cd (const char *args);

// Command table with handler functions
static struct Command {
	const char    *name;
	const char    *description;
	const char    *category;
	command_func_t handler;
} commands[] = {
    // System commands
    {"help", "Show this help message", "System", cmd_help},
    {"clear", "Clear the screen", "System", cmd_clear},
    {"echo", "Echo a message", "System", cmd_echo},
    {"history", "Show the history of commands", "System", cmd_history},

    // System control
    {"poweroff", "Power off the OS", "Control", cmd_poweroff},
    {"reboot", "Reboot the OS", "Control", cmd_reboot},
    {"kpanic", "Test kernel kpanic (DANGEROUS!)", "Control", cmd_panic},

    // Information commands
    {"fetch", "View system information", "Info", cmd_fetch},
    {"time", "Show current date and time", "Info", cmd_time},

    // Graphics testing
    {"test_graphics", "Test the graphics driver", "Graphics", cmd_test_graphics},

    // Filesystem commands
    {"ls", "List directory", "FS", cmd_ls},
    {"cat", "Read file from filesystem", "FS", cmd_cat},
    {"fsize", "Show file size", "FS", cmd_fsize},
    {"cd", "Change current directory", "FS", cmd_cd},
    {"pwd", "Print current directory", "FS", cmd_pwd},

    // Hardware testing
    {"sleep", "Test the HPET timer", "Hardware", cmd_sleep},
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

static void
    handle_command (char *cmd) {
	// Split command and arguments
	char *space = cmd;
	while (*space && *space != ' ')
		++space;
	char *args = KNULL;
	if (*space == ' ') {
		*space = '\0';
		args   = space + 1;
		while (*args == ' ')
			++args;  // skip extra spaces
	}
	for (ksize_t i = 0; i < NUM_COMMANDS; ++i) {
		if (kstrcmp(cmd, commands[i].name) == 0) {
			commands[i].handler(args);
			return;
		}
	}
	kprintf("Unknown command: '%s'\n", cmd);
}

void
    kshell (void) {
	char cmd_buffer[CMD_BUFFER_SIZE];
	int  cmd_ptr = 0;

	kputs("Copyright (c) 2025, Russian95");
	kputs("Type 'help' for a list of commands.");

	while (ktrue) {
		kprintf(
		    "\n$[kern] ");  // use kernel for determining it's kernel space (AKA: ring 0)
		cmd_ptr        = 0;
		input_overflow = 0;

		while (ktrue) {
			char c = (char) getchar();

			if (c == '\n') {
				kputchar('\n');
				if (input_overflow) {
					kputs(
					    "Error: "
					    "command too "
					    "long.");
				} else {
					cmd_buffer[cmd_ptr] = '\0';
					handle_command(cmd_buffer);

					if (history_count < MAX_HISTORY) {
						ksize_t len =
						    (ksize_t) kstrlen(cmd_buffer);
						if (len >= CMD_BUFFER_SIZE)
							len = CMD_BUFFER_SIZE - 1;
						kmemcpy(command_history[history_count],
						        cmd_buffer,
						        len);
						command_history[history_count][len] =
						    '\0';
						history_count++;
					}
				}
				break;
			}

			else if (c == '\b') {
				if (cmd_ptr > 0) {
					cmd_ptr--;
					kputchar('\b');
				}
			}

			else {
				if (cmd_ptr < CMD_BUFFER_SIZE - 1) {
					cmd_buffer[cmd_ptr++] = c;
					kputchar(c);
				} else {
					input_overflow = 1;
					kputchar('\a');
				}
			}
		}
	}
}

static void
    cmd_clear (const char *args) {
	kuint32_t color = 0x000000;

	if (args != KNULL && *args != '\0') {
		int base = 0;
		if (args[0] == '#') {
			args++;
			base = 16;
		}

		kerrno = 0;
		char *end;
		long  val = kstrtol(args, &end, base);

		if (kerrno == 0 && end != args && val >= 0 && val <= 0xFFFFFF) {
			color = (kuint32_t) val;
		}
	}
	video.clear(color);
}

static void
    cmd_help (const char *args) {
	(void) args;
	kputs("Available commands:");

	// Get unique categories
	const char *categories[10];
	int         num_categories = 0;

	for (ksize_t i = 0; i < NUM_COMMANDS; ++i) {
		int found = 0;
		for (int j = 0; j < num_categories; ++j) {
			if (kstrcmp(commands[i].category, categories[j]) == 0) {
				found = 1;
				break;
			}
		}
		if (!found) {
			categories[num_categories++] = commands[i].category;
		}
	}

	// Display commands by category
	for (int cat = 0; cat < num_categories; ++cat) {
		kprintf("\n[%s]\n", categories[cat]);
		for (ksize_t i = 0; i < NUM_COMMANDS; ++i) {
			if (kstrcmp(commands[i].category, categories[cat]) == 0) {
				kprintf("  %-12s - %s\n",
				        commands[i].name,
				        commands[i].description);
			}
		}
	}
}

static void
    cmd_echo (const char *args) {
	if (args && *args) {
		kputs(args);
	} else {
		kputs("Echo...");
		kputs("Use: echo <your message>");
	}
}

static void
    cmd_poweroff (const char *args) {
	(void) args;
	acpi.poweroff();
}

static void
    cmd_reboot (const char *args) {
	(void) args;
	acpi.reboot();
}

static void
    cmd_fetch (const char *args) {
	(void) args;
	const char *ascii[] = {"@                         @",
	                       " @@           @       @@@@",
	                       "     @@@@@@@           ",
	                       "@                        @@",
	                       "   @@@@@@             @ ",
	                       "            @@@@@@@@     ",
	                       "@                         @",
	                       " @     @@@@@@@@@@      @@ "};
	char        info[8][96];
	ksnprintf(info[0], sizeof(info[0]), "os:     wind");
	ksnprintf(info[1], sizeof(info[1]), "kernel: tempest");
	extern char cpu_brand_string[49];
	ksnprintf(info[2], sizeof(info[2]), "cpu:    %s", cpu_brand_string);
	if (fb_info.width && fb_info.height && fb_info.bpp) {
		ksnprintf(info[3],
		          sizeof(info[3]),
		          "resolution: %ux%u %ubpp",
		          fb_info.width,
		          fb_info.height,
		          (unsigned int) fb_info.bpp);
	} else {
		ksnprintf(info[3], sizeof(info[3]), "resolution: unknown");
	}
	memory_stats_t stats    = memory_get_stats();
	kuint64_t      total_kb = stats.total_physical_pages * 4096 / 1024;
	kuint64_t      used_kb  = stats.used_physical_pages * 4096 / 1024;
	kuint64_t      free_kb  = stats.free_physical_pages * 4096 / 1024;
	kuint64_t      total_mb = total_kb / 1024;
	kuint64_t      used_mb  = used_kb / 1024;
	kuint64_t      free_mb  = free_kb / 1024;
	if (total_mb >= 1) {
		ksnprintf(info[4],
		          sizeof(info[4]),
		          "memory: %llu MB used / %llu MB total (%llu MB free)",
		          used_mb,
		          total_mb,
		          free_mb);
	} else {
		ksnprintf(info[4],
		          sizeof(info[4]),
		          "memory: %llu kB used / %llu kB total (%llu kB free)",
		          used_kb,
		          total_kb,
		          free_kb);
	}
	info[5][0] = '\0';
	info[6][0] = '\0';
	info[7][0] = '\0';

	for (int i = 0; i < 8; ++i) {
		if (info[i][0])
			kprintf("%-28s  %s\n", ascii[i], info[i]);
		else
			kprintf("%s\n", ascii[i]);
	}
}

static void
    cmd_sleep (const char *args) {
	if (args && *args) {
		ksleep(katoi(args));
	} else {
		kputs("Zzzzzzzzzzzzzzzzzzz...");
		kputs("Use: sleep <time in milliseconds>");
	}
}

static void
    ls_print_cb (const char *name, kuint8_t file_type) {
	(void) file_type;
	kputs(name);
}

static void
    list_dir_path (const char *path) {
	int rc = ext2.list(path, ls_print_cb);
	if (rc != 0) {
		kprintf("ls: cannot access %s (err %d)\n", path, rc);
	}
}

static void
    cmd_ls (const char *args) {
	// If no path given use current working directory
	const char *path = (args && *args) ? args : KNULL;
	char        buf[256];
	if (!path) {
		vfs.getcwd(buf, sizeof(buf));
		path = buf;
	} else if (path[0] != '/') {
		vfs.resolve(path, buf, sizeof(buf));
		path = buf;
	}
	list_dir_path(path);
}

// Change current working directory
static void
    cmd_cd (const char *args) {
	const char *path = (args && *args) ? args : "/";
	int         rc   = vfs.chdir(path);
	if (rc != 0) {
		kprintf("cd: cannot access %s (err %d)\n", path, rc);
	}
}

static void
    cmd_cat (const char *args) {
	if (!args || *args == '\0') {
		kputs("Usage: cat <path>");
		return;
	}
	char        abs_path[256];
	const char *path = args;
	if (path[0] != '/') {
		vfs.resolve(path, abs_path, sizeof(abs_path));
		path = abs_path;
	}
	ext2_file_t file;
	int         rc = ext2.open(path, &file);
	if (rc != 0) {
		kprintf("cat: cannot open %s (err %d)\n", path, rc);
		return;
	}
	char buf[512];
	int  read;
	while ((read = ext2.read(&file, buf, sizeof(buf) - 1)) > 0) {
		buf[read] = '\0';
		kputs(buf);
	}
}

static void
    cmd_fsize (const char *args) {
	if (!args || *args == '\0') {
		kputs("Usage: fsize <path>");
		return;
	}
	char        abs_path[256];
	const char *path = args;
	if (path[0] != '/') {
		vfs.resolve(path, abs_path, sizeof(abs_path));
		path = abs_path;
	}
	ext2_file_t file;
	int         rc = ext2.open(path, &file);
	if (rc != 0) {
		kprintf("fsize: cannot open %s (err %d)\n", path, rc);
		return;
	}
	kuint64_t size =
	    (((kuint64_t) file.inode.dir_acl_or_size_high) << 32) | file.inode.size_lo;
	kprintf("%s: %llu bytes\n", args, size);
}

static void
    cmd_pwd (const char *args) {
	(void) args;
	char buf[256];
	vfs.getcwd(buf, sizeof(buf));
	kputs(buf);
}

static void
    cmd_history (const char *args) {
	(void) args;

	if (history_count == 0) {
		kputs("No commands in history.");
		return;
	}

	for (int i = 0; i < history_count; i++) {
		kprintf("%2d - %s\n", i + 1, command_history[i]);
	}
}

static void
    cmd_test_graphics (const char *args) {
	(void) args;

	// Safety check for division by zero
	if (!video.is_ready()) {
		return;
	}

	video.clear(0xFFFFFF);

	kuint8_t  circle_diff = k_u_rand32() & 0xFF;
	kuint32_t circle_x    = fb_info.width / 2;
	kuint32_t circle_y    = fb_info.height / 2;
	kuint32_t color       = k_u_rand32() & 0xFFFFFF;
	video.draw_circle((int) circle_x, (int) circle_y, 100, color);

	// First square: left
	color               = k_u_rand32() & 0xFFFFFF;
	kuint32_t square1_x = circle_x - circle_diff;
	kuint32_t square1_y = circle_y;
	video.draw_square((int) square1_x, (int) square1_y, 100, color);

	// Second square: right
	color               = k_u_rand32() & 0xFFFFFF;
	kuint32_t square2_x = circle_x + circle_diff;
	kuint32_t square2_y = circle_y;
	video.draw_square((int) square2_x, (int) square2_y, 100, color);

	ksleep(5000);

	video.clear(0x000000);
}

static void
    cmd_time (const char *args) {
	(void) args;

	// Buffer for date and time strings
	char date_buffer[16];
	char time_buffer[16];

	// Retrieve date and time strings
	kget_date_string(date_buffer, sizeof(date_buffer));
	kget_time_string(time_buffer, sizeof(time_buffer));

	kprintf("Date: %s\n", date_buffer);
	kprintf("Time: %s\n", time_buffer);
}

static void
    cmd_panic (const char *args) {
	if (args == KNULL || *args == '\0') {
		kputs("Usage: kpanic <error_code>");
		kputs("Error codes: 0-16 (0=unknown, 1=div_by_zero, etc.)");
		return;
	}

	int code = katoi(args);
	if (code < 0 || code > 16) {
		kputs("Error code must be between 0 and 16");
		return;
	}

	panic.main(code, KNULL);
}
