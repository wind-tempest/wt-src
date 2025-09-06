// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstring/kstrncmp.c
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
#include <lib/kstring/kstrncmp.h>

int
    kstrncmp (const char *s1, const char *s2, ksize_t n) {
	for (ksize_t i = 0; i < n; i++) {
		if (s1[i] != s2[i]) {
			return (unsigned char) s1[i] - (unsigned char) s2[i];
		}
		if (s1[i] == '\0') {
			return 0;
		}
	}
	return 0;
}
