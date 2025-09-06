// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstdlib/katoi.c
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
#include <lib/kstdlib/katoi.h>
#include <lib/kstdlib/kstdlib.h>

int
    katoi (const char *s) {
	long result = 0;
	int  sign   = 1;

	kerrno = 0;

	while (kisspace(*s))
		s++;

	if (*s == '+' || *s == '-') {
		if (*s == '-')
			sign = -1;
		s++;
	}

	if (!kisdigit(*s)) {
		kerrno = KEINVAL;
		return 0;
	}

	while (kisdigit(*s)) {
		int digit = *s - '0';

		if (result > (KLONG_MAX - digit) / 10) {
			kerrno = KERANGE;
			return sign == 1 ? KINT_MAX : KINT_MIN;
		}

		result = result * 10 + digit;
		s++;
	}

	result *= sign;

	if (result < KINT_MIN || result > KINT_MAX) {
		kerrno = KERANGE;
		return result > 0 ? KINT_MAX : KINT_MIN;
	}

	return (int) result;
}
