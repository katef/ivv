/*
 * Copyright 2012-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <limits.h>
#include <string.h>

#include <rand/rand.h>
#include <rand/fuzz.h>

char
fuzzchar(const char *set)
{
	assert(set != NULL);

	return set[rand_interval(0, strlen(set))];
}

char
fuzzcchar(const char *set)
{
	int c;

	assert(set != NULL);

	do {
		c = rand_interval(0, UCHAR_MAX);
	} while (c != '\0' && strchr(set, c));

	return c;
}

void
fuzzstr(char *buf, size_t maxlen,
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

