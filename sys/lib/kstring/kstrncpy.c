// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstring/kstrncpy.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kstdio/kstddef.h>
#include <lib/kstring/kstrncpy.h>

char *
    kstrncpy (char *dest, const char *src, ksize_t n) {
	char       *d = dest;
	const char *s = src;

	while (n && *s) {
		*d++ = *s++;
		n--;
	}

	while (n--) {
		*d++ = '\0';
	}

	return dest;
}
