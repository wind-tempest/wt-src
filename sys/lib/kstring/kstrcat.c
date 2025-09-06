// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstring/kstrcat.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kstring/kstrcat.h>

char *
    kstrcat (char *dest, const char *src) {
	if (!dest || !src)
		return dest;

	// Find the end of dest
	char *end = dest;
	while (*end) {
		end++;
	}

	// Copy src to the end of dest
	while (*src) {
		*end = *src;
		end++;
		src++;
	}

	*end = '\0';
	return dest;
}
