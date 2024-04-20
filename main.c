#define VERSION "v1.0"

#include <assert.h>
#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static char *
gph_item(char item)
{
	/* NOTE(irek): Having common item strings with the same length
	 * produce nicly aligned output. */
	switch (item) {
	case '.': return "EOF";         /* End Of gopher submenu File*/
	/* Canonical */
	case '0': return "TXT" ;        /* Text file */
	case '1': return "GPH";         /* Gopher submenu */
	case '2': return "CSO";         /* CSO protocol */
	case '3': return "ERR";         /* Error code returned by server */
	case '4': return "BINHEX";      /* BinHex-encoded file (for Macintosh) */
	case '5': return "DOS";         /* DOS file */
	case '6': return "UUENCODED";   /* uuencoded file */
	case '7': return "SEARCH";      /* Gopher full-text search */
	case '8': return "TELNET";      /* Telnet */
	case '9': return "BIN";         /* Binary file */
	case '+': return "MIRROR";      /* Mirror or alternate server */
	case 'g': return "GIF";         /* GIF file */
	case 'I': return "IMG";         /* Image file */
	case 'T': return "TN3270";      /* Telnet 3270 */
	/* Gopher+ */
	case ':': return "BMP";         /* Bitmap image */
	case ';': return "MOV";         /* Movie/video file */
	case '<': return "MP3";         /* Sound file */
	/* Non-canonical */
	case 'd': return "DOC";         /* Doc. Seen used alongside PDF's and .DOC's */
	case 'h': return "WEB";         /* HTML file */
	case 'p': return "PNG";         /* Image file "(especially the png format)" */
	case 'r': return "RTF";         /* Document rtf file ("rich text format") */
	case 's': return "WAV";         /* Sound file (especially the WAV format) */
	case 'P': return "PDF";         /* document pdf file */
	case 'X': return "XML";         /* document xml file */
	case 'i': return "   ";         /* Informational message, widely used */
	}
	return "UNKNOWN";
}

static char *
gph_uri(char *bp)
{
	static char item, host[1024], port[16], path[2048], buf[4096] = "";
	size_t sz;
	item = *bp;
	if (item == 'h') {	/* Special case for HTML web link */
		bp += strcspn(bp, "\t\n\0") + 5;
		sz = strcspn(bp, "\t\n\0") - 1;
		strncpy(buf, bp, sz);
		buf[sz] = 0;
		return buf;
	}
	bp += strcspn(bp, "\t\n\0") + 1;
	snprintf(path, sizeof(path), "%.*s", (int)(sz = strcspn(bp, "\t\n")), bp);
	bp += sz + 1;
	snprintf(host, sizeof(host), "%.*s", (int)(sz = strcspn(bp, "\t\n")), bp);
	bp += sz + 1;
	snprintf(port, sizeof(port), "%.*s", (int)strspn(bp, "0123456789"), bp);
	snprintf(buf, sizeof(buf), "gopher://%s:%s/%c%s", host, port, item, path);
	return buf;
}

/* Only wrap lines according to WRAP line width and TAB width. */
static void
fmt_none(char *str, size_t sz, int wrap, int tab)
{
	size_t i;
	int w;
	assert(str);
	assert(sz > 0);
	for (w=0, i=0; i<sz; i++) {
		putchar(str[i]);
		switch (str[i]) {
		case '\n':
		case '\r':
		case '\0':
			w = 0;
			continue;
		case '\t':
			w += tab - (w % tab);
			break;
		default:
			w++;
		}
		if (w >= wrap) {
			w = 0;
			/* Put new line on wrap only if it is not
			 * followed by another new line char. */
			if (i+1>=sz || str[i+1] != '\n') {
				putchar('\n');
			}
			/* Trime left after wrapping. */
			while (i+1 < sz &&
			       (str[i+1] == ' ' || str[i+1] == '\t')) {
				i++;
			}
		}
	}
}

/* Simple Gopher submenu formatter. */
static void
fmt_gph(char *str)
{
	size_t len;
	char *bp, item;
	int i;                  /* Menu item index */
	assert(str);
	for (i=0, bp=str; *bp && *bp != '.'; bp += strcspn(bp, "\n\0") + 1) {
		item = *bp;
		printf("%s ", gph_item(item));
		bp++;
		len = strcspn(bp, "\t\n\0");
		printf("%.*s", (int)len, bp);
		if (item != 'i') {
			printf(" [%d]", ++i);
		}
		putchar('\n');
	}
	putchar('\n');
	/* Print links */
	for (i=0, bp=str; *bp && *bp != '.'; bp += strcspn(bp, "\n\0") + 1) {
		if (*bp == 'i') {
			continue;
		}
		printf("[%d] %s\n", ++i, gph_uri(bp));
	}
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
		/* TODO(irek): Implement. */
		assert(0 && "Not implemented");
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
