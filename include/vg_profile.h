#ifndef __vg_profile_h
#define __vg_profile_h

#include <stdarg.h>

#define EMPTY()
#define vg_assert(expr, rv) \
	if(!(expr)) { \
		vg_error(#expr" is null or 0"); \
		return rv; \
	} 

enum vg_profile_flag {
	vg_DEBUG = 0,
	vg_WARN = 1,
	vg_ERROR = 2
};


#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define vg_debug(...) \
		vg_profile_inner(vg_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
	#define vg_warn(...) \
		vg_profile_inner(vg_WARN, __FILE__, __LINE__, __VA_ARGS__)
	#define vg_error(...) \
		vg_profile_inner(vg_ERROR, __FILE__, __LINE__, __VA_ARGS__)
	#define vg_profile(flag, ...) \
		vg_profile_inner(flag, __FILE__, __LINE__, __VA_ARGS__)
#elif defined __GNUC__
	#define vg_debug(fmt, args...) \
		vg_profile_inner(vg_DEBUG, __FILE__, __LINE__, fmt, ## args)
	#define vg_warn(fmt, args...) \
		vg_profile_inner(vg_WARN, __FILE__, __LINE__, fmt, ## args)
	#define vg_error(fmt, args...) \
		vg_profile_inner(vg_ERROR, __FILE__, __LINE__, fmt, ## args)
	#define vg_profile(flag, fmt, args...) \
		vg_profile_inner(flag, __FILE__, __LINE__, fmt, ## args)
#endif


int vg_profile_inner(int flag, 
		const char *file, const long line,
		const char *fmt, ...);

#endif
