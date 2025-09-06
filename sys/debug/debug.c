// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/debug/debug.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <debug/debug.h>
#include <drivers/serial/serial.h>
#include <lib/kstdio/kstdbool.h>
#include <lib/kstdio/kstdio.h>
#include <lib/kstdlib/kutoa.h>

kbool              d_enabled          = kfalse;
static const char *debug_type_message = "[    debug] ";

void
    d_puts (const char *s) {
	if (!d_enabled) {
		return;
	}
	if (!s || *s == '\0')
		return;
	serial.writes(debug_type_message);
	serial.writes(s);
	serial.write('\n');
}

int
    d_printf (const char *format, ...) {
	if (!d_enabled)
		return 0;

	if (!format || *format == '\0')
		return 1;

	va_list args;
	k_va_start(args, format);
	int count = 0;

	serial.writes(debug_type_message);
	count += (int) kstrlen(debug_type_message);

	for (const char *p = format; *p; ++p) {
		if (*p != '%') {
			serial.write(*p);
			count++;
			continue;
		}

		p++;

		int left_align = 0;
		int width      = 0;

		if (*p == '-') {
			left_align = 1;
			p++;
		}

		while (*p >= '0' && *p <= '9') {
			width = width * 10 + (*p - '0');
			p++;
		}

		switch (*p) {
			case 's': {
				const char *s   = k_va_arg(args, const char *);
				int         len = 0;
				const char *t   = s;
				while (*t++)
					len++;

				int pad = (width > len) ? (width - len) : 0;

				if (!left_align) {
					for (int i = 0; i < pad; ++i) {
						serial.write(' ');
						count++;
					}
				}

				serial.writes(s);
				count += len;

				if (left_align) {
					for (int i = 0; i < pad; ++i) {
						serial.write(' ');
						count++;
					}
				}
				break;
			}

			case 'd': {
				int   n = k_va_arg(args, int);
				char  buf[12];
				char *ptr = buf;

				if (n < 0) {
					*ptr++ = '-';
					n      = -n;
				}

				char *end_ptr = kutoa(
				    ptr, buf + sizeof(buf) - 1, (unsigned int) n, 10, 0);
				*end_ptr = '\0';
				serial.writes(buf);
				count += (int) (end_ptr - buf);
				break;
			}

			case 'x': {
				unsigned int n = k_va_arg(args, unsigned int);
				char         buf[12];
				char        *end_ptr =
				    kutoa(buf, buf + sizeof(buf) - 1, n, 16, 0);
				*end_ptr = '\0';
				serial.writes(buf);
				count += (int) (end_ptr - buf);
				break;
			}
			case 'l': {
				// Handle long/long long modifiers
				if (*(p + 1) == 'l') {
					p++;  // Skip second 'l'
					if (*(p + 1) == 'x') {
						p++;  // Skip 'x'
						kuint64_t n = k_va_arg(args, kuint64_t);
						char      buf[20];
						char     *end_ptr =
						    kutoa(buf,
						          buf + sizeof(buf) - 1,
						          (unsigned int) (n & 0xFFFFFFFF),
						          16,
						          0);
						*end_ptr = '\0';
						serial.writes(buf);
						count += (int) (end_ptr - buf);
						break;
					}
				}
				// Fall through to default for single 'l'
				goto default_case;
			}

			case 'c': {
				char c = (char) k_va_arg(args, int);
				serial.write(c);
				count++;
				break;
			}

			case '%': {
				serial.write('%');
				count++;
				break;
			}

			default:
			default_case: {
				serial.write('%');
				serial.write(*p);
				count += 2;
				break;
			}
		}
	}

	k_va_end(args);
	return count;
}

static void
    d_dbgtype (const char *type,
               const char *subsystem,
               const char *message,
               const char *extra) {
	if (!message || *message == '\0')
		return;

	kprintf("[    %s", type);

	if (subsystem && *subsystem != '\0')
		kprintf("::%s", subsystem);

	kprintf("] %s", message);

	if (extra && *extra != '\0')
		kprintf(": %s", extra);

	kprintf("\n");
}

static void
    d_crit (const char *m, const char *s, const char *e) {
	d_dbgtype("crit", s, m, e);
}

static void
    d_alert (const char *m, const char *s, const char *e) {
	d_dbgtype("alert", s, m, e);
}

static void
    d_emerg (const char *m, const char *s, const char *e) {
	d_dbgtype("emerg", s, m, e);
}

static void
    d_warn (const char *m, const char *s, const char *e) {
	d_dbgtype("warn", s, m, e);
}

static void
    d_err (const char *m, const char *s, const char *e) {
	d_dbgtype("err", s, m, e);
}

static void
    d_notice (const char *m, const char *s, const char *e) {
	d_dbgtype("notice", s, m, e);
}

static void
    d_info (const char *m, const char *s, const char *e) {
	d_dbgtype("info", s, m, e);
}

struct Debug debug = {.crit   = d_crit,
                      .alert  = d_alert,
                      .emerg  = d_emerg,
                      .warn   = d_warn,
                      .err    = d_err,
                      .notice = d_notice,
                      .info   = d_info,
                      .puts   = d_puts,
                      .printf = d_printf};
