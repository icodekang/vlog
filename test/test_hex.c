#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "vlog.h"
#include "stdlib.h"

static int ReadTotalFile( FILE * fp , char ** ptr , long * len )
{
        long            fileLen ;
        int             nret ;
        char            * pStart ;

        *ptr = NULL;

        nret = fseek( fp , 0L , SEEK_END );
        if( nret )
        {
                return -1;
        }

        fileLen = ftell( fp );
        if( fileLen < 0 )
        {
                return -2;
        }

        if( ( pStart = calloc(1, fileLen+1) ) == NULL )
        {
                return -3;
        }

        nret = fseek( fp , 0L , SEEK_SET );
        if( nret )
        {
                free( pStart );
                return -4;
        }

        nret = fread( pStart , fileLen , 1 , fp );
        if( ferror( fp ) )
        {
                free( pStart );
                return -5;
        }

        *ptr = pStart;
        *len = fileLen;

        return 0;
}

int main(int argc, char** argv)
{
	int rc;

	FILE	*fp;
	char	*dmp;
	long	dmp_len = 0;
	int	ntimes;

	if (argc != 3) {
		printf("useage: test_hex [file] [ntimes]\n");
		exit(1);
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		printf("fopen[%s] fail\n", argv[1]);
		exit(1);
	}

	ntimes = atoi(argv[2]);
	
	vlog_category_t *vlg;

	rc = vlog_init("test_hex.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}
	
	vlg = vlog_get_category("my_cat");
	if (!vlg) {
		printf("get category failed\n");
		vlog_fini();
		return -2;
	}


	rc = ReadTotalFile(fp, &dmp, &dmp_len);

	while(ntimes--) hvlog_debug(vlg, dmp, dmp_len);

	fclose(fp);
	free(dmp);

	vlog_fini();
	printf("hex log end\n");
	
	return 0;
}
