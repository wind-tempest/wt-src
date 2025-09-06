// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/khypot.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/khypot.h>
#include <lib/kmath/kmath.h>

double
    khypot (double x, double y) {
	// Handle special cases
	if (kisinf(x) || kisinf(y)) {
		return 1.0 / 0.0;  // Return infinity
	}

	if (kisnan(x) || kisnan(y)) {
		return 0.0;  // Return NaN
	}

	// Use the formula: ksqrt(x^2 + y^2)
	return ksqrt(x * x + y * y);
}
