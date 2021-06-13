#include <stdio.h>
#include <unistd.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vg,*pvg;

	rc = vlog_init("test_prompt.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	vg = vlog_get_category("my_cat");
	pvg = vlog_get_category("prompt");
	if (!vg || !pvg) {
		printf("get cat fail\n");
		vlog_fini();
		return -2;
	}

	vlog_debug(vg, "%s%d", "hello, vlog ", 1);
	vlog_info(vg, "hello, vlog 2");

	for (int i =0; i<15;i++){
		vlog_info(pvg, "prompt>");
		sleep(1);
		if (! (i%3))
			vlog_debug(vg, "dummy log entry %d",i);
		if (! (i%5))
			vlog_info(vg, "hello, vlog %d",i);
	}
	vlog_info(vg, "done");

//	vlog_profile();

	vlog_fini();

	return 0;
}
