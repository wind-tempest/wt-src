// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstring/kmemcmp.c
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
#include <lib/kstring/kmemcmp.h>

int
    kmemcmp (const void *s1, const void *s2, ksize_t n) {
	const unsigned char *a = s1;
	const unsigned char *b = s2;
	for (ksize_t i = 0; i < n; ++i) {
		if (a[i] != b[i])
			return (int) a[i] - (int) b[i];
	}
	return 0;
}
