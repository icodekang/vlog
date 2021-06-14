#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "vlog.h"

#define CONFIG            "test_multithread.conf"
#define NB_THREADS        200
#define THREAD_LOOP_DELAY 10000	/* 0.01" */
#define RELOAD_DELAY      10

enum {
	vlog_LEVEL_TRACE = 10,
	vlog_LEVEL_SECURITY = 150,
	/* must equals conf file setting */
};

#define vlog_trace(cat, format, args...) \
	vlog(cat, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	vlog_LEVEL_TRACE, format, ##args)

#define vlog_security(cat, format, args...) \
	vlog(cat, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	vlog_LEVEL_SECURITY, format, ##args)


struct thread_info {    /* Used as argument to thread_start() */
	pthread_t thread_id;    /* ID returned by pthread_create() */
	int       thread_num;   /* Application-defined thread # */
	vlog_category_t *vlg;    /* The logger category struc address; (All threads will use the same category, so he same address) */
	long long int loop;     /* Counter incremented to check the thread's health */
};

struct thread_info *tinfo;


void intercept(int sig)
{
	int i;

    printf("\nIntercept signal %d\n\n", sig);

    signal (sig, SIG_DFL);
    raise (sig);

	printf("You can import datas below in a spreadsheat and check if any thread stopped increment the Loop counter during the test.\n\n");
	printf("Thread;Loop\n");
	for (i=0; i<NB_THREADS; i++)
	{
		printf("%d;%lld\n", tinfo[i].thread_num, tinfo[i].loop);
    }

    free(tinfo);

    vlog_fini();
}

void *myThread(void *arg)
{
    struct thread_info *tinfo = arg;

    tinfo->vlg = vlog_get_category("thrd");
	if (!tinfo->vlg) {
		printf("get thrd %d cat fail\n", tinfo->thread_num);
	}
	else
	{
		while(1)
		{
			usleep(THREAD_LOOP_DELAY);
			vlog_info(tinfo->vlg, "%d;%lld", tinfo->thread_num, tinfo->loop++);
		}
	}

	return NULL;
}

int main(int argc, char** argv)
{
	int rc;
	vlog_category_t *vlg;
	vlog_category_t *mc;
	vlog_category_t *hl;
	int i = 0;
	struct stat stat_0, stat_1;

	/* Create the logging directory if not yet ceated */
	mkdir("./test_multithread-logs", 0777);

	if (stat(CONFIG, &stat_0))
	{
		printf("Configuration file not found\n");
		return -1;
	}

	rc = vlog_init(CONFIG);
	if (rc) {
		printf("main init failed\n");
		return -2;
	}

	vlg = vlog_get_category("main");
	if (!vlg) {
		printf("main get cat fail\n");
		vlog_fini();
		return -3;
	}

	mc = vlog_get_category("clsn");
	if (!mc) {
		printf("clsn get cat fail\n");
		vlog_fini();
		return -3;
	}

	hl = vlog_get_category("high");
	if (!hl) {
		printf("high get cat fail\n");
		vlog_fini();
		return -3;
	}

	/* Interrupt (ANSI).		<Ctrl-C> */
	if (signal(SIGINT, intercept) == SIG_IGN )
	{
		vlog_fatal(vlg, "Can't caught the signal SIGINT, Interrupt (ANSI)");
		signal(SIGINT, SIG_IGN );
		return -4;
	}

	// start threads
    tinfo = calloc(NB_THREADS, sizeof(struct thread_info));
	for (i=0; i<NB_THREADS; i++)
	{
        tinfo[i].thread_num = i + 1;
        tinfo[i].loop = 0;
		tinfo[i].vlg = vlg;
		if(pthread_create(&tinfo[i].thread_id, NULL, myThread, &tinfo[i]) != 0)
		{
			vlog_fatal(vlg, "Unable to start thread %d", i);
			vlog_fini();
			return(-5);
		}
    }

	/* Wait and log thread informations */
	sleep(1);
	for (i=0; i<NB_THREADS; i++)
	{
		vlog_info(vlg, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, tinfo[i].vlg);
		vlog_fatal(mc, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, mc);
		vlog_error(mc, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, mc);
		vlog_warn(mc, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, mc);
		vlog_notice(mc, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, mc);
		vlog_info(mc, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, mc);
		vlog_trace(mc, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, mc);
		vlog_debug(mc, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, mc);
		vlog_security(hl, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, hl);
		vlog_warn(hl, "Thread [%d], vlog_category:@%p", tinfo[i].thread_num, hl);
    }

	/* Log main loop status */
	i=0;
	while(1)
	{
		int reload;

		sleep(1);
		i++;
		vlog_info(vlg, "Running time: %02d:%02d:%02d", i/3600, (i/60)%60, i%60);

		/* Check configuration file update */
		stat(CONFIG, &stat_1);

		/* Is configuration file modified */
		reload = (stat_0.st_mtime != stat_1.st_mtime);

		/* Or do we want to reload periodicaly the configuration file */
		if ( ! reload)
			if ( RELOAD_DELAY > 0)
				reload = (i % RELOAD_DELAY == 0);

		if (reload)
		{
			vlog_info(vlg, "Will reload configuration...");
			rc = vlog_reload(CONFIG);
			if (rc) {
				printf("main init failed\n");
				return -6;
			}
			vlog_info(vlg, "Configuration reloaded :)");
			stat(CONFIG, &stat_0);
		}
	}

    exit(EXIT_SUCCESS);
}
