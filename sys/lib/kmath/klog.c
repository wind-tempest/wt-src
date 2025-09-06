// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/klog.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/klog.h>
#include <lib/kmath/kmath.h>

double
    klog (double x) {
	// Handle special cases
	if (x <= 0.0) {
		// Return NaN for non-positive numbers
		return 0.0;
	}

	if (x == 1.0)
		return 0.0;

	// Use Newton's method to find ln(x)
	// We solve: e^y = x, so y = ln(x)
	double y = 0.0;

	// Initial guess
	if (x > 1.0) {
		y = 1.0;
	} else {
		y = -1.0;
	}

	// Newton iteration: y_{n+1} = y_n + (x - e^y_n) / e^y_n
	for (int i = 0; i < 10; i++) {
		double prev_y = y;
		double exp_y  = kexp(y);
		y             = y + (x - exp_y) / exp_y;

		if (kfabs(y - prev_y) < 1e-15) {
			break;
		}
	}

	return y;
}
