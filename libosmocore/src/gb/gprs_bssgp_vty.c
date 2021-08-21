/*! \file gprs_bssgp_vty.c
 * VTY interface for our GPRS BSS Gateway Protocol (BSSGP) implementation. */
/*
 * (C) 2010 by Harald Welte <laforge@gnumonks.org>
 *
 * All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <arpa/inet.h>

#include <osmocom/core/msgb.h>
#include <osmocom/gsm/tlv.h>
#include <osmocom/core/talloc.h>
#include <osmocom/core/select.h>
#include <osmocom/core/rate_ctr.h>
#include <osmocom/gprs/gprs_ns.h>
#include <osmocom/gprs/gprs_bssgp.h>
#include <osmocom/gprs/gprs_bssgp_bss.h>

#include <osmocom/vty/vty.h>
#include <osmocom/vty/command.h>
#include <osmocom/vty/logging.h>
#include <osmocom/vty/telnet_interface.h>
#include <osmocom/vty/misc.h>

static void log_set_bvc_filter(struct log_target *target,
				struct bssgp_bvc_ctx *bctx)
{
	if (bctx) {
		target->filter_map |= (1 << LOG_FLT_GB_BVC);
		target->filter_data[LOG_FLT_GB_BVC] = bctx;
	} else if (target->filter_data[LOG_FLT_GB_BVC]) {
		target->filter_map = ~(1 << LOG_FLT_GB_BVC);
		target->filter_data[LOG_FLT_GB_BVC] = NULL;
	}
}

static struct cmd_node bssgp_node = {
	L_BSSGP_NODE,
	"%s(config-bssgp)# ",
	1,
};

static int config_write_bssgp(struct vty *vty)
{
	vty_out(vty, "bssgp%s", VTY_NEWLINE);

	return CMD_SUCCESS;
}

DEFUN(cfg_bssgp, cfg_bssgp_cmd,
      "bssgp",
      "Configure the GPRS BSS Gateway Protocol")
{
	vty->node = L_BSSGP_NODE;
	return CMD_SUCCESS;
}

static void dump_bvc(struct vty *vty, struct bssgp_bvc_ctx *bvc, int stats)
{
	vty_out(vty, "NSEI %5u, BVCI %5u, RA-ID: %s, CID: %u, "
		"STATE: %s%s", bvc->nsei, bvc->bvci, osmo_rai_name(&bvc->ra_id),
		bvc->cell_id, bvc->state & BVC_S_BLOCKED ? "BLOCKED" : "UNBLOCKED", VTY_NEWLINE);

	if (stats) {
		struct bssgp_flow_control *fc = bvc->fc;

		vty_out_rate_ctr_group(vty, " ", bvc->ctrg);

		if (fc)
			vty_out(vty, "FC-BVC(bucket_max: %uoct, leak_rate: "
				"%uoct/s, cur_tokens: %uoct, max_q_d: %u, "
				"cur_q_d: %u)%s", fc->bucket_size_max,
				fc->bucket_leak_rate, fc->bucket_counter,
				fc->max_queue_depth, fc->queue_depth,
				VTY_NEWLINE);
	}
}

static void dump_bssgp(struct vty *vty, int stats)
{
	struct bssgp_bvc_ctx *bvc;

	llist_for_each_entry(bvc, &bssgp_bvc_ctxts, list) {
		dump_bvc(vty, bvc, stats);
	}
}

DEFUN(bvc_reset, bvc_reset_cmd,
      "bssgp bvc nsei <0-65535> bvci <0-65535> reset",
      "Initiate BVC RESET procedure for a given NSEI and BVCI\n"
      "Filter based on BSSGP Virtual Connection\n"
      "NSEI of the BVC to be filtered\n"
      "Network Service Entity Identifier (NSEI)\n"
      "BVCI of the BVC to be filtered\n"
      "BSSGP Virtual Connection Identifier (BVCI)\n"
      "Perform reset procedure\n")
{
	int r;
	uint16_t nsei = atoi(argv[0]), bvci = atoi(argv[1]);
	struct bssgp_bvc_ctx *bvc = btsctx_by_bvci_nsei(bvci, nsei);
	if (!bvc) {
		vty_out(vty, "No BVC for NSEI %d BVCI %d%s", nsei, bvci,
			VTY_NEWLINE);
		return CMD_WARNING;
	}
	r = bssgp_tx_bvc_reset(bvc, bvci, BSSGP_CAUSE_OML_INTERV);
	vty_out(vty, "Sent BVC RESET for NSEI %d BVCI %d: %d%s", nsei, bvci, r,
		VTY_NEWLINE);
	return CMD_SUCCESS;
}

#define BSSGP_STR "Show information about the BSSGP protocol\n"

DEFUN(show_bssgp, show_bssgp_cmd, "show bssgp",
	SHOW_STR BSSGP_STR)
{
	dump_bssgp(vty, 0);
	return CMD_SUCCESS;
}

DEFUN(show_bssgp_stats, show_bssgp_stats_cmd, "show bssgp stats",
	SHOW_STR BSSGP_STR
	"Include statistics\n")
{
	dump_bssgp(vty, 1);
	return CMD_SUCCESS;
}

DEFUN(show_bvc, show_bvc_cmd, "show bssgp nsei <0-65535> [stats]",
	SHOW_STR BSSGP_STR
	"Show all BVCs on one NSE\n"
	"The NSEI\n" "Include Statistics\n")
{
	struct bssgp_bvc_ctx *bvc;
	uint16_t nsei = atoi(argv[0]);
	int show_stats = 0;

	if (argc >= 2)
		show_stats = 1;

	llist_for_each_entry(bvc, &bssgp_bvc_ctxts, list) {
		if (bvc->nsei != nsei)
			continue;
		dump_bvc(vty, bvc, show_stats);
	}

	return CMD_SUCCESS;
}

DEFUN(logging_fltr_bvc,
      logging_fltr_bvc_cmd,
      "logging filter bvc nsei <0-65535> bvci <0-65535>",
	LOGGING_STR FILTER_STR
	"Filter based on BSSGP Virtual Connection\n"
	"NSEI of the BVC to be filtered\n"
	"Network Service Entity Identifier (NSEI)\n"
	"BVCI of the BVC to be filtered\n"
	"BSSGP Virtual Connection Identifier (BVCI)\n")
{
	struct log_target *tgt;
	struct bssgp_bvc_ctx *bvc;
	uint16_t nsei = atoi(argv[0]);
	uint16_t bvci = atoi(argv[1]);

	log_tgt_mutex_lock();
	tgt = osmo_log_vty2tgt(vty);
	if (!tgt) {
		log_tgt_mutex_unlock();
		return CMD_WARNING;
	}

	bvc = btsctx_by_bvci_nsei(bvci, nsei);
	if (!bvc) {
		vty_out(vty, "No BVC by that identifier%s", VTY_NEWLINE);
		log_tgt_mutex_unlock();
		return CMD_WARNING;
	}

	log_set_bvc_filter(tgt, bvc);
	log_tgt_mutex_unlock();
	return CMD_SUCCESS;
}

int bssgp_vty_init(void)
{
	install_lib_element_ve(&show_bssgp_cmd);
	install_lib_element_ve(&show_bssgp_stats_cmd);
	install_lib_element_ve(&show_bvc_cmd);
	install_lib_element_ve(&logging_fltr_bvc_cmd);
	install_lib_element_ve(&bvc_reset_cmd);

	install_lib_element(CFG_LOG_NODE, &logging_fltr_bvc_cmd);

	install_lib_element(CONFIG_NODE, &cfg_bssgp_cmd);
	install_node(&bssgp_node, config_write_bssgp);

	return 0;
}
