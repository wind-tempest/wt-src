// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kstring/kmemset.c
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
#include <lib/kstdio/kstdint.h>

void *
    kmemset (void *restrict s, int c, ksize_t n) {
	if (!s)
		return KNULL;  // NULL-check for safety

	unsigned char *byte_ptr = (unsigned char *) s;
	ksize_t        i        = 0;

	// Prepare the word-sized pattern
	kuintptr_t pattern = (unsigned char) c;
	pattern |= pattern << 8;
	pattern |= pattern << 16;
#if KUINTPTR_MAX > 0xffffffff
	pattern |= pattern << 32;
#endif

	// Align to word boundary
	ksize_t    align    = sizeof(kuintptr_t);
	kuintptr_t addr     = (kuintptr_t) byte_ptr;
	ksize_t    misalign = addr % align;
	ksize_t    to_align = misalign ? (align - misalign) : 0;

	// Fill byte-by-byte until aligned
	for (; i < n && to_align--; ++i) {
		byte_ptr[i] = (unsigned char) c;
	}

	// Fill word-by-word
	ksize_t     words    = (n - i) / align;
	kuintptr_t *word_ptr = (kuintptr_t *) (byte_ptr + i);
	for (ksize_t w = 0; w < words; ++w) {
		word_ptr[w] = pattern;
	}

	// Advance index
	i += words * align;

	// Fill remaining bytes
	for (; i < n; ++i) {
		byte_ptr[i] = (unsigned char) c;
	}

	return s;
}
