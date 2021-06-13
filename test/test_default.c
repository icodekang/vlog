#include <stdio.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;

	rc = dvlog_init("test_default.conf", "my_cat");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	dvlog_info("hello, vlog");

	vlog_fini();
	
	return 0;
}
