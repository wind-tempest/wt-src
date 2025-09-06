// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/ktan.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/kmath.h>
#include <lib/kmath/ktan.h>

double
    ktan (double x) {
	double cos_x = kcos(x);
	if (cos_x == 0.0) {
		// Return infinity for undefined values
		return 1.0 / 0.0;
	}
	return ksin(x) / cos_x;
}
