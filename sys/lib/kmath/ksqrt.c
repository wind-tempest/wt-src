// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/ksqrt.c
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
#include <lib/kmath/ksqrt.h>

double
    ksqrt (double x) {
	if (x < 0.0) {
		// Return NaN for negative numbers
		return 0.0;
	}

	if (x == 0.0 || x == 1.0) {
		return x;
	}

	// Newton-Raphson method for square root
	double guess = x / 2.0;
	double prev_guess;

	do {
		prev_guess = guess;
		guess      = (guess + x / guess) / 2.0;
	} while (kfabs(guess - prev_guess) > 1e-15);

	return guess;
}
