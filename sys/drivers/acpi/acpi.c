// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/drivers/acpi/acpi.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "acpi.h"

#include <arch/amd64/kasm/kio.h>
#include <debug/debug.h>
#include <lib/kstdio/kstddef.h>
#include <lib/kunistd/ksleep.h>

/*
 * This is not ACPI yet, this is a placeholder for ACPI.
 * Since I will polish the Tempest kernel before I decide
 * to lose my precious time on ACPI.
 */

// ? Maybe use uACPI?

// Define timeout for poweroff in milliseconds
#define POWEROFF_TIMEOUT_MS 5000

void
    a_poweroff (void) {
	koutw(0x604, 0x2000);   // Port 0x604
	koutw(0xB004, 0x2000);  // Port 0xB004

	ksleep(POWEROFF_TIMEOUT_MS);
	// If poweroff fails, log warning
	debug.err("Legacy poweroff failed", "acpi", KNULL);
}

void
    a_reboot (void) {
	while (kinb(0x64) & 0x02)
		;           // Wait for keyboard controller ready
	koutb(0x64, 0xFE);  // Send reset command

	ksleep(POWEROFF_TIMEOUT_MS);
	// If reboot fails, log warning
	debug.err("Legacy reboot failed", "acpi", KNULL);
}

struct Acpi acpi = {.poweroff = a_poweroff, .reboot = a_reboot};
