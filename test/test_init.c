#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	
	vlog_category_t *vlg;

	rc = vlog_init("test_init.conf");
	if (rc) {
		printf("init fail");
		return -2;
	}
	vlg = vlog_get_category("my_cat");
	if (!vlg) {
		printf("vlog_get_category fail\n");
		vlog_fini();
		return -1;
	}
	vlog_info(vlg, "before update");
	sleep(1);
	rc = vlog_reload("test_init.2.conf");
	if (rc) {
		printf("update fail\n");
	}
	vlog_info(vlg, "after update");
	vlog_profile();
	vlog_fini();

	sleep(1);
	vlog_init("test_init.conf");
	vlg = vlog_get_category("my_cat");
	if (!vlg) {
		printf("vlog_get_category fail\n");
		vlog_fini();
		return -1;
	}
	vlog_info(vlg, "init again");
	vlog_fini();
	
	return 0;
}
