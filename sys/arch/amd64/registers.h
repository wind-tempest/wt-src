// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/arch/amd64/registers.h
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

#include <lib/kstdio/kstdint.h>

// This struct defines the registers that our ISR/IRQ stubs push to the stack.
// We receive a pointer to this in our C-level interrupt handlers.
typedef struct {
	kuint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	kuint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
	kuint64_t int_no, err_code;  // These are pushed first by the ISR.
} registers_t;
