#include <stdio.h>
#include <unistd.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vlg;

	rc = vlog_init("test_tmp.conf");
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

	vlog_debug(vlg, "%s%d", "hello, vlog ", 1);
	vlog_info(vlg, "hello, vlog 2");

	sleep(1);

	vlog_info(vlg, "hello, vlog 3");
	vlog_debug(vlg, "hello, vlog 4");

//	vlog_profile();

	vlog_fini();
	
	return 0;
}
