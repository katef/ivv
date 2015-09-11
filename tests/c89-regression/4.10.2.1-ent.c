#define _XOPEN_SOURCE 600

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

#include <stats/randtest.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if WORKAROUND

/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */

#include <stdint.h>

#undef RAND_MAX
#define RAND_MAX UINT32_MAX

struct pcg_state_setseq_64 {
	uint64_t state;
	uint64_t inc;
};
typedef struct pcg_state_setseq_64 pcg32_random_t;

static uint32_t
pcg32_random_r(pcg32_random_t* rng)
{
	uint64_t oldstate = rng->state;
	rng->state = oldstate * 6364136223846793005ULL + rng->inc;
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static void
pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq)
{
	rng->state = 0U;
	rng->inc = (initseq << 1u) | 1u;
	pcg32_random_r(rng);
	rng->state += initstate;
	pcg32_random_r(rng);
}

#endif

static double
error(double experimental, double theoretical)
{
	return fabs((experimental - theoretical) / theoretical);
}

int
main(int argc, char *argv[])
{
	struct rt_stats r;
	int i, n = 12345678; /* arbitary sample size */

#if WORKAROUND
	pcg32_random_t pcg32;
#endif

#if WORKAROUND
	pcg32_srandom_r(&pcg32, 0, 1);
#else
	srand(0);
#endif

	rt_init(0);

	for (i = 0; i <= n; i++) {
		int r;

#if WORKAROUND
		r = pcg32_random_r(&pcg32);
#else
		r = rand();
#endif

		rt_add(&r, sizeof r);
	}

	rt_end(&r);

	{
		/* TODO: rt_stats struct */
		struct {
			double epi, ent, mean, scc, chisq, chip;
		} expected;

		/* ideal entropy, in bits per byte */
		double ideal = log2(RAND_MAX + 1.0) / sizeof (int);

		/* Acceptable ranges picked experimentally, based on okayish PRNGs */

		expected.epi  = 0.20;  /* acceptable error for calculating Pi */
		expected.ent  = ideal * 0.80; /* minimum acceptable entropy */
		expected.mean = 0.20; /* percentage error of mean */
		expected.scc  = -0.1;  /* 0.0 is totally uncorrelated */
		expected.chisq = 10000000.00;
		expected.chip  = 0.1;

		printf("1..6\n");

		printf("%s 1 - Serial correlation coefficient\n",
			r.scc > expected.scc ? "ok" : "not ok");
		printf("\texpected scc > %1.6f # %s\n", expected.scc,
			"0.0 = totally uncorrelated");
		if (r.scc >= -99999) {
			printf("\tgot scc = %1.6f\n", r.scc);
		} else {
			printf("\tgot %s\n", "all values equal");
		}

		printf("%s 2 - Entropy\n"
			"\texpected ent > %1.6f # bits per byte\n"
			"\tgot ent = %1.6f\n"
			"\tideal = %1.6f\n",
			r.ent > expected.ent ? "ok" : "not ok",
			expected.ent,
			r.ent,
			ideal);

		printf("%s 3 - Arithmetic mean\n"
			"\texpected mean = 127.5 +/- %0.2f%%\n"
			"\tgot mean = %1.6f # error = %0.2f%%\n",
			error(fabs(r.mean), 127.5) < expected.mean ? "ok" : "not ok",
			100.0 * expected.mean,
			r.mean, 100.0 * error(fabs(r.mean), 127.5));

		printf("%s 4 - Monte Carlo value for Pi\n"
			"\texpected pi = %1.6f +/- %0.2f%%\n"
			"\tgot pi = %1.6f # error = %1.6f%%\n",
			error(fabs(r.montepi), M_PI) < expected.epi ? "ok" : "not ok",
			M_PI, 100.0 * expected.epi,
			r.montepi, 100.0 * error(fabs(r.montepi), M_PI));
		/* XXX: use SEM for errors.
		 * <pestle> kate, the standard error there should be sqrt(p*(1-p)/n),
		 * where p = (area of circle)/(area of square) */

		printf("%s 5 - Chi square distribution\n"
			"\tgexpected chisq < %.2f\n"
			"\tgot chisq = %.2f\n",
			r.chisq < expected.chisq ? "ok" : "not ok",
			expected.chisq,
			r.chisq);

		printf("%s 6 - Chi square probability\n"
			"\tgexpected chip < %.2f%%\n"
			"\tgot chip = %.2f%%\n",
			r.chip < expected.chip ? "ok" : "not ok",
			100.0 * expected.chip,
			100.0 * r.chip);
	}

	return 0;
}

