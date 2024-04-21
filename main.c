#define VERSION "v1.0"

#include <assert.h>
#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt/none.h"
#include "fmt/gph.h"
#include "fmt/gmi.h"

#define WRAP	72	/* Default value of bytes for wrapping lines */
#define TAB	8	/* Default tab width */

enum fmt { NONE=0, GPH, GMI };

static void
usage(char *arg0)
{
	assert(arg0);
	printf("usage: %s [-w num] [-t num] [-v] [-h] [(none|gph|gmi)] <stdin\n"
	       "\n"
	       "	-w	Lines wrap (%d).\n"
	       "	-t	Tab width (%d).\n"
	       "	-v	Print program version.\n"
	       "	-h	Print this help message.\n"
	       , arg0, WRAP, TAB);
}

/* Allocate FP file stream as string to OUT, return string length. */
static size_t
falloc(FILE *fp, char **out)
{
	const size_t chunk = 4096;
	size_t sz, len = 0;
	assert(fp);
	assert(out);
	if (!(*out = malloc(chunk))) {
		err(1, "Failed to allocate %lu", chunk);
	}
	while ((sz = fread(*out + len, 1, chunk - 1, fp))) {
		len += sz;
		if (sz == chunk - 1 && !(*out = realloc(*out, len + chunk))) {
			err(1, "Failed to reallocate %lu", len + chunk);
		}
	}
	memcpy(*out + len, "\0", 1);	/* Null terminate */
	return len;
}

static enum fmt
get_fmt(char *name)
{
	assert(name);
	if (!strcmp(name, "gph")) return GPH;
	if (!strcmp(name, "gmi")) return GMI;
	return NONE;
}

static void
run(enum fmt fmt, int wrap, int tab)
{
	char *str;
	size_t sz;
	assert(wrap > 0);
	assert(tab > 0);
	sz = falloc(stdin, &str);
	switch (fmt) {
	case NONE:
		fmt_none(str, sz, wrap, tab);
		break;
	case GPH:
		/* TOOD(irek): Should I wrap lines? */
		fmt_gph(str);
		break;
	case GMI:
		fmt_gmi(str, sz, wrap, tab);
		break;
	}
}

int
main(int argc, char **argv)
{
	int i, wrap=WRAP, tab=TAB;
	enum fmt fmt=0;
	while ((i = getopt(argc, argv, "w:t:vh")) != -1) {
		switch (i) {
		case 'w':
			if ((wrap = atoi(optarg)) < 0) {
				errx(1, "-w has to be bigger than 0");
			}
			break;
		case 't':
			if ((tab = atoi(optarg)) < 0) {
				errx(1, "-t has to be bigger than 0");
			}
			break;
		case 'v':
			puts(VERSION);
			return 0;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}
	if (argc - optind > 0) {
		fmt = get_fmt(argv[optind]);
	}
	run(fmt, wrap, tab);
	return 0;
}
