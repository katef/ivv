#ifndef _TAP_ASSERT_H
#define _TAP_ASSERT_H

#if defined(assert)
#undef assert
#endif

#if (__STDC_VERSION - 0) >= 199901L
#define assert(e) tap_assert((e), __FILE__, __LINE__, __func__, #e)
#else
#define assert(e) tap_assert((e), __FILE__, __LINE__, 0, #e)
#endif

#endif

