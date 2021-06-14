#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "vlog.h"
#include "test_conf2.conf.h"

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vlg;

	rc = vlog_init(test_conf2_conf);
	if (rc) {
		printf("init failed, save [] in a config file and try vlog-chk-conf for more detail [%s]\n", test_conf2_conf);
		return -1;
	}

	vlg = vlog_get_category("my_cat");
	if (!vlg) {
		printf("get cat fail\n");
		vlog_fini();
		return -2;
	}

	vlog_info(vlg, "hello, vlog");

	vlog_fini();
	printf("log end\n");
	
	return 0;
}
