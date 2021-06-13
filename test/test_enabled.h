#ifndef __test_level_h
#define __test_level_h

#include "vlog.h"

enum {
	vlog_LEVEL_TRACE = 30,
	/* must equals conf file setting */
};

#define vlog_trace(cat, format, args...) \
	vlog(cat, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	vlog_LEVEL_TRACE, format, ##args)

#define vlog_trace_enabled(cat) vlog_level_enabled(cat, vlog_LEVEL_TRACE)

#endif
