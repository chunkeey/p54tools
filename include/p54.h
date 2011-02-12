/*
 * Shared defines for all mac80211 Prism54 code
 *
 * Copyright (c) 2006, Michael Wu <flamingice@sourmilk.net>
 *
 * Based on the islsm (softmac prism54) driver, which is:
 * Copyright 2004-2006 Jean-Baptiste Note <jbnote@gmail.com>, et al.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef P54_H
#define P54_H

#include "types.h"

struct p54_cal_database {
	size_t entries;
	size_t entry_size;
	size_t offset;
	size_t len;
	uint8_t data[0];
};

struct p54_rssi_db_entry_v1 {
	__le16 mul;
	__le16 add;
	__le16 longbow_unkn;
	__le16 longbow_unkn2;
};

struct p54_rssi_db_entry_v2 {
	__le16 freq;
	__le16 mul;
	__le16 add;
	__le16 longbow_unkn;
	__le16 longbow_unkn2;
};

#define P54_MAX_EEPROM_LEN	8192

#define PDA_REAL_LEN(entry) ((uint16_t)(le16_to_cpu((entry)->len) + 1) * 2)
#define PDA_DATA_LEN(entry) ((uint16_t)(le16_to_cpu((entry)->len) - 1) * 2)

#endif /* P54_H */
