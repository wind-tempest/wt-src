// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        include/lib/kmath/kmath.h
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#pragma once

#include "kacos.h"
#include "kasin.h"
#include "katan.h"
#include "katan2.h"
#include "kcbrt.h"
#include "kceil.h"
#include "kcopysign.h"
#include "kcos.h"
#include "kexp.h"
#include "kexp2.h"
#include "kexpm1.h"
#include "kfabs.h"
#include "kfloor.h"
#include "kfmax.h"
#include "kfmin.h"
#include "kfmod.h"
#include "kfrexp.h"
#include "khypot.h"
#include "kisfinite.h"
#include "kisinf.h"
#include "kisnan.h"
#include "kldexp.h"
#include "klog.h"
#include "klog10.h"
#include "klog1p.h"
#include "klog2.h"
#include "kmath.h"
#include "kmodf.h"
#include "knan.h"
#include "knearbyint.h"
#include "knextafter.h"
#include "kpow.h"
#include "kremainder.h"
#include "kremquo.h"
#include "krint.h"
#include "kround.h"
#include "ksin.h"
#include "ksqrt.h"
#include "ktan.h"
#include "ktrunc.h"

// Mathematical constants
#define K_M_E        2.7182818284590452354   // e
#define K_M_LOG2E    1.4426950408889634074   // log_2 e
#define K_M_LOG10E   0.43429448190325182765  // log_10 e
#define K_M_LN2      0.69314718055994530942  // log_e 2
#define K_M_LN10     2.30258509299404568402  // log_e 10
#define K_M_PI       3.14159265358979323846  // pi
#define K_M_PI_2     1.57079632679489661923  // pi/2
#define K_M_PI_4     0.78539816339744830962  // pi/4
#define K_M_1_PI     0.31830988618379067154  // 1/pi
#define K_M_2_PI     0.63661977236758134308  // 2/pi
#define K_M_2_SQRTPI 1.12837916709551257390  // 2/ksqrt(pi)
#define K_M_SQRT2    1.41421356237309504880  // ksqrt(2)
#define K_M_SQRT1_2  0.70710678118654752440  // 1/ksqrt(2)

/*
    Just a fucking warning!
    FP_INFINITE: classification macro (integer)
    INFINITY: floating-point constant (IEEE 754 positive infinity)
*/

// Infinity and NaN constants
#define K_INFINITY (1.0 / 0.0)
#define K_NAN      (0.0 / 0.0)

// Classification macros
#define K_FP_NAN       0
#define K_FP_ZERO      2
#define K_FP_SUBNORMAL 3
#define K_FP_NORMAL    4
