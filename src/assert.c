#include <stdio.h>

#include <tap/assert.h>
#include <tap/plan.h>

void
tap_assert(int e, const char *file, long int line, const char *func, const char *s)
{
	tap_count++;

	if (func != NULL) {
		(void) printf("%s %u - assert %s # %s:%ld:%s\n",
			e ? "ok" : "not ok", tap_count,
			s, file, line, func);
	} else {
		(void) printf("%s %u - assert %s # %s:%ld\n",
			e ? "ok" : "not ok", tap_count,
			s, file, line);
	}
}

