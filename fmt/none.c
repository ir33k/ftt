#include <assert.h>
#include <stdio.h>
#include "none.h"

/* Only wrap lines according to WRAP line width and TAB width. */
void
fmt_none(char *str, size_t sz, int wrap, int tab)
{
	size_t i;
	int w;
	assert(str);
	assert(sz > 0);
	assert(wrap > 0);
	assert(tab > 0);
	for (w=0, i=0; i<sz; i++) {
		putchar(str[i]);
		switch (str[i]) {
		case '\r':
			continue;
		case '\n':
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
