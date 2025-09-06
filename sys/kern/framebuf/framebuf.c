// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/kern/framebuf/framebuf.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "framebuf.h"

#include <debug/debug.h>
#include <lib/kstdio/kstdint.h>

struct framebuffer_info fb_info;

void
    map_framebuffer_address (kuint64_t phys_addr) {
	kuint64_t virt_addr = 0xFFFF800000000000ULL + phys_addr;

	debug.printf("Mapping framebuffer 0x%llx -> 0x%llx\n", phys_addr, virt_addr);

	fb_info.addr = virt_addr;
}
