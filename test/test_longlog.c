#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "vlog.h"

#define str "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
#define str2 str str
#define str4 str2 str2
#define str8 str4 str4
#define str16 str8 str8
#define str32 str16 str16
#define str64 str32 str32

int main(int argc, char** argv)
{
	int i, k;
	int rc;
	vlog_category_t *vg;

	if (argc != 2) {
		printf("useage: test_longlog [count]\n");
		exit(1);
	}

	rc = vlog_init("test_longlog.conf");
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

	k = atoi(argv[1]);
	while (k-- > 0) {
		i = rand();
		switch (i % 3) {
		case 0:
			vlog_info(vg, str32);
			break;
		case 1:
			vlog_info(vg, str64);
			break;
		case 2:
			vlog_info(vg, str16);
			break;
		}
	}


	vlog_fini();
	
	return 0;
}
