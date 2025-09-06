// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/katan2.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/katan2.h>
#include <lib/kmath/kmath.h>

double
    katan2 (double y, double x) {
	// Handle special cases
	if (kisnan(x) || kisnan(y))
		return knan("");
	if (kisinf(x) && kisinf(y))
		return knan("");

	// If x is zero
	if (x == 0.0) {
		if (y > 0.0)
			return K_M_PI_2;
		if (y < 0.0)
			return -K_M_PI_2;
		return 0.0;  // y is also zero
	}

	// If y is zero
	if (y == 0.0) {
		if (x > 0.0)
			return 0.0;
		if (x < 0.0)
			return K_M_PI;
		return 0.0;
	}

	// If x is infinite
	if (kisinf(x)) {
		if (x > 0.0)
			return y > 0.0 ? 0.0 : -0.0;
		else
			return y > 0.0 ? K_M_PI : -K_M_PI;
	}

	// If y is infinite
	if (kisinf(y)) {
		return y > 0.0 ? K_M_PI_2 : -K_M_PI_2;
	}

	// Normal case: use atan(y/x) with proper quadrant adjustment
	double result = katan(y / x);

	if (x < 0.0) {
		if (y >= 0.0)
			result += K_M_PI;
		else
			result -= K_M_PI;
	}

	return result;
}
