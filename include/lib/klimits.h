// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        include/lib/klimits.h
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

// Number of bits in a `char`.
#define CHAR_BIT 8

// Minimum and maximum values for signed char.
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127

// Minimum and maximum values for char.
#if defined(__CHAR_UNSIGNED__)
#	define CHAR_MIN 0
#	define CHAR_MAX 255
#else
#	define CHAR_MIN SCHAR_MIN
#	define CHAR_MAX SCHAR_MAX
#endif

// Maximum value for unsigned char.
#define KUCHAR_MAX 255

// Maximum value for multibyte character length.
#define KMB_LEN_MAX 16

// Minimum and maximum values for short.
#define KSHRT_MIN  (-32768)
#define KSHRT_MAX  32767
#define KUSHRT_MAX 65535U

// Minimum and maximum values for int.
#define KINT_MIN  (-2147483647 - 1)
#define KINT_MAX  2147483647
#define KUINT_MAX 4294967295U

// Minimum and maximum values for long.
#if defined(_LP64) || defined(__x86_64__)
// LP64: long is 64 bits
#	define KLONG_MIN  (-9223372036854775807L - 1)
#	define KLONG_MAX  9223372036854775807L
#	define KULONG_MAX 18446744073709551615UL
#else
// ILP32: long is 32 bits
#	define KLONG_MIN  (-2147483647L - 1)
#	define KLONG_MAX  2147483647L
#	define KULONG_MAX 4294967295UL
#endif

// Minimum and maximum values for long long.
#define KLLONG_MIN  (-9223372036854775807LL - 1)
#define KLLONG_MAX  9223372036854775807LL
#define KULLONG_MAX 18446744073709551615ULL

// Additional limits (C99 and POSIX)

// Pointer difference type
#define KPTRDIFF_MIN (-KINT_MAX - 1)
#define KPTRDIFF_MAX KINT_MAX

// Size type
#define KSIZE_MAX KUINT_MAX

// sig_atomic_t
#define KSIG_ATOMIC_MIN (-2147483647 - 1)
#define KSIG_ATOMIC_MAX 2147483647

// wchar_t and wint_t
#define KWCHAR_MIN 0
#define KWCHAR_MAX 4294967295U
#define KWINT_MIN  0u
#define KWINT_MAX  4294967295u
