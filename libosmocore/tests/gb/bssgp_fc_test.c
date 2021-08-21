/* test routines for BSSGP flow control implementation in libosmogb
 * (C) 2012 by Harald Welte <laforge@gnumonks.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include <osmocom/core/application.h>
#include <osmocom/core/utils.h>
#include <osmocom/core/logging.h>
#include <osmocom/core/talloc.h>
#include <osmocom/gprs/gprs_bssgp.h>

static unsigned long in_ctr = 1;
static struct timeval tv_start;
void *ctx = NULL;

int get_centisec_diff(void)
{
	struct timeval tv;
	struct timeval now;
	osmo_gettimeofday(&now, NULL);

	timersub(&now, &tv_start, &tv);

	return tv.tv_sec * 100 + tv.tv_usec/10000;
}

static int fc_out_cb(struct bssgp_flow_control *fc, struct msgb *msg,
		     uint32_t llc_pdu_len, void *priv)
{
	unsigned int csecs = get_centisec_diff();

	printf("%u: FC OUT Nr %lu\n", csecs, (unsigned long) msg->cb[0]);
	msgb_free(msg);
	return 0;
}

static void fc_in(struct bssgp_flow_control *fc, unsigned int pdu_len)
{
	struct msgb *msg;
	unsigned int csecs = get_centisec_diff();
	int rc;

	msg = msgb_alloc(1, "fc test");
	msg->cb[0] = in_ctr++;

	printf("%u: FC IN Nr %lu\n", csecs, msg->cb[0]);
	rc = bssgp_fc_in(fc, msg, pdu_len, NULL);
	switch (rc) {
	case 0:
		printf(" -> %d: ok\n", rc);
		break;
	case -ENOSPC:
		printf(" -> %d: queue full, msg dropped.\n", rc);
		break;
	case -EIO:
		printf(" -> %d: PDU too large, msg dropped.\n", rc);
		break;
	default:
		printf(" -> %d: error, msg dropped.\n", rc);
		break;
	}
}


static void test_fc(uint32_t bucket_size_max, uint32_t bucket_leak_rate,
		    uint32_t max_queue_depth, uint32_t pdu_len,
		    uint32_t pdu_count)
{
	struct bssgp_flow_control *fc = talloc_zero(ctx, struct bssgp_flow_control);
	int i;

	osmo_gettimeofday_override_time = (struct timeval){
		.tv_sec = 1486385000,
		.tv_usec = 423423,
	};
	osmo_gettimeofday_override = true;

	bssgp_fc_init(fc, bucket_size_max, bucket_leak_rate, max_queue_depth,
		      fc_out_cb);

	osmo_gettimeofday(&tv_start, NULL);

	/* Fill the queue with PDUs, possibly beyond the queue being full. If it is full, additional PDUs
	 * are discarded. */
	for (i = 0; i < pdu_count; i++) {
		fc_in(fc, pdu_len);
		osmo_timers_check();
		osmo_timers_prepare();
		osmo_timers_update();
	}

	while (1) {
		osmo_gettimeofday_override_add(0, 100000);

		osmo_timers_check();
		osmo_timers_prepare();
		osmo_timers_update();

		if (llist_empty(&fc->queue))
			break;
	}

	talloc_free(fc);
}

static void help(void)
{
	printf(" -h --help                This help message\n");
	printf(" -s --bucket-size-max N   Maximum size of bucket in octets\n");
	printf(" -r --bucket-leak-rate N  Bucket leak rate in octets/sec\n");
	printf(" -d --max-queue-depth N   Maximum length of pending PDU queue (msgs)\n");
	printf(" -l --pdu-length N        Length of each PDU in octets\n");
}

int bssgp_prim_cb(struct osmo_prim_hdr *oph, void *ctx)
{
	return -1;
}

static struct log_info info = {};

int main(int argc, char **argv)
{
	uint32_t bucket_size_max = 100;	/* octets */
	uint32_t bucket_leak_rate = 100; /* octets / second */
	uint32_t max_queue_depth = 5; /* messages */
	uint32_t pdu_length = 10; /* octets */
	uint32_t pdu_count = 20; /* messages */
	int c;
	void *tall_msgb_ctx;
	ctx = talloc_named_const(NULL, 0, "bssgp_fc_test");

	static const struct option long_options[] = {
		{ "bucket-size-max", 1, 0, 's' },
		{ "bucket-leak-rate", 1, 0, 'r' },
		{ "max-queue-depth", 1, 0, 'd' },
		{ "pdu-length", 1, 0, 'l' },
		{ "pdu-count", 1, 0, 'c' },
		{ "help", 0, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	osmo_init_logging2(ctx, &info);
	log_set_use_color(osmo_stderr_target, 0);
	log_set_print_filename2(osmo_stderr_target, LOG_FILENAME_NONE);
	log_set_print_category(osmo_stderr_target, 0);
	log_set_print_category_hex(osmo_stderr_target, 0);

	tall_msgb_ctx = msgb_talloc_ctx_init(ctx, 0);

	while ((c = getopt_long(argc, argv, "s:r:d:l:c:",
				long_options, NULL)) != -1) {
		switch (c) {
		case 's':
			bucket_size_max = atoi(optarg);
			break;
		case 'r':
			bucket_leak_rate = atoi(optarg);
			break;
		case 'd':
			max_queue_depth = atoi(optarg);
			break;
		case 'l':
			pdu_length = atoi(optarg);
			break;
		case 'c':
			pdu_count = atoi(optarg);
			break;
		case 'h':
			help();
			exit(EXIT_SUCCESS);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

	/* bucket leak rate less than 100 not supported! */
	if (bucket_leak_rate < 100) {
		fprintf(stderr, "Bucket leak rate < 100 not supported!\n");
		exit(EXIT_FAILURE);
	}

	printf("===== BSSGP flow-control test START\n");
	printf("size-max=%u oct, leak-rate=%u oct/s, "
		"queue-len=%u msgs, pdu_len=%u oct, pdu_cnt=%u\n\n", bucket_size_max,
		bucket_leak_rate, max_queue_depth, pdu_length, pdu_count);
	test_fc(bucket_size_max, bucket_leak_rate, max_queue_depth,
		pdu_length, pdu_count);
	printf("msgb ctx: %zu b in %zu blocks (0 b in 1 block == just the context)\n",
	       talloc_total_size(tall_msgb_ctx),
	       talloc_total_blocks(tall_msgb_ctx));
	OSMO_ASSERT(talloc_total_size(tall_msgb_ctx) == 0);
	OSMO_ASSERT(talloc_total_blocks(tall_msgb_ctx) == 1);
	talloc_free(tall_msgb_ctx);
	printf("===== BSSGP flow-control test END\n\n");

	exit(EXIT_SUCCESS);
}
