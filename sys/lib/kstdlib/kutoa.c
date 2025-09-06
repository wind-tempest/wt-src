// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstdlib/kutoa.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kgeneral/kcharset/kcharset.h>
#include <lib/kstdlib/kstdlib.h>
#include <lib/kstdlib/kutoa.h>

char *
    kutoa (char *buf, char *end, unsigned long value, int base, int is_uppercase) {
	if (base <= 1) {
		if (buf < end)
			*buf = '\0';
		return buf;
	}
	const char *digit_set = is_uppercase ? kdigits_upper : kdigits_lower;
	char       *rev       = buf;

	if (value == 0) {
		if (buf < end) {
			*rev++ = '0';
		}
		return rev;
	}

	while (value != 0 && rev < end) {
		*rev++ = digit_set[(unsigned) (value % (unsigned) base)];
		value /= (unsigned) base;
	}

	// Reverse string
	for (char *p1 = buf, *p2 = rev - 1; p1 < p2; p1++, p2--) {
		char tmp = *p1;
		*p1      = *p2;
		*p2      = tmp;
	}

	return rev;
}
