// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstdlib/kitoa.c
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
#include <lib/kstdlib/kitoa.h>
#include <lib/kstdlib/kstdlib.h>

char *
    kitoa (char *buf, char *end, long value, int base, int is_uppercase) {
	if (base <= 1) {
		if (buf < end)
			*buf = '\0';
		return buf;
	}
	const char *digit_set = is_uppercase ? kdigits_upper : kdigits_lower;
	char       *rev       = buf;
	int         negative  = 0;

	// Ensure we have at least 2 bytes for null terminator
	if (buf >= end) {
		if (buf < end)
			*buf = '\0';
		return buf;
	}

	if (value == 0) {
		*rev++ = '0';
		*rev   = '\0';
		return rev;
	}

	if (value < 0) {
		negative = 1;
		value    = -value;
	}

	while (value != 0 && rev < end - 1) {
		*rev++ = digit_set[(unsigned) (value % (unsigned) base)];
		value /= (unsigned) base;
	}

	if (negative && rev < end - 1)
		*rev++ = '-';

	// Reverse string
	for (char *p1 = buf, *p2 = rev - 1; p1 < p2; p1++, p2--) {
		char tmp = *p1;
		*p1      = *p2;
		*p2      = tmp;
	}

	return rev;
}
