/*
 * Entropy calculation and analysis of putative random sequences.
 *
 * Designed and implemented by John "Random" Walker in May 1985.
 *
 * Multiple analyses of random sequences added in December 1985.
 *
 * Bit stream analysis added in September 1997.
 *
 * getopt() command line processing, optional stdin input,
 * and HTML documentation added in October 1998.
 *
 * Replaced table look-up for chi square to probability
 * conversion with algorithmic computation in January 2008.
 *
 * For additional information and the latest version,
 * see http://www.fourmilab.ch/random/
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <unistd.h>

#include "randtest.h"

#define UPDATE "January 28th, 2008"

#define FALSE 0
#define TRUE  1

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Print information on how to call */
static void
help(void)
{
	printf("ent --  Calculate entropy of file.  Call");
	printf("\n        with ent [options] [input-file]");
	printf("\n");
	printf("\n        Options:   -b   Treat input as a stream of bits");
	printf("\n                   -f   Fold upper to lower case letters");
	printf("\n                   -t   Terse output in CSV format");
	printf("\n                   -u   Print this message\n");
	printf("\nBy John Walker");
	printf("\n   http://www.fourmilab.ch/");
	printf("\n   %s\n", UPDATE);
}

int
main(int argc, char *argv[])
{
	int oc, opt;
	long ccount[256];	      /* Bins to count occurrences of values */
	long totalc = 0;	      /* Total character count */
	char *samp;
	struct rt_stats r;

	FILE *fp = stdin;

	int fold   = FALSE, /* Fold upper to lower */
	    binary = FALSE; /* Treat input as a bitstream */

	while (opt = getopt(argc, argv, "bfu?BFU"), opt != -1) {
		switch (tolower(opt)) {
		case 'b': binary = TRUE; break;
		case 'f': fold   = TRUE; break;

		case '?':
		case 'u':
			help();
			return 0;
		}
	}

	if (optind < argc) {
		if (optind != (argc - 1)) {
			printf("Duplicate file name.\n");
			help();
			return 2;
		}
		if ((fp = fopen(argv[optind], "rb")) == NULL) {
			printf("Cannot open file %s\n", argv[optind]);
			return 2;
		}
	}

	samp = binary ? "bit" : "byte";
	memset(ccount, 0, sizeof ccount);

	/* Initialise for calculations */
	rt_init(binary);

	/* Scan input file and count character occurrences */
	while (oc = fgetc(fp), oc != EOF) {
		unsigned char ocb;

		if (fold && isalpha(oc) && isupper(oc)) {
			oc = tolower(oc);
		}

		ocb = (unsigned char) oc;
		totalc += binary ? 8 : 1;
		if (binary) {
			int b;
			unsigned char ob = ocb;

			for (b = 0; b < 8; b++) {
				ccount[ob & 1]++;
				ob >>= 1;
			}
		} else {
			ccount[ocb]++; /* Update counter for this bin */
		}
		rt_add(&ocb, 1);
	}
	fclose(fp);

	/* Complete calculation and return sequence metrics */
	rt_end(&r);

	/* Print calculated results */
	printf("Entropy = %f bits per %s.\n", r.ent, samp);
	printf("Chi square distribution for %ld samples is %1.2f, and randomly\n",
		totalc, r.chisq);
	if (r.chip < 0.0001) {
		printf("would exceed this value less than 0.01 percent of the times.\n\n");
	} else if (r.chip > 0.9999) {
		printf("would exceed this value more than than 99.99 percent of the times.\n\n");
	} else {
		printf("would exceed this value %1.2f percent of the times.\n\n",
		r.chip * 100);
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

	return 0;
}

