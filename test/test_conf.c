#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vg;
	

	rc = vlog_init("test_conf.conf");
	if (rc) {
		printf("init failed, try vlog-chk-conf test_conf.conf for more detail\n");
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
	printf("log end\n");
	
	return 0;
}
