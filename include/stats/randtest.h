/*
 * Copyright 2012-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef STATS_RANDTEST_H
#define STATS_RANDTEST_H

struct rt_stats {
	double ent;
	double chisq;
	double chip;
	double mean;
	double montepi;
	double scc;
};

void rt_init(int binmode);
void rt_add(void *buf, int bufl);
void rt_end(struct rt_stats *r);

#endif

