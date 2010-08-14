#include <stdio.h>
#include <ctype.h>

static void print_hex_dump_bytes(const char *pre, const void *buf,
			  size_t len)
{
	char line[58];
	char str[17] = { 0 };
	const unsigned char *tmp = (const void *) buf;
	char *pbuf = line;
	size_t i;

	for (i = 0; i < len; i++) {
		if (i % 16 == 0) {
			if (pbuf != line) {
				printf("%s%s: %s\n", pre ? pre : "", line, str);
				pbuf = line;
			}

			pbuf += sprintf(pbuf, "0x%04lx: ", (unsigned long)i);
		}

		pbuf += sprintf(pbuf, "%.2x ", tmp[i]);
		str[i % 16] = (isprint(tmp[i]) && isascii(tmp[i])) ? tmp[i] : '.';
	}

	if (pbuf != line) {
		if ((i % 16)) {
			str[i % 16] = '\0';

			for (i = 16 - (i % 16); i != 0; i--)
				pbuf += sprintf(pbuf, "   ");
		}

		printf("%s%s: %s\n", pre, line, str);
	}
}
