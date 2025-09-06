// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/kcbrt.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/kcbrt.h>
#include <lib/kmath/kmath.h>

double
    kcbrt (double x) {
	// Handle special cases
	if (kisnan(x))
		return knan("");
	if (kisinf(x))
		return x;
	if (x == 0.0)
		return 0.0;

	// Use Newton's method for cube root
	double y = kfabs(x);
	double guess;

	// Initial guess
	if (y > 1.0)
		guess = y / 3.0;
	else
		guess = y * 2.0 / 3.0;

	// Newton iteration: x_{n+1} = (2*x_n + y/x_n^2) / 3
	for (int i = 0; i < 10; i++) {
		double guess2    = guess * guess;
		double new_guess = (2.0 * guess + y / guess2) / 3.0;
		if (kfabs(new_guess - guess) < 1e-15)
			break;
		guess = new_guess;
	}

	return x < 0.0 ? -guess : guess;
}
