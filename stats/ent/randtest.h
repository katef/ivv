#ifndef RANDTEST_H
#define RANDTEST_H

struct rt_stats {
	double ent;
	double chisq;
	double chip;
	double mean;
	double montepi;
	double scc;
};

extern void rt_init(int binmode);
extern void rt_add(void *buf, int bufl);
extern void rt_end(struct rt_stats *r);

#endif

