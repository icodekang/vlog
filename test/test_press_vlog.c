#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#include "vlog.h"

static vlog_category_t *vg;
static long loop_count;

void * work(void *ptr)
{
	long j = loop_count;
	while(j-- > 0) {
		vlog_info(vg, "loglog");
	}
	return 0;
}


int test(long process_count, long thread_count)
{
	long i;
	pid_t pid;
	long j;

	for (i = 0; i < process_count; i++) {
		pid = fork();
		if (pid < 0) {
			printf("fork fail\n");
		} else if(pid == 0) {
			pthread_t  tid[thread_count];
			for (j = 0; j < thread_count; j++) { 
				pthread_create(&(tid[j]), NULL, work, NULL);
			}
			for (j = 0; j < thread_count; j++) { 
				pthread_join(tid[j], NULL);
			}
			return 0;
		}
	}

	for (i = 0; i < process_count; i++) {
		pid = wait(NULL);
	}

	return 0;
}


int main(int argc, char** argv)
{
	int rc;

	if (argc != 4) {
		fprintf(stderr, "test nprocess nthreads nloop\n");
		exit(1);
	}

	rc = vlog_init("test_press_vlog.conf");
	if (rc) {
		printf("init failed\n");
		return 2;
	}

	vg = vlog_get_category("my_cat");
	if (!vg) {
		printf("get cat failed\n");
		vlog_fini();
		return 3;
	}

	loop_count = atol(argv[3]);
	test(atol(argv[1]), atol(argv[2]));

	vlog_fini();
	
	return 0;
}
