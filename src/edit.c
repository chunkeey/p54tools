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
#include <ctype.h>
#include "p54eeprom.h"

#include "p54.h"
#include "compiler.h"

#include "dbg.c"

static void checksum_help(void)
{
	fprintf(stderr, "\nDescription:\n");
	fprintf(stderr, "\tEdit EEPROM fields.\n");

	fprintf(stderr, "\nParameteres:\n");
	fprintf(stderr, "\t -m [1-4]\t\t\t = switch to machine output.\n");
	fprintf(stderr, "\t -e [IIII,O:XYZ...]\t = Add/Edit ID.\n");
	fprintf(stderr, "\t -r [IIII]\t\t = remove ID.\n");
	fprintf(stderr, "\t -l [IIII]\t\t = print ID content.\n");
	fprintf(stderr, "\t -d\t\t\t = lists all descriptors.\n");
	fprintf(stderr, "\t 'EEPROM-FILE'\t\t = eeprom file name\n");
	fprintf(stderr, "\n");
}

struct list_entry {
	struct list_head head;
	unsigned char op;
	unsigned int id;
	size_t data_len;
	union {
		char data_text[0];
		unsigned char data_misc[0];
	};
};

static struct p54e *ee;
static LIST_HEAD(work_list);
static int output;

enum output_modes {
	HUMAN		= 0,
	HEXCHAR,
	SEPHEXCHAR,
	HEADER,
	BINARY,
	__OUTPUT_NUM,
};

static ssize_t parser(struct list_entry *e)
{
	int i, start, l;

	if (e->id > 0xffff)
		return -EBADMSG;

	if (e->op == 'l' || e->op == 'r') {
		e->data_len = 0;
		return 0;
	}

	l = strlen(e->data_text);
	if (l < 3)
		return -EMSGSIZE;

	for (i = 0; i < l; i++)
		if (isalpha(e->data_text[i]))
			break;

	switch (e->data_text[i++]) {
	case 's':
		for (start = i; i < l; i++)
			e->data_text[i - start] = e->data_text[i];

		e->data_len = l - start;
		break;

	case 'h': {
		unsigned int num, nums, has;
		char *p;

		has = 0;
		num = 0;
		nums = 0;
		for (start = i, p = (char *)&e->data_text[i]; i < l; i++, p++) {
			if (isxdigit(*p)) {
				unsigned int t;

				has++;

				sscanf(p, "%1x", &t);
				num = num << 4 | t;
				if ((has % 2) == 0) {
					e->data_text[nums++] = num;
					num = 0;
				}
			} else if (*p == 'x' || *p == 'X') {
				has--;
				num >>= 8;
			} else if (*p == ',' || *p == ' ' || *p == ':') {
			} else {
				return -EINVAL;
			}
		}

		e->data_len = nums;
		}
		break;

	default:
		return -EBADRQC;
	}

	if (e->data_len == 0)
		return -ENODATA;

	if (e->data_len % 2)
		return -EINVAL;

	e->data_text[e->data_len + 1] = 0;

	return 0;
}

char *parse_options(const int argc, char *args[])
{
	struct list_entry *entry;
	int err = -EINVAL, opt;

	while ((opt = getopt(argc, args, "m:dr:e:l:")) != -EXIT_FAILURE) {
		switch (opt) {
		case 'm':
			err = 0;
			if (sscanf(optarg, "%d", &output) != 1)
				err = -EINVAL;

			if (output >= __OUTPUT_NUM)
				err = -EINVAL;

			break;

		case 'd':
			err = 0;
			entry = malloc(sizeof(*entry));
			entry->op = opt;
			list_add_tail(&entry->head, &work_list);
			break;

		case 'l':
		case 'r':
		case 'e': {

			/*
			 * rule of thumb, we need at least the same
			 * amout of space to store the data as the
			 * argument is long.
			 */

			entry = malloc(sizeof(*entry) + strlen(optarg));
			if (sscanf(optarg, "%4x%s", &entry->id, entry->data_text) < 1) {
				err = -EINVAL;
				break;
			}

			entry->op = opt;

			err = parser(entry);
			if (err)
				break;

			list_add_tail(&entry->head, &work_list);
			break;
			}

		default:
			err = -EBADR;
			break;
		}

		if (err)
			goto err_out;
	}

	if (err) {
		goto err_out;
	}

	return args[optind];

err_out:
	return ERR_PTR((err));
}

static void display_entry(const struct pda_entry *entry)
{
	if (output != HUMAN) {
		int i;

		for (i = 0; i < PDA_REAL_LEN(entry); i++) {
			switch (output) {
			case HEXCHAR:
				fprintf(stdout, "%.2x", *(((const uint8_t *)entry) + i));
				break;
			case SEPHEXCHAR:
				fprintf(stdout, "%.2x ", *(((const uint8_t *)entry) + i));
				break;
			case HEADER:
				fprintf(stdout, "0x%.2x, ", *(((const uint8_t *)entry) + i));
				break;
			case BINARY:
				fprintf(stdout, "%c", *(((const uint8_t *)entry) + i));
				break;
			}
		}

		switch (output) {
		case HEADER:
		case HEXCHAR:
		case SEPHEXCHAR:
			fprintf(stdout, "\n");
			break;

		case BINARY:
			fputc(0, stdout);
			break;
		}
	} else {
		fprintf(stdout, "Entry: %x, Length: %d\n",
			entry->code, PDA_DATA_LEN(entry));
		print_hex_dump_bytes("\t", entry->data, PDA_DATA_LEN(entry));
	}
}

static int show_entry(unsigned int id)
{
	const struct pda_entry *entry;

	entry = p54e_desc_find(ee, id);
	if (!entry)
		return -ENODATA;

	display_entry(entry);
	return 0;
}

static int del_entry(unsigned int id)
{
	struct pda_entry *entry;

	entry = p54e_desc_find_mod(ee, id);
	if (!entry)
		return -ENODATA;

	p54e_desc_del(ee, entry);
	return 0;
}

static int mod_entry(unsigned int id, const void *data, unsigned int len)
{
	struct pda_entry *entry;

	entry = p54e_desc_find_mod(ee, id);
	if (entry) {
		int newlen;

		newlen = len - PDA_DATA_LEN(entry);

		entry = p54e_desc_mod_len(ee, entry, newlen);
		if (IS_ERR_OR_NULL(entry))
			return PTR_ERR(entry);

		memcpy(entry->data, data, len);
	} else {
		int err;

		entry = malloc(sizeof(*entry) + ALIGN(len, 2));
		if (!entry)
			return -ENOMEM;

		entry->code = cpu_to_le16(id);
		entry->len = 1 + ALIGN(len, 2) / 2;

		memcpy(entry->data, data, len);

		err = p54e_desc_add(ee, entry);
		if (err)
			return err;
	}

	return 0;
}

static void display_all(void)
{
	const struct pda_entry *iter = NULL;

	while ((iter = p54e_desc_next(ee, iter))) {
		display_entry(iter);
	}
}

static int work(void)
{
	struct list_entry *iter;
	int err;

	list_for_each_entry(iter, &work_list, head) {
		switch (iter->op) {
		case 'l':
			err = show_entry(iter->id);
			break;
		case 'r':
			err = del_entry(iter->id);
			break;
		case 'e':
			err = mod_entry(iter->id, iter->data_misc, iter->data_len);
			break;
		case 'd':
			display_all();
			err = 0;
			break;

		default:
			err = -EINVAL;
			break;
		}

		if (err) {
			switch (output) {
			case HUMAN:
				fprintf(stdout, "error during operation (%d)\n", err);
				break;
			case HEADER:
			case HEXCHAR:
			case SEPHEXCHAR:
				fprintf(stdout, "\n");
				break;
			case BINARY:
				fputc(0, stdout);
			}

			return err;
		}
	}

	return 0;
}

static void free_work(void)
{
	struct list_entry *iter;

	while (!list_empty(&work_list)) {
		iter = list_first_entry(&work_list, struct list_entry, head);
		list_del(&iter->head);
		free(iter);
	}
}

int main(const int argc, char *args[])
{
	int err = 0;
	char *filename;

	filename = parse_options(argc, args);
	if (IS_ERR_OR_NULL(filename)) {
		err = PTR_ERR(filename);
		fprintf(stderr, "problem while parsing arguments %d.\n", err);
		goto out;
	}

	ee = p54e_load_file(filename);
	if (IS_ERR_OR_NULL(ee)) {
		err = PTR_ERR(ee);
		fprintf(stderr, "Failed to open file \"%s\" (%d).\n", filename, err);
		goto out;
	}

	err = work();
	if (err)
		goto out;

	if (p54e_is_dirty(ee)) {
		err = p54e_store(ee);
		if (err) {
			fprintf(stderr, "Failed to apply changes (%d).\n", err);
			goto out;
		}
	}

out:
	free_work();

	switch (err) {
	case 0:
		break;
	case -EINVAL:
		checksum_help();
		break;
	default:
		break;
	}

	p54e_release(ee);
	return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
