// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/kfmod.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/kfmod.h>
#include <lib/kmath/kmath.h>

double
    kfmod (double x, double y) {
	if (y == 0.0) {
		// Return NaN for division by zero
		return 0.0;
	}

	if (x == 0.0) {
		return 0.0;
	}

	// Calculate remainder using: x - y * floor(x/y)
	double quotient = x / y;
	double int_part = kfloor(quotient);
	return x - y * int_part;
}
