// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/drivers/ata/ata.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "drivers/ata/ata.h"

#include <arch/amd64/kasm/kio.h>
#include <lib/kstdio/kstddef.h>

#define ATA_PRIMARY_IO   0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

// I/O registers (offsets from base)
#define ATA_REG_DATA     0x00
#define ATA_REG_ERROR    0x01
#define ATA_REG_SECCOUNT 0x02
#define ATA_REG_LBA_LO   0x03
#define ATA_REG_LBA_MID  0x04
#define ATA_REG_LBA_HI   0x05
#define ATA_REG_DRIVE    0x06
#define ATA_REG_STATUS   0x07
#define ATA_REG_COMMAND  0x07

// Control register
#define ATA_REG_CONTROL  0x02  // base 0x3F6 + 2 == 0x3F8, but 0x3F6 often used
#define ATA_CONTROL_nIEN 0x02

// Status bits
#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

#define ATA_CMD_READ_SECTORS 0x20

static inline void
    io_wait (void) {
	// 400ns delay by reading alternate-status port 4 times
	for (int i = 0; i < 4; i++)
		(void) kinb(ATA_PRIMARY_CTRL);
}

// Poll until BSY=0 and either DRQ=1 or ERR=1. Return 0 on ready, -1 on error
static int
    ata_poll (void) {
	kuint8_t status;
	// Initial delay
	io_wait();
	while ((status = kinb(ATA_PRIMARY_IO + ATA_REG_STATUS)) & ATA_SR_BSY)
		;
	if (status & ATA_SR_ERR)
		return -1;
	if (!(status & ATA_SR_DRQ))
		return -1;
	return 0;
}

int
    ata_pio_read (kuint64_t lba, kuint32_t count, void *buf) {
	if (count == 0 || buf == KNULL)
		return -1;
	if (lba > 0x0FFFFFFF)  // 28-bit limit
		return -1;

	kuint8_t *ptr = (kuint8_t *) buf;
	for (kuint32_t i = 0; i < count; i++) {
		kuint32_t cur_lba = (kuint32_t) (lba + i);

		// Select drive + LBA bits 24-27
		koutb(ATA_PRIMARY_IO + ATA_REG_DRIVE, 0xE0 | ((cur_lba >> 24) & 0x0F));

		// Sector count
		koutb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT, 1);
		// LBA low/mid/high
		koutb(ATA_PRIMARY_IO + ATA_REG_LBA_LO, (kuint8_t) cur_lba);
		koutb(ATA_PRIMARY_IO + ATA_REG_LBA_MID, (kuint8_t) (cur_lba >> 8));
		koutb(ATA_PRIMARY_IO + ATA_REG_LBA_HI, (kuint8_t) (cur_lba >> 16));

		// Issue READ SECTORS command
		koutb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

		if (ata_poll() != 0)
			return -1;

		// Transfer 256 words (512 bytes)
		for (int w = 0; w < 256; w++) {
			kuint16_t data = kinw(ATA_PRIMARY_IO + ATA_REG_DATA);
			ptr[0]         = (kuint8_t) data;
			ptr[1]         = (kuint8_t) (data >> 8);
			ptr += 2;
		}
	}
	return 0;
}
struct Ata ata = {.poll = ata_poll, .pio_read = ata_pio_read};
