// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/ksin.c
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
#include <lib/kmath/ksin.h>

double
    ksin (double x) {
	// Normalize x to [-π, π]
	x = kfmod(x, 2.0 * K_M_PI);
	if (x > K_M_PI)
		x -= 2.0 * K_M_PI;
	if (x < -K_M_PI)
		x += 2.0 * K_M_PI;

	// Use Taylor series for small values
	if (kfabs(x) < 0.1) {
		double x2 = x * x;
		double x3 = x2 * x;
		double x5 = x3 * x2;
		double x7 = x5 * x2;
		return x - x3 / 6.0 + x5 / 120.0 - x7 / 5040.0;
	}

	// For larger values, use more terms of Taylor series
	double result    = 0.0;
	double term      = x;
	double x2        = x * x;
	double factorial = 1.0;
	int    sign      = 1;

	for (int i = 1; i <= 10; i++) {
		result += sign * term / factorial;
		term *= x2;
		factorial *= (2 * i) * (2 * i + 1);
		sign = -sign;
	}

	return result;
}
