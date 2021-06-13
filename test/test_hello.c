#include <stdio.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vg;

	rc = vlog_init("test_hello.conf");
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

	vlog_info(vg, "hello, vlog");

	vlog_fini();
	
	return 0;
}
