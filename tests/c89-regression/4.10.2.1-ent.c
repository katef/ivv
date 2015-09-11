#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h> /* XXX */
#include <math.h>

#include <stats/randtest.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int
main(int argc, char *argv[])
{
	struct rt_stats r;
	int i, n = 12345678; /* arbitary sample size */

	srand(0);

	rt_init(0);

	for (i = 0; i <= n; i++) {
		int r;

		r = rand();

		rt_add(&r, sizeof r);
	}

	rt_end(&r);

	{
		struct {
			double epi, ent, mean, scc, chisq, chip;
		} expected;

		/* ideal entropy, in bits per byte */
		double ideal = log2(RAND_MAX + 1.0) / sizeof (int);

		/* Acceptable ranges picked experimentally, based on okayish PRNGs */

		expected.epi  = 0.20;  /* acceptable error for calculating Pi */
		expected.ent  = ideal * 0.80; /* minimum acceptable entropy */
		expected.mean = 100.0; /* abs of mean; 127.5 is random */
		expected.scc  = -0.1;  /* 0.0 is totally uncorrelated */

		/* TODO: print TAP instead */
		assert(r.scc >= -99999 || !"all values equal");
		assert(r.scc > expected.scc);
		assert(r.ent > expected.ent);
		assert(fabs(r.mean) > expected.mean);
		assert(fabs(M_PI - r.montepi) / M_PI < expected.epi);
		assert(expected.chisq < 10000000.00);
		assert(expected.chip  < 10.0);
	}

	return 0;
}

