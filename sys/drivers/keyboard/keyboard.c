// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/drivers/keyboard/keyboard.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "drivers/keyboard/keyboard.h"

#include <arch/amd64/idt.h>
#include <arch/amd64/kasm/kio.h>
#include <lib/kstdio/kstdio.h>

#define KBD_DATA_PORT   0x60
#define KBD_BUFFER_SIZE 256

// Simple keyboard buffer
static unsigned char kbd_buffer[KBD_BUFFER_SIZE];
static int           kbd_buffer_head = 0;
static int           kbd_buffer_tail = 0;
static kbool         shift_pressed   = kfalse;
static kbool         caps_lock       = kfalse;

// Scancode set 1 to ASCII mapping for US keyboard layout
static unsigned char kbd_us[128] = {
    0,   27,   '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   '-', 0,   0,   0,   /* + -> */ 0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,
};

static unsigned char kbd_us_shift[128] = {
    0,   27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   '-', 0,   0,   0,   /* + -> */ 0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,
};

static unsigned char
    k_adjust_case (unsigned char c) {
	if (caps_lock && !shift_pressed && c >= 'a' && c <= 'z')
		return (unsigned char) (c - 32);
	if (caps_lock && shift_pressed && c >= 'A' && c <= 'Z')
		return (unsigned char) (c + 32);
	return c;
}

static void
    k_handler (registers_t *regs) {
	(void) regs;
	unsigned char scancode = kinb(KBD_DATA_PORT);

	// Caps Lock pressed
	if (scancode == 0x3A) {
		caps_lock = !caps_lock;  // Toggle caps lock
		return;
	}

	if (scancode == 0x2A || scancode == 0x36) {
		shift_pressed = ktrue;
		return;
	}
	if (scancode == 0xAA || scancode == 0xB6) {
		shift_pressed = kfalse;
		return;
	}

	if (scancode < 128) {
		unsigned char c = shift_pressed ? kbd_us_shift[scancode]
		                                : kbd_us[scancode];

		c = keyboard.adjust_case(c);

		if (c != 0) {
			if ((kbd_buffer_head + 1) % KBD_BUFFER_SIZE != kbd_buffer_tail) {
				kbd_buffer[kbd_buffer_head] = c;
				kbd_buffer_head = (kbd_buffer_head + 1) % KBD_BUFFER_SIZE;
			}
		}
	}
}

/*
 ! This only works in single-tasking environments.
 ! Needs locking or buffer protection for multi-tasking.
 */
int
    getchar (void) {
	// Wait for a character to be available
	while (kbd_buffer_head == kbd_buffer_tail) {
		__asm__ volatile("sti; hlt");
	}
	__asm__ volatile("cli");

	int c           = kbd_buffer[kbd_buffer_tail];
	kbd_buffer_tail = (kbd_buffer_tail + 1) % KBD_BUFFER_SIZE;

	__asm__ volatile("sti");
	return c;
}

void
    k_init (void) {
	register_irq_handler(1, k_handler);
}

struct Keyboard keyboard = {
    .init = k_init, .handle = k_handler, .adjust_case = k_adjust_case};
