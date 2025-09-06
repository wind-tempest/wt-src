// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/kfrexp.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/kfrexp.h>
#include <lib/kmath/kmath.h>

double
    kfrexp (double x, int *exp) {
	// Handle special cases
	if (kisnan(x)) {
		*exp = 0;
		return knan("");
	}
	if (kisinf(x)) {
		*exp = 0;
		return x;
	}
	if (x == 0.0) {
		*exp = 0;
		return 0.0;
	}

	// Extract exponent using log2
	*exp = (int) kfloor(klog2(kfabs(x))) + 1;

	// Calculate mantissa: x / 2^exp
	return x / kpow(2.0, *exp);
}
