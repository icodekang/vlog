#ifndef __vlg_profile_h
#define __vlg_profile_h

#include <stdarg.h>

#define EMPTY()
#define vlg_assert(expr, rv) \
	if(!(expr)) { \
		vlg_error(#expr" is null or 0"); \
		return rv; \
	} 

enum vlg_profile_flag {
	vlg_DEBUG = 0,
	vlg_WARN = 1,
	vlg_ERROR = 2
};


#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define vlg_debug(...) \
		vlg_profile_inner(vlg_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
	#define vlg_warn(...) \
		vlg_profile_inner(vlg_WARN, __FILE__, __LINE__, __VA_ARGS__)
	#define vlg_error(...) \
		vlg_profile_inner(vlg_ERROR, __FILE__, __LINE__, __VA_ARGS__)
	#define vlg_profile(flag, ...) \
		vlg_profile_inner(flag, __FILE__, __LINE__, __VA_ARGS__)
#elif defined __GNUC__
	#define vlg_debug(fmt, args...) \
		vlg_profile_inner(vlg_DEBUG, __FILE__, __LINE__, fmt, ## args)
	#define vlg_warn(fmt, args...) \
		vlg_profile_inner(vlg_WARN, __FILE__, __LINE__, fmt, ## args)
	#define vlg_error(fmt, args...) \
		vlg_profile_inner(vlg_ERROR, __FILE__, __LINE__, fmt, ## args)
	#define vlg_profile(flag, fmt, args...) \
		vlg_profile_inner(flag, __FILE__, __LINE__, fmt, ## args)
#endif


int vlg_profile_inner(int flag, 
		const char *file, const long line,
		const char *fmt, ...);

#endif
