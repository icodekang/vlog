#include <stdio.h>

#include "vlog.h"

int output(vlog_msg_t *msg)
{
	printf("[mystd]:[%s][%s][%ld]\n", msg->path, msg->buf, (long)msg->len);
	return 0;
}

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vg;

	rc = vlog_init("test_record.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	vlog_set_record("myoutput", output);

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
