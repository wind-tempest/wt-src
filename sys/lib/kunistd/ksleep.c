// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/kunistd/ksleep.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include <arch/amd64/kasm/kio.h>
#include <lib/kstdio/kstddef.h>
#include <lib/kstdio/kstdint.h>
#include <lib/kunistd/ksleep.h>

volatile kuint64_t *hpet = (volatile kuint64_t *) 0xFED00000;

#define HPET_GEN_CONF        0x10
#define HPET_MAIN_COUNTER    0xF0
#define HPET_CAP_ID          0x0
#define HPET_GEN_CONF_ENABLE (1 << 0)

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

void
    khpet_enable (void) {
	if ((hpet[HPET_GEN_CONF / 8] & HPET_GEN_CONF_ENABLE) == 0) {
		hpet[HPET_GEN_CONF / 8] |= HPET_GEN_CONF_ENABLE;
	}
}

// PIT busy-wait ~1ms per tick
void
    kpit_wait (int ms) {
	koutb(PIT_COMMAND, 0x34);

	kuint16_t reload = 1193;  // ~1 ms (1193182 Hz / 1000)
	koutb(PIT_CHANNEL0, (kuint8_t) (reload & 0xFF));
	koutb(PIT_CHANNEL0, (kuint8_t) ((reload >> 8) & 0xFF));

	for (int i = 0; i < ms; i++) {
		kuint8_t prev = 0xFF, curr = 0;
		do {
			curr = kinb(PIT_CHANNEL0);
			if (curr > prev)
				break;
			prev = curr;
		} while (curr != 0);
	}
}

void
    ksleep (int ms) {
	if (hpet != KNULL) {
		khpet_enable();

		kuint64_t period_fs = hpet[HPET_CAP_ID / 8] >> 32;
		kuint64_t start     = hpet[HPET_MAIN_COUNTER / 8];

		kuint64_t ticks = (kuint64_t) ms * 1000000000ULL / (period_fs / 1000);

		while ((hpet[HPET_MAIN_COUNTER / 8] - start) < ticks) {
			kcpu_relax();
		}
	} else {
		kpit_wait(ms);
	}
}
