// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstdio/kprint/kprint.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <drivers/driver.h>
#include <lib/kstdio/kstdio.h>
#include <lib/kstdlib/kutoa.h>

// ? I think this is... safe?
void
    kputhex (kuint64_t n) {
	static const char *hex = "0123456789ABCDEF";
	char               buf[17];
	buf[16] = '\0';

	if (n == 0) {
		kputs("0");
		return;
	}

	for (int i = 15; i >= 0; --i) {
		buf[i] = hex[n & 0xF];
		n >>= 4;
	}

	int start = 0;
	while (start < 16 && buf[start] == '0') {
		start++;
	}

	kputs(&buf[start]);
}

void
    kputdec (kuint32_t n) {
	if (n == 0) {
		kputchar('0');
		return;
	}

	char buf[11];
	int  i = 0;

	while (n > 0) {
		buf[i++] = (char) ('0' + (n % 10));
		n /= 10;
	}

	while (--i >= 0) {
		kputchar(buf[i]);
	}
}

void
    kputs (const char *s) {
	video.puts(s);
	kputchar('\n');
}

int
    kvsnprintf (char *buffer, ksize_t size, const char *format, va_list args) {
	char *out = buffer;
	char *end = buffer + size - 1;

	for (const char *p = format; *p && out < end; ++p) {
		if (*p != '%') {
			*out++ = *p;
			continue;
		}
		++p;

		int left_align = 0;
		if (*p == '-') {
			left_align = 1;
			++p;
		}
		int width = 0;
		while (*p >= '0' && *p <= '9') {
			width = width * 10 + (*p++ - '0');
		}

		char temp[32];
		// Handle length modifier for long long ("ll")
		int long_long = 0;
		if (*p == 'l' && *(p + 1) == 'l') {
			long_long = 1;
			p += 2;  // Skip the two 'l's
		}

		char *t = temp;

		// Use long_long flag to choose value width
		switch (*p) {
			case 's': {
				const char *s = k_va_arg(args, const char *);
				while (*s && t < temp + sizeof(temp) - 1) {
					*t++ = *s++;
				}
				break;
			}
			case 'c': {
				*t++ = (char) k_va_arg(args, int);
				break;
			}
			case 'd': {
				if (long_long) {
					kint64_t  val = k_va_arg(args, kint64_t);
					kuint64_t uval;
					if (val < 0) {
						*t++ = '-';
						uval = (kuint64_t) (-val);
					} else {
						uval = (kuint64_t) val;
					}
					t = kutoa(t,
					          temp + sizeof(temp) - 1,
					          (unsigned long) uval,
					          10,
					          0);
				} else {
					int          val = k_va_arg(args, int);
					unsigned int uval;
					if (val < 0) {
						*t++ = '-';
						uval = (unsigned int) (-val);
					} else {
						uval = (unsigned int) val;
					}
					t = kutoa(
					    t, temp + sizeof(temp) - 1, uval, 10, 0);
				}
				break;
			}
			case 'u': {
				if (long_long) {
					kuint64_t uval = k_va_arg(args, kuint64_t);
					t              = kutoa(t,
                                                  temp + sizeof(temp) - 1,
                                                  (unsigned long) uval,
                                                  10,
                                                  0);
				} else {
					unsigned int uval = k_va_arg(args, unsigned int);
					t                 = kutoa(
                                            t, temp + sizeof(temp) - 1, uval, 10, 0);
				}
				break;
			}
			case 'x': {
				unsigned int uval = k_va_arg(args, unsigned int);
				t = kutoa(t, temp + sizeof(temp) - 1, uval, 16, 0);
				break;
			}
			case 'X': {
				unsigned int uval = k_va_arg(args, unsigned int);
				t = kutoa(t, temp + sizeof(temp) - 1, uval, 16, 1);
				break;
			}
			case '%': {
				*t++ = '%';
				break;
			}
			default: {
				*t++ = '%';
				*t++ = *p;
				break;
			}
		}
		++p;

		ksize_t len = (ksize_t) (t - temp);
		int     pad = width > (int) len ? width - (int) len : 0;
		if (!left_align) {
			while (pad-- > 0 && out < end)
				*out++ = ' ';
		}
		for (ksize_t i = 0; i < len && out < end; ++i) {
			*out++ = temp[i];
		}
		if (left_align) {
			while (pad-- > 0 && out < end)
				*out++ = ' ';
		}
		--p;
	}

	*out = '\0';
	return (int) (out - buffer);
}

int
    ksnprintf (char *buffer, ksize_t size, const char *format, ...) {
	va_list args;
	k_va_start(args, format);
	int result = kvsnprintf(buffer, size, format, args);
	k_va_end(args);
	return result;
}

int
    kprintf (const char *format, ...) {
	va_list args;
	k_va_start(args, format);
	int count = 0;

	for (const char *p = format; *p; ++p) {
		if (*p != '%') {
			kputchar(*p);
			count++;
			continue;
		}
		p++;

		// Field width and left alignment
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

		// Handle "ll" length modifier (for %llu)
		int long_long = 0;
		if (*p == 'l' && *(p + 1) == 'l') {
			long_long = 1;
			p += 2;
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
						kputchar(' ');
						count++;
					}
				}

				for (int i = 0; i < len; ++i) {
					kputchar(s[i]);
					count++;
				}

				if (left_align) {
					for (int i = 0; i < pad; ++i) {
						kputchar(' ');
						count++;
					}
				}
				break;
			}

			case 'd': {
				if (long_long) {
					kint64_t n = k_va_arg(args, kint64_t);
					if (n < 0) {
						kputchar('-');
						count++;
						n = -n;
					}
					char buf[21];
					int  idx = 20;
					buf[idx] = '\0';
					if (n == 0)
						buf[--idx] = '0';
					else {
						kuint64_t un = (kuint64_t) n;
						while (un) {
							buf[--idx] =
							    (char) ('0' + (un % 10));
							un /= 10;
						}
					}
					int len = 20 - idx;
					int pad = (width > len) ? width - len : 0;
					if (!left_align) {
						for (int i = 0; i < pad; ++i) {
							kputchar(' ');
							count++;
						}
					}
					for (int i = idx; i < 20; ++i) {
						kputchar(buf[i]);
						count++;
					}
					if (left_align) {
						for (int i = 0; i < pad; ++i) {
							kputchar(' ');
							count++;
						}
					}
				} else {
					int n = k_va_arg(args, int);
					if (n < 0) {
						kputchar('-');
						count++;
						n = -n;
					}
					kputdec((kuint32_t) n);
					int temp = n, digits = 1;
					while (temp >= 10) {
						temp /= 10;
						digits++;
					}
					count += digits;
				}
				break;
			}
			case 'x': {
				if (long_long) {
					kuint64_t n = k_va_arg(args, kuint64_t);
					kputhex(n);
					kuint64_t temp   = n;
					int       digits = 1;
					while (temp >= 16) {
						temp /= 16;
						digits++;
					}
					count += digits;
				} else {
					unsigned int n = k_va_arg(args, unsigned int);
					kputhex((kuint64_t) n);
					unsigned int temp   = n;
					int          digits = 1;
					while (temp >= 16) {
						temp /= 16;
						digits++;
					}
					count += digits;
				}
				break;
			}
			case 'c': {
				char c = (char) k_va_arg(args, int);
				kputchar(c);
				count++;
				break;
			}

			case '%': {
				kputchar('%');
				count++;
				break;
			}
			case 'u': {
				if (long_long) {
					kuint64_t n = k_va_arg(args, kuint64_t);
					char      buf[21];
					int       idx = 20;
					buf[idx]      = '\0';
					if (n == 0)
						buf[--idx] = '0';
					else {
						while (n) {
							buf[--idx] =
							    (char) ('0' + (n % 10));
							n /= 10;
						}
					}
					int len = 20 - idx;
					int pad = (width > len) ? width - len : 0;
					if (!left_align) {
						for (int i = 0; i < pad; ++i) {
							kputchar(' ');
							count++;
						}
					}
					for (int i = idx; i < 20; ++i) {
						kputchar(buf[i]);
						count++;
					}
					if (left_align) {
						for (int i = 0; i < pad; ++i) {
							kputchar(' ');
							count++;
						}
					}
				} else {
					unsigned int n = k_va_arg(args, unsigned int);
					kputdec(n);
					unsigned int temp   = n;
					int          digits = 1;
					while (temp >= 10) {
						temp /= 10;
						digits++;
					}
					count += digits;
				}
				break;
			}
			default:
				kputchar('%');
				kputchar(*p);
				count += 2;
				break;
		}
	}

	k_va_end(args);
	return count;
}

void
    kputchar (int c) {
	video.put_char((char) c);
}
