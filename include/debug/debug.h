// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        include/debug/debug.h
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

extern struct Debug debug;

struct Debug {
	void (*crit)(const char *, const char *, const char *);
	void (*alert)(const char *, const char *, const char *);
	void (*emerg)(const char *, const char *, const char *);
	void (*warn)(const char *, const char *, const char *);
	void (*err)(const char *, const char *, const char *);
	void (*notice)(const char *, const char *, const char *);
	void (*info)(const char *, const char *, const char *);
	void (*puts)(const char *);
	int (*printf)(const char *format, ...);
};
