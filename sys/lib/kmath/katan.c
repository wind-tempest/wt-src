// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/katan.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/katan.h>
#include <lib/kmath/kmath.h>

double
    katan (double x) {
	// Handle special cases
	if (kisnan(x))
		return knan("");
	if (kisinf(x))
		return x > 0 ? K_M_PI_2 : -K_M_PI_2;
	if (x == 0.0)
		return 0.0;

	// Use series expansion for small values
	if (kfabs(x) < 1.0) {
		double x2 = x * x;
		double x3 = x2 * x;
		double x5 = x3 * x2;
		double x7 = x5 * x2;
		double x9 = x7 * x2;
		return x - x3 / 3.0 + x5 / 5.0 - x7 / 7.0 + x9 / 9.0;
	}

	// For large values, use atan(x) = π/2 - atan(1/x)
	if (x > 0)
		return K_M_PI_2 - katan(1.0 / x);
	else
		return -K_M_PI_2 - katan(1.0 / x);
}
