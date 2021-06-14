#ifndef __vlg_xplatform_h
#define __vlg_xplatform_h

#include <limits.h>

#define vlog_INT32_LEN   sizeof("-2147483648") - 1
#define vlog_INT64_LEN   sizeof("-9223372036854775808") - 1

#if ((__GNU__ == 2) && (__GNUC_MINOR__ < 8))
#define vlog_MAX_UINT32_VALUE  (uint32_t) 0xffffffffLL
#else
#define vlog_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#endif

#define vlog_MAX_INT32_VALUE   (uint32_t) 0x7fffffff

#define MAXLEN_PATH 1024
#define MAXLEN_CFG_LINE (MAXLEN_PATH * 4)
#define MAXLINES_NO 128

#define FILE_NEWLINE "\n"
#define FILE_NEWLINE_LEN 1

#include <string.h>
#include <strings.h>

#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#define STRICMP(_a_,_C_,_b_) ( strcasecmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strncasecmp(_a_,_b_,_n_) _C_ 0 )


#ifdef __APPLE__
#include <AvailabilityMacros.h>
#endif

/* Define vlog_fstat to fstat or fstat64() */
#if defined(__APPLE__) && !defined(MAC_OS_X_VERSION_10_6)
#define vlog_fstat fstat64
#define vlog_stat stat64
#else
#define vlog_fstat fstat
#define vlog_stat stat
#endif

/* Define vlog_fsync to fdatasync() in Linux and fsync() for all the rest */
#ifdef __linux__
#define vlog_fsync fdatasync
#else
#define vlog_fsync fsync
#endif



#endif
