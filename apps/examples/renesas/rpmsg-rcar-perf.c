/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sched.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"

#define RPMSG_SERV_NAME  "rpmsg-perf-sample"
#define HELLO_MSG        "Hello world!"
#define BYE_MSG          "Goodbye!"
#define MSG_LIMIT_DEFAULT 100000
#define LATENCY_RANGE     80000

#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

static struct rpmsg_endpoint lept;
static int rnum;
static int err_cnt;
static int ept_deleted;
static volatile int response_received;

static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)ept;
	(void)data;
	(void)len;
	(void)src;
	(void)priv;

	rnum++;
	response_received = 1;

	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;

	rpmsg_destroy_ept(&lept);
	LPRINTF("echo test: service is destroyed\r\n");
	ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
				       const char *name, uint32_t dest)
{
	LPRINTF("new endpoint notification is received.\r\n");

	if (strcmp(name, RPMSG_SERV_NAME))
		LPERROR("Unexpected name service %s.\r\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_SERV_NAME,
				       RPMSG_ADDR_ANY, dest,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);
}

int app(struct rpmsg_device *rdev, void *priv, int msg_limit)
{
	int ret;
	int i;
	const char *msg      = HELLO_MSG;
	int         msg_len  = strlen(HELLO_MSG);
	struct timespec ts_current;
	struct timespec ts_end;
	struct timespec ts_start_test;
	struct timespec ts_end_test;
	int    latency             = 0;
	static int latencies[LATENCY_RANGE];
	int    latency_worst_case  = 0;
	int    latency_best_case   = INT_MAX;
	long   latency_valid_total = 0;
	int    spike_count         = 0;
	int    valid_count         = 0;
	double latency_average     = 0;

	memset(latencies, 0, sizeof(latencies));

	LPRINTF("=================================\r\n");
	LPRINTF(" RPMsg Performance Test\r\n");
	LPRINTF(" Number of messages : %d\r\n", msg_limit);
	LPRINTF("=================================\r\n");

	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERV_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);
	if (ret) {
		LPERROR("Failed to create RPMsg endpoint.\r\n");
		return ret;
	}

	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);

	LPRINTF("RPMSG endpoint is binded with remote.\r\n");

	/* Send the 1st message */
	response_received = 0;
	rpmsg_send(&lept, msg, msg_len);
	while (!response_received)
		platform_poll(priv);

	clock_gettime(CLOCK_MONOTONIC, &ts_start_test);

	for (i = 1; i <= msg_limit; i++) {
		response_received = 0;

		clock_gettime(CLOCK_MONOTONIC, &ts_current);

		if (i < msg_limit)
			ret = rpmsg_send(&lept, msg, msg_len);
		else
			ret = rpmsg_send(&lept, BYE_MSG, strlen(BYE_MSG));

		if (ret < 0) {
			LPERROR("Failed to send data...\r\n");
			break;
		}

		while (!response_received && !err_cnt) {
			platform_poll(priv);
			if (!response_received)
				sched_yield();
		}

		clock_gettime(CLOCK_MONOTONIC, &ts_end);

		long sec_diff  = ts_end.tv_sec  - ts_current.tv_sec;
		long nsec_diff = ts_end.tv_nsec - ts_current.tv_nsec;

		if (nsec_diff < 0) {
			sec_diff--;
			nsec_diff += 1000000000L;
		}

		latency = (int)(sec_diff * 1000000L + nsec_diff / 1000L);

		if (latency < latency_best_case)
			latency_best_case = latency;
		if (latency > latency_worst_case)
			latency_worst_case = latency;

		if (latency > LATENCY_RANGE) {
			spike_count++;
			printf("[SPIKE] i=%d latency=%d us\n", i, latency);
			continue;
		}

		latency_valid_total += latency;
		valid_count++;
		latencies[latency]++;
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_end_test);

	if (valid_count > 0)
		latency_average = (double)latency_valid_total / valid_count;

	printf("\n========== Results ==========\n");
	printf("Total samples  : %d\n",      msg_limit);
	printf("Valid  samples : %d\n",      valid_count);
	printf("Spike  samples : %d\n",      spike_count);
	printf("Best-case  RTT : %d us\n",   latency_best_case);
	printf("Average    RTT : %.2f us\n", latency_average);
	printf("Worst-case RTT : %d us\n",   latency_worst_case);
	printf("Total time     : %ld s\n", ts_end_test.tv_sec - ts_start_test.tv_sec);
	printf("=============================\n");

	LPRINTF("**********************************\r\n");
	LPRINTF(" Test Results: Error count = %d\r\n", err_cnt);
	LPRINTF("**********************************\r\n");

	while (!ept_deleted)
		platform_poll(priv);

	LPRINTF("Quitting application .. rpmsg sample test end\r\n");

	return 0;
}

int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;
	int opt;

	/* Default values */
	int msg_limit = MSG_LIMIT_DEFAULT;
	int proc_id   = 0;
	int rsc_id    = 0;

	/* Parse args */
	while ((opt = getopt(argc, argv, "c:p:r:")) != -1) {
		switch (opt) {
		case 'c':
			msg_limit = atoi(optarg);
			if (msg_limit <= 0) {
				fprintf(stderr, "Invalid number of messages, using default: %d\n",
					MSG_LIMIT_DEFAULT);
				msg_limit = MSG_LIMIT_DEFAULT;
			}
			break;
		case 'p':
			proc_id = atoi(optarg);
			if (proc_id < 0) {
				fprintf(stderr, "Invalid remote proc id, using default: 0\n");
				proc_id = 0;
			}
			break;
		case 'r':
			rsc_id = atoi(optarg);
			if (rsc_id < 0) {
				fprintf(stderr, "Invalid resource table id, using default: 0\n");
				rsc_id = 0;
			}
			break;
		default:
			fprintf(stderr,
				"Usage: %s [-c msg_count] [-p proc_id] [-r rsc_id]\n"
				"  -c  number of messages (default: %d)\n"
				"  -p  remote processor id (default: 0)\n"
				"  -r  resource table id (default: 0)\n",
				argv[0], MSG_LIMIT_DEFAULT);
			return -1;
		}
	}

	/* Build argv for platform_init */
	char proc_id_str[16];
	char rsc_id_str[16];

	snprintf(proc_id_str, sizeof(proc_id_str), "%d", proc_id);
	snprintf(rsc_id_str,  sizeof(rsc_id_str),  "%d", rsc_id);

	char *platform_argv[3];

	platform_argv[0] = argv[0];
	platform_argv[1] = proc_id_str;
	platform_argv[2] = rsc_id_str;

	/* Initialize platform */
	ret = platform_init(3, platform_argv, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\r\n");
		return -1;
	}

	rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_DRIVER,
					   NULL, rpmsg_name_service_bind_cb);
	if (!rpdev) {
		LPERROR("Failed to create rpmsg virtio device.\r\n");
		platform_cleanup(platform);
		return -1;
	}

	app(rpdev, platform, msg_limit);

	platform_release_rpmsg_vdev(rpdev, platform);

	LPRINTF("Stopping application...\r\n");
	platform_cleanup(platform);

	return 0;
}
