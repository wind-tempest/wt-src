// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/fs/vfs/vfs.h
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

#include <lib/kstdio/kstddef.h>

extern struct Vfs vfs;

struct Vfs {
	void (*getcwd)(char *out, ksize_t size);
	void (*normalize)(const char *path, char *out, ksize_t size);
	void (*resolve)(const char *path, char *out, ksize_t size);
	int (*chdir)(const char *path);
};
