#include <stdio.h>

#include "test_enabled.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vlg;

	rc = vlog_init("test_enabled.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	vlg = vlog_get_category("my_cat");
	if (!vlg) {
		printf("get cat fail\n");
		vlog_fini();
		return -2;
	}

	if (vlog_trace_enabled(vlg)) {
		/* do something heavy to collect data */
		vlog_trace(vlg, "hello, vlog - trace");
	}

	if (vlog_debug_enabled(vlg)) {
		/* do something heavy to collect data */
		vlog_debug(vlg, "hello, vlog - debug");
	}

	if (vlog_info_enabled(vlg)) {
		/* do something heavy to collect data */
		vlog_info(vlg, "hello, vlog - info");
	}

	vlog_fini();

	return 0;
}
