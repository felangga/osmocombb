/* Main method of the layer2/3 stack */

/* (C) 2010 by Holger Hans Peter Freyther
 * (C) 2010 by Harald Welte <laforge@gnumonks.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <osmocom/bb/common/osmocom_data.h>
#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/mobile/app_mobile.h>

#include <osmocom/core/talloc.h>
#include <osmocom/core/linuxlist.h>
#include <osmocom/core/gsmtap_util.h>
#include <osmocom/core/gsmtap.h>
#include <osmocom/core/signal.h>
#include <osmocom/core/application.h>

#include <arpa/inet.h>

#define _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <libgen.h>

void *l23_ctx = NULL;
struct llist_head ms_list;
static char *gsmtap_ip = 0;
static const char *custom_cfg_file = NULL;
struct gsmtap_inst *gsmtap_inst = NULL;
char *config_dir = NULL;
int use_mncc_sock = 0;
int daemonize = 0;

int mncc_recv_socket(struct osmocom_ms *ms, int msg_type, void *arg);

int mobile_delete(struct osmocom_ms *ms, int force);
int mobile_signal_cb(unsigned int subsys, unsigned int signal,
		     void *handler_data, void *signal_data);
int mobile_work(struct osmocom_ms *ms);
int mobile_exit(struct osmocom_ms *ms, int force);


const char *debug_default =
	"DCS:DNB:DPLMN:DRR:DMM:DSIM:DCC:DMNCC:DSS:DLSMS:DPAG:DSUM:DSAP:DGPS:DMOB:DPRIM:DLUA";

const char *openbsc_copyright =
	"Copyright (C) 2010-2015 Andreas Eversberg, Sylvain Munaut, Holger Freyther, Harald Welte\n"
	"Contributions by Alex Badea, Pablo Neira, Steve Markgraf and others\n\n"
	"License GPLv2+: GNU GPL version 2 or later "
		"<http://gnu.org/licenses/gpl.html>\n"
	"This is free software: you are free to change and redistribute it.\n"
	"There is NO WARRANTY, to the extent permitted by law.\n";

static void print_usage(const char *app)
{
	printf("Usage: %s\n", app);
}

static void print_help()
{
	printf(" Some help...\n");
	printf("  -h --help		this text\n");
	printf("  -i --gsmtap-ip	The destination IP used for GSMTAP.\n");
	printf("  -d --debug		Change debug flags. default: %s\n",
		debug_default);
	printf("  -D --daemonize	Run as daemon\n");
	printf("  -c --config-file filename The config file to use.\n");
	printf("  -m --mncc-sock	Disable built-in MNCC handler and "
		"offer socket\n");
}

static int handle_options(int argc, char **argv)
{
	while (1) {
		int option_index = 0, c;
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"gsmtap-ip", 1, 0, 'i'},
			{"debug", 1, 0, 'd'},
			{"daemonize", 0, 0, 'D'},
			{"config-file", 1, 0, 'c'},
			{"mncc-sock", 0, 0, 'm'},
			/* DEPRECATED options, to be removed */
			{"vty-ip", 1, 0, 'u'},
			{"vty-port", 1, 0, 'v'},
			{0, 0, 0, 0},
		};

		c = getopt_long(argc, argv, "hi:u:c:v:d:Dm",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_usage(argv[0]);
			print_help();
			exit(0);
			break;
		case 'i':
			gsmtap_ip = optarg;
			break;
		case 'c':
			custom_cfg_file = optarg;
			break;
		case 'd':
			log_parse_category_mask(osmo_stderr_target, optarg);
			break;
		case 'D':
			daemonize = 1;
			break;
		case 'm':
			use_mncc_sock = 1;
			break;
		/* DEPRECATED options, to be removed */
		case 'u':
		case 'v':
			fprintf(stderr, "Both 'u' and 'v' options are "
				"deprecated! Please use the configuration file "
				"in order to set VTY bind address.\n");
			/* fall-thru */
		default:
			/* Unknown parameter passed */
			return -EINVAL;
		}
	}

	return 0;
}

void sighandler(int sigset)
{
	static uint8_t _quit = 1, count_int = 0;

	if (sigset == SIGHUP || sigset == SIGPIPE)
		return;

	fprintf(stderr, "\nSignal %d received.\n", sigset);

	switch (sigset) {
	case SIGINT:
		/* The first signal causes initiating of shutdown with detach
		 * procedure. The second signal causes initiating of shutdown
		 * without detach procedure. The third signal will exit process
		 * immediately. (in case it hangs)
		 */
		if (count_int == 0) {
			fprintf(stderr, "Performing shutdown with clean "
				"detach, if possible...\n");
			osmo_signal_dispatch(SS_GLOBAL, S_GLOBAL_SHUTDOWN,
				NULL);
			count_int = 1;
			break;
		}
		if (count_int == 2) {
			fprintf(stderr, "Unclean exit, please turn off phone "
				"to be sure it is not transmitting!\n");
			exit(0);
		}
		/* fall through */
	case SIGTSTP:
		count_int = 2;
		fprintf(stderr, "Performing shutdown without detach...\n");
		osmo_signal_dispatch(SS_GLOBAL, S_GLOBAL_SHUTDOWN, &_quit);
		break;
	case SIGABRT:
		/* in case of abort, we want to obtain a talloc report and
		 * then run default SIGABRT handler, who will generate coredump
		 * and abort the process. abort() should do this for us after we
		 * return, but program wouldn't exit if an external SIGABRT is
		 * received.
		 */
		talloc_report_full(l23_ctx, stderr);
		signal(SIGABRT, SIG_DFL);
		raise(SIGABRT);
		break;
	case SIGUSR1:
	case SIGUSR2:
		talloc_report_full(l23_ctx, stderr);
		break;
	}
}

int main(int argc, char **argv)
{
	char *config_file;
	int quit = 0;
	int rc;

	printf("%s\n", openbsc_copyright);

	srand(time(NULL));

	INIT_LLIST_HEAD(&ms_list);

	l23_ctx = talloc_named_const(NULL, 1, "layer2 context");
	/* TODO: measure and choose a proper pool size */
	msgb_talloc_ctx_init(l23_ctx, 0);

	/* Init default stderr logging */
	osmo_init_logging2(l23_ctx, &log_info);

	rc = handle_options(argc, argv);
	if (rc) { /* Abort in case of parsing errors */
		fprintf(stderr, "Error in command line options. Exiting.\n");
		return 1;
	}

	if (gsmtap_ip) {
		gsmtap_inst = gsmtap_source_init(gsmtap_ip, GSMTAP_UDP_PORT, 1);
		if (!gsmtap_inst) {
			fprintf(stderr, "Failed during gsmtap_init()\n");
			exit(1);
		}
		gsmtap_source_add_sink(gsmtap_inst);
	}

	if (custom_cfg_file) {
		/* Use full path provided by user */
		config_file = talloc_strdup(l23_ctx, custom_cfg_file);
	} else {
		/* Obtain the user's home directory path */
		const char *home_dir = getenv("HOME");
		if (!home_dir)
			home_dir = "~";

		/* Concatenate it with default config path */
		config_file = talloc_asprintf(l23_ctx, "%s/%s",
			home_dir, ".osmocom/bb/mobile.cfg");
	}

	/* save the config file directory name */
	config_dir = talloc_strdup(l23_ctx, config_file);
	config_dir = dirname(config_dir);

	if (use_mncc_sock)
		rc = l23_app_init(mncc_recv_socket, config_file);
	else
		rc = l23_app_init(NULL, config_file);
	if (rc)
		exit(rc);

	signal(SIGINT, sighandler);
	signal(SIGTSTP, sighandler);
	signal(SIGHUP, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGPIPE, sighandler);
	signal(SIGABRT, sighandler);
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);

	if (daemonize) {
		printf("Running as daemon\n");
		rc = osmo_daemonize();
		if (rc)
			fprintf(stderr, "Failed to run as daemon\n");
	}

	while (1) {
		l23_app_work(&quit);
		if (quit && llist_empty(&ms_list))
			break;
		osmo_select_main(0);
	}

	l23_app_exit();
	log_fini();

	talloc_free(config_file);
	talloc_free(config_dir);
	talloc_report_full(l23_ctx, stderr);

	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGUSR2, SIG_DFL);

	return 0;
}
