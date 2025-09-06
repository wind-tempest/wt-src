// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/drivers/serial/serial.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "serial.h"

#include <arch/amd64/kasm/kio.h>
#include <lib/kstdio/kstdbool.h>
#include <lib/kstdlib/kitoa.h>

#define PORT 0x3f8  // COM1

kbool
    s_is_available (void) {
	// Save original values.
	unsigned char original_lcr = kinb(PORT + 3);
	unsigned char original_mcr = kinb(PORT + 4);

	// Set up loopback mode.
	koutb(PORT + 3, 0x80);  // Enable DLAB.
	koutb(PORT + 4, 0x10);  // Enable loopback mode.

	// Test value to write and read back.
	unsigned char test_value = 0xAA;

	// Write test value.
	koutb(PORT, test_value);

	// Small delay to ensure data is processed.
	for (volatile int i = 0; i < 1000; i++)
		;

	// Read back the value.
	unsigned char read_value = kinb(PORT);

	// Restore original values.
	koutb(PORT + 3, original_lcr);
	koutb(PORT + 4, original_mcr);

	// If read value matches test value, port is available.
	return (read_value == test_value);
}

void
    s_init (void) {
	if (!s_is_available())
		return;         // Exit if serial not available.
	koutb(PORT + 1, 0x00);  // Disable all interrupts.
	koutb(PORT + 3, 0x80);  // Enable DLAB (set baud rate divisor).
	koutb(PORT + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
	koutb(PORT + 1, 0x00);  //                  (hi byte)
	koutb(PORT + 3, 0x03);  // 8 bits, no parity, one stop bit.
	koutb(PORT + 2,
	      0xC7);            // Enable FIFO, clear them, with 14-byte threshold.
	koutb(PORT + 4, 0x0B);  // IRQs enabled, RTS/DSR set.
}

static int
    s_is_transmit_empty (void) {
	return kinb(PORT + 5) & 0x20;
}

void
    s_write (char a) {
	if (!s_is_available() || !a)
		return;  // Exit if serial not available or if char is null.
	while (s_is_transmit_empty() == 0)
		;
	koutb(PORT, (unsigned char) a);
}

void
    s_writes (const char *s) {
	if (!s_is_available() || !s)
		return;  // Exit if serial not available or if string is null.
	while (*s) {
		s_write(*s++);
	}
}

void
    s_write_int (int i) {
	if (!s_is_available())
		return;  // Exit if serial not available.
	char buf[16];
	kitoa(buf, buf + 15, i, 10, 0);
	serial.writes(buf);
}

struct Serial serial = {.init         = s_init,
                        .write        = s_write,
                        .writes       = s_writes,
                        .write_int    = s_write_int,
                        .is_available = s_is_available};
