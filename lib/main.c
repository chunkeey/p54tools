/*
 * Copyright 2010, Christian Lamparter <chunkeey@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "p54.h"
#include "p54eeprom.h"

static int allow_bad_eeproms;

struct p54e_file {
	char *name;
	size_t len;
	char *data;
};

struct p54e {
	struct p54e_file eeprom;

	struct list_head desc_list;
	unsigned int desc_list_entries,
		     desc_list_len;

	int modified;
};

#define p54e_walk_descs(iter, ee)					\
	list_for_each_entry(iter, &ee->desc_list, h.list)

#define p54e_walk_descs_reverse(iter, ee)				\
	list_for_each_entry_reverse (iter, &ee->desc_list, h.list)

struct p54e_list_entry_head {
	struct list_head list;
};

struct p54e_list_entry {
	struct p54e_list_entry_head h;
	union {
		struct pda_entry head;
		uint32_t data[0];
		char text[0];
	};
};

static void p54e_mark_dirty(struct p54e *ee)
{
	ee->modified = 1;
}

static void p54e_clear_dirty(struct p54e *ee)
{
	ee->modified = 0;
}

static inline const struct p54e_list_entry *p54e_desc_to_entry_c(const struct pda_entry *head)
{
	return const_container_of(head, const struct p54e_list_entry, head);
}

static inline struct p54e_list_entry *p54e_desc_to_entry(struct pda_entry *head)
{
	return container_of(head, struct p54e_list_entry, head);
}

static inline struct pda_entry *p54e_entry_to_desc(struct p54e_list_entry *entry)
{
	return &entry->head;
}

static inline const struct pda_entry *p54e_entry_to_desc_c(const struct p54e_list_entry *entry)
{
	return &entry->head;
}

static void p54e_entry_unlink(struct p54e *ee, struct p54e_list_entry *entry)
{
	p54e_mark_dirty(ee);
	ee->desc_list_entries--;
	ee->desc_list_len -= PDA_REAL_LEN(&entry->head);
	list_del(&entry->h.list);
}

static void p54e_entry_del(struct p54e *ee, struct p54e_list_entry *entry)
{
	p54e_entry_unlink(ee, entry);
	free(entry);
}

static struct p54e_list_entry *p54e_find_entry(struct p54e *ee, const __le16 id)
{
	struct p54e_list_entry *iter;

	p54e_walk_descs(iter, ee) {
		if (id == iter->head.code)
			return (void *)iter;
	}

	return NULL;
}

static struct p54e_list_entry *__p54e_entry_add_prepare(struct p54e *ee,
	const struct pda_entry *desc)
{
	struct p54e_list_entry *tmp;
	unsigned int len;

	len = PDA_REAL_LEN(desc);

	if (len < sizeof(struct pda_entry))
		return ERR_PTR(-EINVAL);

	if (len % 2)
		return ERR_PTR(-EDOM);

	tmp = malloc(sizeof(*tmp) + len);
	if (!tmp)
		return ERR_PTR(-ENOMEM);

	ee->desc_list_entries++;
	ee->desc_list_len += len;

	memcpy(tmp->data, desc, len);
	return tmp;
}

static void __p54e_release(struct p54e_file *f)
{
	f->len = 0;
	if (f->name)
		free(f->name);
	f->name = NULL;

	if (f->data)
		free(f->data);
	f->data = NULL;
}

void p54e_release(struct p54e *ee)
{
	struct p54e_list_entry *entry;

	if (!IS_ERR_OR_NULL(ee)) {
		while (!list_empty(&ee->desc_list)) {
			entry = list_entry(ee->desc_list.next,
					   struct p54e_list_entry, h.list);
			p54e_entry_del(ee, entry);
		}

		__p54e_release(&ee->eeprom);
		free(ee);
	}
}

const void *p54e_desc_find(struct p54e *ee, const uint16_t descid)
{
	struct p54e_list_entry *tmp;

	tmp = p54e_find_entry(ee, cpu_to_le16(descid));

	return tmp ? p54e_entry_to_desc_c(tmp) : NULL;
}

void *p54e_desc_find_mod(struct p54e *ee, const uint16_t descid)
{
	struct p54e_list_entry *tmp;

	tmp = p54e_find_entry(ee, cpu_to_le16(descid));
	if (tmp) {
		p54e_mark_dirty(ee);
		return p54e_entry_to_desc(tmp);
	} else {
		return NULL;
	}
}

static int __p54e_desc_add_head(struct p54e *ee, const struct pda_entry *desc,
		    struct p54e_list_entry *pos)
{
	struct p54e_list_entry *tmp;

	tmp = __p54e_entry_add_prepare(ee, desc);
	if (IS_ERR(tmp))
		return PTR_ERR(tmp);

	list_add_head(&tmp->h.list, &pos->h.list);
	return 0;
}

int __p54e_desc_add(struct p54e *ee, const struct pda_entry *desc)
{
	struct p54e_list_entry *iter = NULL;
	__le16 code = le16_to_cpu(desc->code);

	/* PDR_END is handled else where */
	if (code == PDR_END)
		return -EINVAL;

	/*
	 * sort all descriptor entries by ascending order.
	 * Normally, the list data-structure is fine, since
	 * the EEPROM content is nicely pre-sorted.
	 *
	 * But for more advanced modifications, we should
	 * move to a tree-based storage.
	 */
	p54e_walk_descs_reverse(iter, ee) {
		__le16 ic;

		ic = le16_to_cpu(iter->head.code);

		if (code == ic) {
			if (!allow_bad_eeproms)
				return -EINVAL;

			p54e_mark_dirty(ee);
			continue;
		}

		if (code > ic)
			goto insert;
	}

	iter = list_first_entry(&ee->desc_list, struct p54e_list_entry, h.list);

insert:
	return __p54e_desc_add_head(ee, desc, iter);

}

int p54e_desc_add(struct p54e *ee, const struct pda_entry *desc)
{
	int err;

	err = __p54e_desc_add(ee, desc);
	if (err == 0)
		p54e_mark_dirty(ee);

	return err;
}

void p54e_desc_unlink(struct p54e *ee, struct pda_entry *desc)
{
	p54e_entry_unlink(ee, p54e_desc_to_entry(desc));
}

void p54e_desc_del(struct p54e *ee, struct pda_entry *desc)
{
	p54e_entry_del(ee, p54e_desc_to_entry(desc));
}

void *p54e_desc_mod_len(struct p54e *ee __unused,
	struct pda_entry *desc, int len)
{
	struct p54e_list_entry *obj, tmp;
	int new_len = PDA_REAL_LEN(desc) + len;

	if (new_len < (int)sizeof(*desc))
		return ERR_PTR(-EINVAL);

	if (new_len % 2)
		return ERR_PTR(-EDOM);

	if (new_len > P54_MAX_EEPROM_LEN)
		return ERR_PTR(-E2BIG);

	obj = p54e_desc_to_entry(desc);

	memcpy(&tmp, obj, sizeof(tmp));
	obj = realloc(obj, new_len + sizeof(struct p54e_list_entry_head));
	if (obj == NULL)
		return ERR_PTR(-ENOMEM);

	list_replace(&tmp.h.list, &obj->h.list);

	desc = p54e_entry_to_desc(obj);
	desc->len = le16_to_cpu((new_len - 1) / 2);
	ee->desc_list_len += len;
	p54e_mark_dirty(ee);

	return desc;
}

const void *p54e_desc_next(struct p54e *ee, const struct pda_entry *pos)
{
	const struct p54e_list_entry *entry;

	if (!pos)
		entry = (const struct p54e_list_entry *) &ee->desc_list;
	else
		entry = p54e_desc_to_entry_c(pos);

	if (list_at_tail(entry, &ee->desc_list, h.list))
		return NULL;

	entry = (struct p54e_list_entry *) entry->h.list.next;

	return p54e_entry_to_desc_c(entry);
}

#define p54e_for_each_desc(desc, ee_desc)				\
        for (desc = ee_desc;						\
             desc->code != cpu_to_le16(PDR_END) &&			\
             le16_to_cpu(desc->len) >= (sizeof(struct pda_entry) / 2)	\
	     &&	le16_to_cpu(desc->len) < (P54_MAX_EEPROM_LEN / 2);	\
             desc = (void *)((unsigned long)desc + PDA_REAL_LEN(desc)))


#if BYTE_ORDER == LITTLE_ENDIAN
#define CRC16POLY_LE 0x8408

/* CRC16-CCITT  */
static uint16_t crc16_le(uint16_t crc, const unsigned char *p, unsigned int len)
{
	int i;
	while (len--) {
		crc ^= *p++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRC16POLY_LE : 0);
	}
	return crc;
}

#else
#error "this tool does not work with a big endian host yet!"
#endif

static int p54e_parse_descs(struct p54e *ee)
{
	const struct eeprom_pda_wrap *eeprom;
	const struct pda_entry *iter = NULL;
	const struct pda_entry *first;
	int err = -ENODATA;
	uint16_t crc16 = ~0;

	eeprom = (struct eeprom_pda_wrap *)ee->eeprom.data;
	first = (void *)((unsigned long)eeprom +
		sizeof(*eeprom) + le16_to_cpu(eeprom->len));

	p54e_for_each_desc(iter, first) {
		err = __p54e_desc_add(ee, iter);
		if (err)
			return err;

		crc16 = crc16_le(crc16, (const void *)iter, PDA_REAL_LEN(iter));
	}
	if (iter && iter->code == cpu_to_le16(PDR_END)) {
		/*
		 * PDR_END is added automatically by p54e_store.
		 * Therefore, just check the CRC and decide whenever
		 * to continue or bail-out.
		 */

		crc16 = ~crc16_le(crc16, (const void *)iter, 4);

		if (cpu_to_le16(crc16) != *((const __le16 *)iter->data)) {
			if (!allow_bad_eeproms)
				return -EBADMSG;

			p54e_mark_dirty(ee);
		}
	} else {
		return -ENOMSG;
	}

	return err;
}

static struct p54e *p54e_init(void)
{
	struct p54e *ee;

	ee = calloc(1, sizeof(*ee));
	if (!ee)
		return ERR_PTR(-ENOMEM);

	init_list_head(&ee->desc_list);
	p54e_clear_dirty(ee);

	return ee;
}

struct p54e *p54e_load_def(const unsigned char *data, size_t len)
{
	struct p54e *ee;
	int err;

	ee = p54e_init();
	if (IS_ERR_OR_NULL(ee))
		return ee;

	ee->eeprom.data = malloc(len);
	if (ee == NULL) {
		err = -ENOMEM;
		goto err_out;
	}

	memcpy(ee->eeprom.data, data, len);
	ee->eeprom.len = len;

	err = p54e_parse_descs(ee);
	if (err)
		goto err_out;

	return ee;

err_out:
	p54e_release(ee);
	return ERR_PTR(err);
}

struct p54e *p54e_load_file(const char *name)
{
	struct stat file_stat;
	FILE *fh;
	struct p54e *ee;
	int err;

	ee = p54e_init();
	if (IS_ERR_OR_NULL(ee))
		return ee;

	fh = fopen(name, "r");
	if (fh == NULL) {
		err = errno ? -errno : -1;
		goto err_out;
	}

	err = fstat(fileno(fh), &file_stat);
	if (err) {
		err = errno ? -errno : -1;
		goto err_out;
	}

	ee->eeprom.len = file_stat.st_size;
	ee->eeprom.data = malloc(ee->eeprom.len);
	if (ee->eeprom.data == NULL) {
		err = -ENOMEM;
		goto err_out;
	}

	err = fread(ee->eeprom.data, ee->eeprom.len, 1, fh);
	if (err != 1) {
		err = ferror(fh);
		goto err_out;
	}

	fclose(fh);

	if (p54e_set_filename(ee, name)) {
		err = -ENOMEM;
		goto err_out;
	}

	err = p54e_parse_descs(ee);
	if (err)
		goto err_out;

	return ee;

err_out:
	p54e_release(ee);
	return ERR_PTR(err);
}

static const struct pda_entry last_desc = {
	.len = cpu_to_le16(2),
	.code = PDR_END
};

int p54e_store(struct p54e *ee)
{
	struct eeprom_pda_wrap *head;
	struct p54e_list_entry *iter;
	FILE *fh;
	unsigned int elen;
	int err;
	uint16_t crc16 = ~0;

	if (!ee->eeprom.name)
		return -ENOENT;

	fh = fopen(ee->eeprom.name, "w");

	head = (void *) ee->eeprom.data;

	elen = le16_to_cpu(head->len) + sizeof(struct eeprom_pda_wrap);

	err = fwrite(ee->eeprom.data, elen, 1, fh);

	p54e_walk_descs(iter, ee) {
		elen = PDA_REAL_LEN(&iter->head);

		if (elen > P54_MAX_EEPROM_LEN) {
			err = -E2BIG;
			goto close_out;
		}

		crc16 = crc16_le(crc16, (const void *)iter->data, elen);

		err = fwrite(iter->data, elen, 1, fh);
		if (err != 1) {
			err = ferror(fh);
			goto close_out;
		}
	}

	err = fwrite(&last_desc, sizeof(last_desc), 1, fh);
	if (err != 1) {
		err = ferror(fh);
		goto close_out;
	}

	/* Note: the result has to be inverted */
	crc16 = ~crc16_le(crc16, (const uint8_t *)&last_desc, sizeof(last_desc));

	err = fwrite(&crc16, sizeof(crc16), 1, fh);
	if (err != 1) {
		err = ferror(fh);
		goto close_out;
	}

	err = 0;
	p54e_clear_dirty(ee);

close_out:
	fclose(fh);
	return err;
}

unsigned int p54e_get_descs_num(struct p54e *ee)
{
	return ee->desc_list_entries;
}

unsigned int p54e_get_descs_size(struct p54e *ee)
{
	return ee->desc_list_len;
}

void p54e_allow_tainted_eeproms(void)
{
	allow_bad_eeproms = 1;
}

int p54e_set_filename(struct p54e *ee, const char *file)
{
	ee->eeprom.name = strdup(file);
	if (ee->eeprom.name == NULL)
		return -ENOMEM;

	return 0;
}

char *p54e_get_filename(struct p54e *ee)
{
	return ee->eeprom.name;
}

int p54e_is_dirty(struct p54e *ee)
{
	return ee->modified;
}
