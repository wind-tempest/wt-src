// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/drivers/video/font/font.h
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#pragma once

#include <lib/kstdio/kstdint.h>

// A simple 8x8 font for rendering text to the framebuffer.
// Each character is 8 pixels wide and 8 pixels tall.
extern const kuint32_t FONT_WIDTH;
extern const kuint32_t FONT_HEIGHT;

extern const unsigned char font[128][8];
