#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vlog.h"

int main(int argc, char** argv)
{
	unsigned char aa[32];
	int i, j;

	if (argc != 3) {
		printf("useage: test_bitmap i j\n");
		exit(1);
	}

	dvlog_init(NULL, "AA");


	i = atoi(argv[1]);
	j = atoi(argv[2]);

	memset(aa, 0x00, sizeof(aa));

	/* 32 byte, 256 bit
	 * [11111..1100...00]
	 *          i
	 */
	aa[i/8] |=  ~(0xFF << (8 - i % 8));
	memset(aa + i/8 + 1, 0xFF, sizeof(aa) - i/8 - 1);

	hdvlog_info(aa, sizeof(aa));

	dvlog_info("%0x", aa[j/8]);
	dvlog_info("%0x", aa[j/8] >> 6);

	/* see j of bits fits */
	dvlog_info("%0x", ~((aa[j/8] >> (7 - j % 8)) & 0x01) );

	vlog_fini();
	
	return 0;
}
