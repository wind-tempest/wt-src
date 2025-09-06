// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kmath/kacos.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <lib/kmath/kacos.h>
#include <lib/kmath/kmath.h>

double
    kacos (double x) {
	// Check for invalid input
	if (x < -1.0 || x > 1.0)
		return knan("");

	// Use the relationship: acos(x) = π/2 - asin(x)
	return K_M_PI_2 - kasin(x);
}
