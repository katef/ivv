/*
 * Copyright 2012-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

static void
h(va_list *p)
{
	int x;

	assert(p != NULL);

	x = va_arg(*p, int);

	assert(x == 5);
}

static void
g(int dummy, va_list ap)
{
	(void) dummy;

#if WORKAROUND

	/*
	 * This is a work-around for a non-conformant implementation for amd64
	 * where va_list is an array, which prevents passing &ap to h().
	 * Since ap evaulates to a pointer as a function argument, &ap is therefore
	 * a pointer to a pointer, whereas &hack is a pointer to an array.
	 */
	{
		va_list hack;

		va_copy(hack, ap);

		h(&hack);
	}

#else

	/* should be legal */
	h(&ap);

#endif

}

static void
f(int dummy, ...)
{
	va_list ap;

	va_start(ap, dummy);
	g(dummy, ap);
	va_end(ap);
}

int main(void) {
	f(0, 5);

	return 0;
}

