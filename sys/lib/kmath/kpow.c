// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/kpow.c
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
#include <lib/kmath/kpow.h>

double
    kpow (double x, double y) {
	// Handle special cases
	if (x == 0.0) {
		if (y > 0.0)
			return 0.0;
		if (y == 0.0)
			return 1.0;
		// y < 0: return infinity
		return 1.0 / 0.0;
	}

	if (x == 1.0)
		return 1.0;

	if (y == 0.0)
		return 1.0;
	if (y == 1.0)
		return x;
	if (y == 2.0)
		return x * x;
	if (y == 0.5)
		return ksqrt(x);

	// For integer powers, use repeated multiplication
	if (y == (int) y && y > 0 && y < 100) {
		double result = 1.0;
		for (int i = 0; i < (int) y; i++) {
			result *= x;
		}
		return result;
	}

	// For negative integer powers
	if (y == (int) y && y < 0 && y > -100) {
		double result = 1.0;
		for (int i = 0; i < -(int) y; i++) {
			result /= x;
		}
		return result;
	}

	// General case: use exp(y * log(x))
	return kexp(y * klog(x));
}
