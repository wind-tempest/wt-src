// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/kexp.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/kexp.h>
#include <lib/kmath/kmath.h>

double
    kexp (double x) {
	// Handle special cases
	if (x == 0.0)
		return 1.0;
	if (x < -700.0)
		return 0.0;  // Underflow
	if (x > 700.0)
		return 1.0 / 0.0;  // Overflow

	// Use Taylor series: e^x = 1 + x + x^2/2! + x^3/3! + ...
	double result    = 1.0;
	double term      = 1.0;
	double factorial = 1.0;

	for (int i = 1; i <= 20; i++) {
		term *= x;
		factorial *= i;
		result += term / factorial;
	}

	return result;
}
