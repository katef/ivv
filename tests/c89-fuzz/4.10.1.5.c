#define _POSIX_SOURCE

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include <dlfcn.h>

static void
escputc(int c, FILE *f)
{
	size_t i;

	struct {
		int in;
		const char *out;
	} esc[] = {
		{ '\"', "\\\"" },
		{ '\'', "\\\'" },
		{ '\\', "\\\\" },
		{ '\f', "\\f"  },
		{ '\n', "\\n"  },
		{ '\r', "\\r"  },
		{ '\t', "\\t"  },
		{ '\v', "\\v"  }
	};

	assert(f != NULL);

	for (i = 0; i < sizeof esc / sizeof *esc; i++) {
		if (esc[i].in == c) {
			fputs(esc[i].out, f);
			return;
		}
	}

	if (!isprint(c)) {
		fprintf(f, "\\x%x", (unsigned char) c);
		return;
	}

	putc(c, f);
}

static void
escputs(const char *s, FILE *f)
{
	const char *p;

	for (p = s; *p != '\0'; p++) {
		escputc(*p, f);
	}
}

/*
 * Returns in the interval [min, max]
 * adapted from http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
 */
unsigned int
rand_interval(unsigned int min, unsigned int max)
{
	unsigned range;
	unsigned buckets;
	unsigned limit;
	int r;

	assert(max >= min);
	assert(1 + max - min <= RAND_MAX);

	/*
	 * Create equal size buckets all in a row, then fire randomly towards
	 * the buckets until you land in one of them. All buckets are equally
	 * likely. If you land off the end of the line of buckets, try again.
	 */

	range   = 1 + max - min;
	buckets = RAND_MAX / range;
	limit   = buckets  * range;

	do {
		r = rand();
	} while (r >= limit);

	return min + r / buckets; /* truncates intentionally */
}

static char
randchar(const char *set)
{
	assert(set != NULL);

	return set[rand_interval(0, strlen(set))];
}

static char
randcchar(const char *set)
{
	int c;

	assert(set != NULL);

	do {
		c = rand_interval(0, UCHAR_MAX);
	} while (c != '\0' && strchr(set, c));

	return c;
}

static void
randstr(char *buf, size_t maxlen,
	char (*c)(const char *), const char *set)
{
	size_t i, l;

	assert(c != NULL);
	assert(set != NULL);

	l = rand_interval(0, maxlen);

	for (i = 0; i < l; i++) {
		buf[i] = c(set);
	}

	buf[i] = '\0';
}

int main(int argc, char *argv[]) {
	/*
	 * S4.10.1.5 The strtol function
	 */

	char white[32];
	char sign[1];
	char num[256];
	char tail[16];
	int base;
	int seed;

	static const char alnum[] = "0123456789" "abcdefghijklmnopqrstuvwxyz";
	char digit[sizeof alnum];

	char buf[sizeof white + sizeof sign + sizeof num + sizeof tail + 1];

	{
		const char *s;

		s = getenv("SEED");
		if (s == NULL) {
			seed = 1;
		} else {
			seed = atoi(s);
		}

		if (seed == 0) {
			fprintf(stderr, "invalid seed: %s\n", s);
			return 1;
		}

		srand(seed);
	}

	/*
	 * S4.10.1.5p1 "... an initial, possibly empty,
	 * sequence of white-space characters ..."
	 */
	randstr(white, sizeof white, randchar, " \f\n\r\t\v");

	/*
	 * S4.10.1.5p1 "... (as specified by the isspace function)"
	 */
	{
		const char *p;

		for (p = white; *p != '\0'; p++) {
			assert(isspace((unsigned char) *p));
		}
	}

	/*
	 * S4.10.1.5p2 "... optionally preceded by a plus or minus sign ..."
	 */
	randstr(sign, sizeof sign, randchar, "+-");

	/*
	 * S4.10.1.5p1 "... a subject sequence resembling an unsigned integer
	 * represented in some radix determined by the value of base ..."
	 */
	{
		size_t i;

		for (i = 0; i < sizeof alnum; i++) {
			digit[i] = rand_interval(0, 1) ? toupper(alnum[i]) : tolower(alnum[i]);
		}

		base = rand_interval(1, sizeof digit);
		digit[base] = '\0';

		randstr(num, sizeof num, randchar, digit);
	}

	/*
	 * S4.10.1.5p1 "... and a final string of one or more
	 * unrecognized characters, including the terminating
	 * null character of the input string."
	 */
	/* An unrecognised character is something which is not
	 * a digit for the given base (or '\0') */
	randstr(tail, sizeof tail, randcchar, digit);

	sprintf(buf, "%s%s%s%s", white, sign, num, tail);

	{
		struct {
			long int (*strtol)(const char *, char **, int);
			void *h;

			long l;
			char *e;
			int err;
		} ref, dut;

		typedef long int (strtol_t)(const char *, char **, int);
		int end;

		if (argc != 2) {
			fprintf(stderr, "usage: <dut.so>\n");
			return 1;
		}

		ref.h = dlopen(NULL, RTLD_LAZY);
		if (ref.h == NULL) {
			fprintf(stderr, "dlopen: %s\n", dlerror());
			return 1;
		}

		dut.h = dlopen(argv[1], RTLD_LAZY);
		if (dut.h == NULL) {
			fprintf(stderr, "dlopen: %s\n", dlerror());
			return 1;
		}

		ref.strtol = (strtol_t *) dlsym(dut.h, "strtol");
		dut.strtol = (strtol_t *) dlsym(dut.h, "strtol");

		if (dut.strtol == NULL) {
			fprintf(stderr, "dlsym: %s\n", dlerror());
			return 1;
		}

		end = rand_interval(0, 1);
		if (!end) {
			ref.e = NULL;
			dut.e = NULL;
		}

		errno = 0;
		ref.l = ref.strtol(buf, end ? &ref.e : NULL, base);
		ref.err = errno;

		errno = 0;
		dut.l = dut.strtol(buf, end ? &dut.e : NULL, base);
		dut.err = errno;

		{
			int status;

			status = ref.l == dut.l
				&& ref.e == dut.e
				&& ref.err == dut.err;

			printf("%s %u - strtol fuzz $SEED=%d\n",
				status ? "ok" : "not ok", (unsigned int) seed, seed);

			printf("\tinput    buf   = \"");
			escputs(buf, stdout);
			printf("\"\n");

			printf("\tinput    end   = %s\n", end ? "&e" : "NULL", base);

			printf("\tinput    base  = %d\n", base);

			printf("\texpected l     = %ld\n", ref.l);
			if (ref.e == NULL) {
				printf("\texpected end   = NULL\n");
			} else {
				printf("\texpected end   = \"");
				escputs(ref.e, stdout);
				printf("\"\n");
			}
			printf("\texpected errno = %d (%s)\n", ref.err, strerror(ref.err));

			printf("\tgot      l     = %ld\n", dut.l);
			if (dut.e == NULL) {
				printf("\tgot      end   = NULL\n");
			} else {
				printf("\tgot      end   = \"");
				escputs(dut.e, stdout);
				printf("\"\n");
			}
			printf("\tgot      errno = %d (%s)\n", dut.err, strerror(ref.err));
		}

		dlclose(ref.h);
		dlclose(dut.h);
	}

	return 0;
}

