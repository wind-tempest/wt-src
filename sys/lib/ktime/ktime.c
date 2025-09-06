// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/lib/ktime/ktime.c
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
#include <lib/kstdio/kstdio.h>
#include <lib/ktime/ktime.h>

// BIOS interrupt for getting RTC (Real Time Clock) data
#define RTC_COMMAND_PORT 0x70
#define RTC_DATA_PORT    0x71

// RTC register addresses
#define RTC_SECONDS      0x00
#define RTC_MINUTES      0x02
#define RTC_HOURS        0x04
#define RTC_DAY_OF_WEEK  0x06
#define RTC_DAY_OF_MONTH 0x07
#define RTC_MONTH        0x08
#define RTC_YEAR         0x09
#define RTC_CENTURY      0x32

// RTC status register B
#define RTC_STATUS_B 0x0B

// BCD to decimal conversion
static kuint8_t
    kbcd_to_decimal (kuint8_t bcd) {
	return (kuint8_t) (((bcd >> 4) * 10) + (bcd & 0x0F));
}

// Read a byte from RTC register
static kuint8_t
    krtc_read (kuint8_t reg) {
	koutb(RTC_COMMAND_PORT, reg);
	return kinb(RTC_DATA_PORT);
}

// Check if RTC is in BCD mode
static kbool
    krtc_is_bcd (void) {
	kuint8_t status_b = krtc_read(RTC_STATUS_B);
	return !(status_b & 0x04);  // Bit 2 clear means BCD mode
}

// Get current BIOS time
void
    kget_bios_time (struct bios_time *time) {
	if (!time)
		return;

	kbool is_bcd = krtc_is_bcd();

	// Read all time components
	kuint8_t seconds     = krtc_read(RTC_SECONDS);
	kuint8_t minutes     = krtc_read(RTC_MINUTES);
	kuint8_t hours       = krtc_read(RTC_HOURS);
	kuint8_t day         = krtc_read(RTC_DAY_OF_MONTH);
	kuint8_t month       = krtc_read(RTC_MONTH);
	kuint8_t year        = krtc_read(RTC_YEAR);
	kuint8_t century     = krtc_read(RTC_CENTURY);
	kuint8_t day_of_week = krtc_read(RTC_DAY_OF_WEEK);

	// Convert BCD to decimal if needed
	if (is_bcd) {
		time->second      = kbcd_to_decimal(seconds);
		time->minute      = kbcd_to_decimal(minutes);
		time->hour        = kbcd_to_decimal(hours);
		time->day         = kbcd_to_decimal(day);
		time->month       = kbcd_to_decimal(month);
		time->year        = (kuint16_t) (kbcd_to_decimal(year)
                                          + (kbcd_to_decimal(century) * 100));
		time->day_of_week = kbcd_to_decimal(day_of_week);
	} else {
		time->second      = seconds;
		time->minute      = minutes;
		time->hour        = hours;
		time->day         = day;
		time->month       = month;
		time->year        = (kuint16_t) (year + (century * 100));
		time->day_of_week = day_of_week;
	}

	// Handle 12-hour format (bit 6 of hours register indicates PM)
	if (!(hours & 0x80) && (hours & 0x40)) {
		// 12-hour format, PM
		time->hour = (kuint8_t) (((time->hour % 12) + 12) % 24);
	} else if (!(hours & 0x80)) {
		// 12-hour format, AM
		time->hour = time->hour % 12;
	}
}

// Get date string in DD-MM-YYYY format
void
    kget_date_string (char *buffer, ksize_t buffer_size) {
	if (!buffer || buffer_size < 11)
		return;

	struct bios_time time;
	kget_bios_time(&time);

	// Format: DD-MM-YYYY
	ksnprintf(buffer, buffer_size, "%02d/%02d/%04d", time.day, time.month, time.year);
}

// Get time string in HH:MM:SS format
void
    kget_time_string (char *buffer, ksize_t buffer_size) {
	if (!buffer || buffer_size < 9)
		return;

	struct bios_time time;
	kget_bios_time(&time);

	// Format: HH:MM:SS
	ksnprintf(
	    buffer, buffer_size, "%02d:%02d:%02d", time.hour, time.minute, time.second);
}

// Get full date and time string
void
    kget_datetime_string (char *buffer, ksize_t buffer_size) {
	if (!buffer || buffer_size < 16)
		return;

	struct bios_time time;
	kget_bios_time(&time);

	// Format: DD-MM-YYYY HH:MM
	kprintf("%02d-%02d-%04d %02d:%02d",
	        time.day,
	        time.month,
	        time.year,
	        time.hour,
	        time.minute);

	// Copy to buffer
	ksnprintf(buffer,
	          buffer_size,
	          "%02d/%02d/%02d %02d:%02d",
	          time.day,
	          time.month,
	          time.year % 100,
	          time.hour,
	          time.minute);
}

// Get day of week string
const char *
    kget_day_of_week_string (kuint8_t day_of_week) {
	static const char *days[] = {
	    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

	if (day_of_week >= 1 && day_of_week <= 7) {
		return days[day_of_week - 1];
	}
	return "Unknown";
}
