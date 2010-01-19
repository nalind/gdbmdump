/*
 * Copyright 2008,2010 Red Hat, Inc.
 *
 * This Program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This Program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this Program; if not, write to the
 *
 *   Free Software Foundation, Inc.
 *   59 Temple Place, Suite 330
 *   Boston, MA 02111-1307 USA
 *
 */

/* Dump content from a gdbm database in a format resembling that which is used
 * by db_dump, so that it can be read by db_load. */
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gdbm.h>

static int pflag = 0;
static int yflag = 0;

static void
dump_start(int total, const char *name)
{
	printf("VERSION=3\n");
	if (total > 1) {
		printf("database=%s\n", name);
	}
	if (pflag) {
		printf("format=print\n");
	}
	printf("type=hash\n");
	printf("HEADER=END\n");
}
static void
puthex(unsigned char c)
{
	const char chars[] = "0123456789abcdef";
	putchar(chars[c >> 4]);
	putchar(chars[c & 0xf]);
}
static void
dump_entry(datum key, datum value)
{
	int i;
	if (yflag) {
		if ((key.dsize >= 3) && (memcmp(key.dptr, "YP_", 3) == 0)) {
			return;
		}
	}
	if (pflag) {
		putchar(' ');
		for (i = 0; i < key.dsize; i++) {
			if (isprint(key.dptr[i]) &&
			    (key.dptr[i] != ' ') &&
			    (key.dptr[i] != '\t') &&
			    (key.dptr[i] != '\\')) {
				putchar(key.dptr[i]);
			} else {
				putchar('\\');
				puthex(key.dptr[i]);
			}
		}
		putchar('\n');
		putchar(' ');
		for (i = 0; i < value.dsize; i++) {
			if (isprint(value.dptr[i]) &&
			    (value.dptr[i] != ' ') &&
			    (value.dptr[i] != '\t') &&
			    (value.dptr[i] != '\\')) {
				putchar(value.dptr[i]);
			} else {
				putchar('\\');
				puthex(value.dptr[i]);
			}
		}
		putchar('\n');
	} else {
		putchar(' ');
		for (i = 0; i < key.dsize; i++) {
			puthex(key.dptr[i]);
		}
		putchar('\n');
		putchar(' ');
		for (i = 0; i < value.dsize; i++) {
			puthex(value.dptr[i]);
		}
		putchar('\n');
	}
}
static void
dump_end()
{
	printf("DATA=END\n");
}

int
main(int argc, char **argv)
{
	GDBM_FILE dbf;
	datum key, next, value;
	int i;

	while ((i = getopt(argc, argv, "py")) != -1) {
		switch (i) {
		case 'p':
			pflag = 1;
			break;
		case 'y':
			yflag = 1;
			break;
		default:
			fprintf(stderr, "Usage: %s [-p] [-y] file [...]\n",
				strchr(argv[0], '/') ?
				strrchr(argv[0], '/') + 1 : argv[0]);
			return 1;
			break;
		}
	}
	for (i = optind; i < argc; i++) {
		dbf = gdbm_open(argv[i], 0, GDBM_READER, 0600, NULL);
		if (dbf == NULL) {
			fprintf(stderr, "Error opening `%s': %s\n", argv[i],
				gdbm_errno ? gdbm_strerror(gdbm_errno) :
				strerror(errno));
			return 1;
		}
		dump_start(argc - optind, argv[i]);
		key = gdbm_firstkey(dbf);
		while (key.dptr != NULL) {
			value = gdbm_fetch(dbf, key);
			if (value.dptr != NULL) {
				dump_entry(key, value);
				free(value.dptr);
			}
			next = gdbm_nextkey(dbf, key);
			free(key.dptr);
			key = next;
		}
		dump_end();
		gdbm_close(dbf);
	}
	return 0;
}
