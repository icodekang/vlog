#include <stdio.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vlg;

	rc = vlog_init("test_category.conf");
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

	vlog_debug(vlg, "hello, vlog - debug");

	vlg = vlog_get_category("my-cat");
	if (!vlg) {
		printf("get cat fail\n");
		vlog_fini();
		return -2;
	}

	vlog_info(vlg, "hello, vlog - info");

	vlog_fini();
	
	return 0;
}
