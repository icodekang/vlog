#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "vlog.h"

int main(int argc, char** argv)
{
	int rc;
	int k;
	int i;

	if (argc != 2) {
		printf("test_leak ntime\n");
		return -1;
	}

	rc = vlog_init("test_leak.conf");

	k = atoi(argv[1]);
	while (k-- > 0) {
		i = rand();
		switch (i % 4) {
		case 0:
			rc = dvlog_init("test_leak.conf", "xxx");
			dvlog_info("init, rc=[%d]", rc);
			break;
		case 1:
			rc = vlog_reload(NULL);
			dvlog_info("reload null, rc=[%d]", rc);
			break;
		case 2:
			rc = vlog_reload("test_leak.2.conf");
			dvlog_info("reload 2, rc=[%d]", rc);
			break;
		case 3:
			vlog_fini();
			printf("fini\n");
	//		printf("vlog_finish\tj=[%d], rc=[%d]\n", j, rc);
			break;
		}
	}

	vlog_fini();
	return 0;
}
