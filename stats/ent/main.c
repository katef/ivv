/*
 * Entropy calculation and analysis of putative random sequences.
 *
 * Adapted from ent(1), designed and implemented
 * by John "Random" Walker in May 1985.
 * See http://www.fourmilab.ch/random/
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <unistd.h>

#include "randtest.h"

#define FALSE 0
#define TRUE  1

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void
usage(void)
{
	printf("usage: ent [-bu]\n");
	printf("\n");
	printf("  -b   Treat input as a stream of bits\n");
	printf("  -u   Print this message\n");
	printf("\n");
	printf("http://www.fourmilab.ch/\n");
}

int
main(int argc, char *argv[])
{
	struct rt_stats r;

	int binary = FALSE; /* Treat input as a bitstream */

	{
		int opt;

		while (opt = getopt(argc, argv, "bu?"), opt != -1) {
			switch (opt) {
			case 'b': binary = TRUE; break;

			case '?':
			case 'u':
				usage();
				return 0;
			}
		}

		argc -= optind;
		argv += optind;

		if (argc != 0) {
			usage();
			return 2;
		}
	}

	rt_init(binary);

	{
		FILE *f = stdin;
		int c;

		while (c = fgetc(f), c != EOF) {
			unsigned char u = (unsigned char) c;
			rt_add(&u, 1);
		}

		fclose(f);
	}

	rt_end(&r);

	{
		const char *samp = binary ? "bit" : "byte";

		printf("Entropy = %f bits per %s.\n", r.ent, samp);
		printf("Chi square distribution is %1.2f,\n", r.chisq);
		printf("and randomly would exceed this value ");
		if (r.chip < 0.0001) {
			printf("less than 0.01 percent of the times.\n\n");
		} else if (r.chip > 0.9999) {
			printf("more than than 99.99 percent of the times.\n\n");
		} else {
			printf("%1.2f percent of the times.\n\n", r.chip * 100);
		}

		printf(
			"Arithmetic mean value of data %ss is %1.4f (%.1f = random).\n",
			samp, r.mean, binary ? 0.5 : 127.5);
			printf("Monte Carlo value for Pi is %1.9f (error %1.2f percent).\n",
			r.montepi, 100.0 * (fabs(M_PI - r.montepi) / M_PI));

		printf("Serial correlation coefficient is ");
		if (r.scc >= -99999) {
			printf("%1.6f (totally uncorrelated = 0.0).\n", r.scc);
		} else {
			printf("undefined (all values equal!).\n");
		}
	}

	return 0;
}

