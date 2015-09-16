#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include <rand/rand.h>

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

