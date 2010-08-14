/*
 * LMAC Interface specific definitions for mac80211 Prism54 drivers
 *
 * Copyright (c) 2006, Michael Wu <flamingice@sourmilk.net>
 * Copyright (c) 2007 - 2009, Christian Lamparter <chunkeey@web.de>
 *
 * Based on:
 * - the islsm (softmac prism54) driver, which is:
 *   Copyright 2004-2006 Jean-Baptiste Note <jbnote@gmail.com>, et al.
 *
 * - LMAC API interface header file for STLC4560 (lmac_longbow.h)
 *   Copyright (C) 2007 Conexant Systems, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef LMAC_H
#define LMAC_H

/*
 * shared interface ID definitions
 * The interface ID is a unique identification of a specific interface.
 * The following values are reserved: 0x0000, 0x0002, 0x0012, 0x0014, 0x0015
 */
#define IF_ID_ISL36356A			0x0001	/* ISL36356A <-> Firmware */
#define IF_ID_MVC			0x0003	/* MAC Virtual Coprocessor */
#define IF_ID_DEBUG			0x0008	/* PolDebug Interface */
#define IF_ID_PRODUCT			0x0009
#define IF_ID_OEM			0x000a
#define IF_ID_PCI3877			0x000b	/* 3877 <-> Host PCI */
#define IF_ID_ISL37704C			0x000c	/* ISL37704C <-> Fw */
#define IF_ID_ISL39000			0x000f	/* ISL39000 <-> Fw */
#define IF_ID_ISL39300A			0x0010	/* ISL39300A <-> Fw */
#define IF_ID_ISL37700_UAP		0x0016	/* ISL37700 uAP Fw <-> Fw */
#define IF_ID_ISL39000_UAP		0x0017	/* ISL39000 uAP Fw <-> Fw */
#define IF_ID_LMAC			0x001a	/* Interface exposed by LMAC */

struct exp_if {
	__le16 role;
	__le16 if_id;
	__le16 variant;
	__le16 btm_compat;
	__le16 top_compat;
} __packed;

struct dep_if {
	__le16 role;
	__le16 if_id;
	__le16 variant;
} __packed;

#endif /* LMAC_H */
