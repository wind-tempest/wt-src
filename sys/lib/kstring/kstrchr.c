// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstring/kstrchr.c
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
#include <lib/kstring/kstrchr.h>

char *
    kstrchr (const char *str, int c) {
	while (*str != '\0') {
		if (*str == (char) c) {
			return (char *) str;
		}
		str++;
	}

	if (c == '\0') {
		return (char *) str;
	}

	return KNULL;
}
