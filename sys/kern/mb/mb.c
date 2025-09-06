// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/kern/mb/mb.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "mb.h"

#include "kern/framebuf/framebuf.h"

#include <debug/debug.h>
#include <lib/kstdio/kstdio.h>

// Multiboot2 header structure.
struct multiboot_header {
	kuint32_t total_size;
	kuint32_t reserved;
} __attribute__((aligned(8)));

struct multiboot_tag {
	kuint32_t type;
	kuint32_t size;
} __attribute__((aligned(8)));

struct multiboot_tag_framebuffer {
	kuint32_t type;
	kuint32_t size;
	kuint64_t addr;
	kuint32_t pitch;
	kuint32_t width;
	kuint32_t height;
	kuint8_t  bpp;
	kuint8_t  type_fb;
	kuint8_t  red_mask_size;
	kuint8_t  red_mask_shift;
	kuint8_t  green_mask_size;
	kuint8_t  green_mask_shift;
	kuint8_t  blue_mask_size;
	kuint8_t  blue_mask_shift;
	kuint8_t  reserved[2];
} __attribute__((aligned(8)));

typedef enum {
	MULTIBOOT_TAG_TYPE_END         = 0,
	MULTIBOOT_TAG_TYPE_FRAMEBUFFER = 8
} multiboot_tag_type_t;

void
    parse_multiboot_info (void *mb_info) {
	if (mb_info == KNULL) {
		debug.puts("mb_info is NULL!");
		return;
	}

	kuint32_t total_size = *(kuint32_t *) mb_info;
	kuint8_t *current    = (kuint8_t *) ((kuintptr_t) mb_info + 8);
	kuint8_t *end        = (kuint8_t *) ((kuintptr_t) mb_info + total_size);

	debug.puts("Parsing multiboot info...");

	while (current < end) {
		struct multiboot_tag *tag = (struct multiboot_tag *) current;

		if (tag->size == 0) {
			debug.err("Invalid tag size (0)", "multiboot", KNULL);
			return;
		}

		switch ((multiboot_tag_type_t) tag->type) {
			case MULTIBOOT_TAG_TYPE_END:
				debug.puts("End tag found. Parsing complete.");
				return;

			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
				debug.puts("Framebuffer tag found");

				struct multiboot_tag_framebuffer *fb_tag =
				    (struct multiboot_tag_framebuffer *) tag;

				fb_info.pitch            = fb_tag->pitch;
				fb_info.width            = fb_tag->width;
				fb_info.height           = fb_tag->height;
				fb_info.bpp              = fb_tag->bpp;
				fb_info.type             = fb_tag->type_fb;
				fb_info.red_mask_size    = fb_tag->red_mask_size;
				fb_info.red_mask_shift   = fb_tag->red_mask_shift;
				fb_info.green_mask_size  = fb_tag->green_mask_size;
				fb_info.green_mask_shift = fb_tag->green_mask_shift;
				fb_info.blue_mask_size   = fb_tag->blue_mask_size;
				fb_info.blue_mask_shift  = fb_tag->blue_mask_shift;

				map_framebuffer_address(fb_tag->addr);
				break;
			}

			default:
				// Unhandled tag
				// ! DO NOT FUCKING PRINT A ERROR MESSAGE
				// ! THIS IS NOT A ERROR, JUST A UNHANDLED TAG
				break;
		}

		current += (tag->size + 7) & (kuint32_t) ~7;  // align to 8 bytes
	}

	debug.puts("Reached end of multiboot info (no END tag)");
}
