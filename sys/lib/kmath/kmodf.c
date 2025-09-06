// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/kmodf.c
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
#include <lib/kmath/kmodf.h>

double
    kmodf (double x, double *iptr) {
	// Handle special cases
	if (kisnan(x)) {
		*iptr = knan("");
		return knan("");
	}
	if (kisinf(x)) {
		*iptr = x;
		return 0.0;
	}

	// Extract integer part
	if (x >= 0.0)
		*iptr = kfloor(x);
	else
		*iptr = kceil(x);

	// Return fractional part
	return x - *iptr;
}
