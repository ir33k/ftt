#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "gph.h"

static char *
item_label(char item)
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
normalize_uri(char *bp)
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

void
fmt_gph(char *str)
{
	size_t len;
	char *bp, item;
	int i;                  /* Menu item index */
	assert(str);
	for (i=0, bp=str; *bp && *bp != '.'; bp += strcspn(bp, "\n\0") + 1) {
		item = *bp;
		printf("%s ", item_label(item));
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
		printf("[%d] %s\n", ++i, normalize_uri(bp));
	}
}
