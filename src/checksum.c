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
#include <getopt.h>

#include "p54eeprom.h"

#include "compiler.h"

static void checksum_help(void)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\tchecksum [-f] EEPROM-FILE\n");

	fprintf(stderr, "\nDescription:\n");
	fprintf(stderr, "\tThis simple utility adds/updates all integrety "
			"checksums.\n");

	fprintf(stderr, "\nParameteres:\n");
	fprintf(stderr, "\t 'EEPROM-FILE'\t = eeprom file name\n");
	fprintf(stderr, "\t '-f'\t\t = fix bad eeprom checksum\n");
	fprintf(stderr, "\n");
}

int main(int argc, char *args[])
{
	struct p54e *ee = NULL;
	int err = 0, opt;
	char *filename;

	while ((opt = getopt(argc, args, "f")) != -EXIT_FAILURE) {
		switch (opt) {
		case 'f':
			fprintf(stdout, "ignore bad checksums.\n");
			p54e_allow_tainted_eeproms();
			break;
		default:
			err = -EINVAL;
			goto out;
		}
	}

	if (optind >= argc) {
		err = -EINVAL;
		goto out;
	}

	filename = args[optind];

	ee = p54e_load_file(filename);
	if (IS_ERR_OR_NULL(ee)) {
		err = PTR_ERR(ee);
		fprintf(stderr, "Failed to open file \"%s\" (%d).\n",
			filename, err);
		goto out;
	}

	/*
	 * No magic here, The checksum descriptor is added/update
	 * automatically in a subroutine of p54e_store().
	 *
	 * This tools serves as a skeleton/example.
	 */
	err = p54e_store(ee);
	if (err) {
		fprintf(stderr, "Failed to apply checksum (%d).\n", err);
		goto out;
	}

out:
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
