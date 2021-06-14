#include <stdio.h>
#include <unistd.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vlg,*pvlg;

	rc = vlog_init("test_prompt.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	vlg = vlog_get_category("my_cat");
	pvlg = vlog_get_category("prompt");
	if (!vlg || !pvlg) {
		printf("get cat fail\n");
		vlog_fini();
		return -2;
	}

	vlog_debug(vlg, "%s%d", "hello, vlog ", 1);
	vlog_info(vlg, "hello, vlog 2");

	for (int i =0; i<15;i++){
		vlog_info(pvlg, "prompt>");
		sleep(1);
		if (! (i%3))
			vlog_debug(vlg, "dummy log entry %d",i);
		if (! (i%5))
			vlog_info(vlg, "hello, vlog %d",i);
	}
	vlog_info(vlg, "done");

//	vlog_profile();

	vlog_fini();

	return 0;
}
