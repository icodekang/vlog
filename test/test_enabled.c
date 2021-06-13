#include <stdio.h>

#include "test_enabled.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vg;

	rc = vlog_init("test_enabled.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	vg = vlog_get_category("my_cat");
	if (!vg) {
		printf("get cat fail\n");
		vlog_fini();
		return -2;
	}

	if (vlog_trace_enabled(vg)) {
		/* do something heavy to collect data */
		vlog_trace(vg, "hello, vlog - trace");
	}

	if (vlog_debug_enabled(vg)) {
		/* do something heavy to collect data */
		vlog_debug(vg, "hello, vlog - debug");
	}

	if (vlog_info_enabled(vg)) {
		/* do something heavy to collect data */
		vlog_info(vg, "hello, vlog - info");
	}

	vlog_fini();

	return 0;
}
