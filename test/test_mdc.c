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

	rc = vlog_init("test_mdc.conf");
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


	vlog_info(vg, "1.hello, vlog");

	vlog_put_mdc("myname", "Zhang");

	vlog_info(vg, "2.hello, vlog");

	vlog_put_mdc("myname", "Li");

	vlog_info(vg, "3.hello, vlog");

	vlog_fini();
	
	return 0;
}
