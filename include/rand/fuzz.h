#ifndef RAND_FUZZ_H
#define RAND_FUZZ_H

char
fuzzchar(const char *set);

char
fuzzcchar(const char *set);

void
fuzzstr(char *buf, size_t maxlen,
	char (*c)(const char *), const char *set);

#endif

