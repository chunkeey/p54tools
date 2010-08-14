/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef __P54EEPROM_H
#define __P54EEPROM_H

#include <linux/types.h>
#include "types.h"
#include "compiler.h"
#include "eeprom.h"
#include "list.h"

void p54e_allow_tainted_eeproms(void);

struct p54e;

void p54e_release(struct p54e *ee);
struct p54e *p54e_load_file(const char *name);
struct p54e *p54e_load_def(const unsigned char *data, size_t len);

int p54e_store(struct p54e *ee);

const void *p54e_desc_find(struct p54e *ee, const __le16 id);
void *p54e_desc_find_mod(struct p54e *ee, const __le16 id);

int p54e_desc_add(struct p54e *ee, const struct pda_entry *desc);

void *p54e_desc_mod_len(struct p54e *ee, struct pda_entry *desc, int len);

void p54e_desc_unlink(struct p54e *ee, struct pda_entry *desc);

void p54e_desc_del(struct p54e *ee, struct pda_entry *entry);

const void *p54e_desc_next(struct p54e *ee, const struct pda_entry *pos);

unsigned int p54e_get_descs_num(struct p54e *ee);
unsigned int p54e_get_descs_size(struct p54e *ee);

int p54e_set_filename(struct p54e *ee, const char *file);
char *p54e_get_filename(struct p54e *ee);

int p54e_is_dirty(struct p54e *ee);
#endif /* __P54EEPROM_H */
