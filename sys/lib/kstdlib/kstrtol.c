// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstdlib/kstrtol.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kgeneral/kctype/kctype.h>
#include <lib/kgeneral/kerrno/kerrno.h>
#include <lib/klimits.h>
#include <lib/kstdlib/kstdlib.h>
#include <lib/kstdlib/kstrtol.h>

long
    kstrtol (const char *nptr, char **endptr, int base) {
	const char *s   = nptr;
	long        acc = 0;
	int         c;
	int         neg = 0;
	long        cutoff;
	int         cutlim;

	// skip whitespace
	while (kisspace((unsigned char) *s))
		s++;

	// sign
	if (*s == '-') {
		neg = 1;
		s++;
	} else if (*s == '+') {
		s++;
	}

	// detect base if 0
	if (base == 0) {
		if (*s == '0') {
			if (s[1] == 'x' || s[1] == 'X') {
				base = 16;
				s += 2;
			} else {
				base = 8;
				s++;
			}
		} else {
			base = 10;
		}
	} else if (base == 16) {
		if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
			s += 2;
		}
	}

	// compute cutoff for overflow
	cutoff = neg ? KLONG_MIN : KLONG_MAX;
	cutlim = (int) (cutoff % base);
	cutoff /= base;
	if (cutlim < 0) {
		cutlim += base;
		cutoff += 1;
	}

	// convert digits
	for (;; s++) {
		c = (unsigned char) *s;
		if (kisdigit(c))
			c -= '0';
		else if (kisalpha(c))
			c = ktoupper(c) - 'A' + 10;
		else
			break;
		if (c >= base)
			break;

		// check overflow
		if (neg) {
			if (acc < cutoff || (acc == cutoff && c > cutlim)) {
				acc    = KLONG_MIN;
				kerrno = KERANGE;
				neg    = 0;  // to avoid negating again
			} else {
				acc *= base;
				acc -= c;
			}
		} else {
			if (acc > cutoff || (acc == cutoff && c > cutlim)) {
				acc    = KLONG_MAX;
				kerrno = KERANGE;
				neg    = 0;  // to avoid negating again
			} else {
				acc *= base;
				acc += c;
			}
		}
	}

	if (endptr)
		*endptr = (char *) (acc == 0 && s == nptr ? nptr : s);

	return acc;
}
