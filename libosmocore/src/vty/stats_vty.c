/*
 * (C) 2009-2010 by Harald Welte <laforge@gnumonks.org>
 * (C) 2009-2014 by Holger Hans Peter Freyther
 * (C) 2015      by sysmocom - s.f.m.c. GmbH
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "../../config.h"

#include <osmocom/vty/command.h>
#include <osmocom/vty/buffer.h>
#include <osmocom/vty/vty.h>
#include <osmocom/vty/telnet_interface.h>
#include <osmocom/vty/telnet_interface.h>
#include <osmocom/vty/misc.h>

#include <osmocom/core/stats.h>
#include <osmocom/core/counter.h>
#include <osmocom/core/rate_ctr.h>

#define CFG_STATS_STR "Configure stats sub-system\n"
#define CFG_REPORTER_STR "Configure a stats reporter\n"

#define SHOW_STATS_STR "Show statistical values\n"

#define STATS_STR "Stats related commands\n"

/*! \file stats_vty.c
 *  VTY interface for statsd / statistic items
 *
 *  This code allows you to register a couple of VTY commands that
 *  permit configuration of the \ref stats functionality from the VTY.
 *
 *  Use \ref osmo_stats_vty_add_cmds once at application start-up to
 *  enable related commands.
 */

/* containing version info */
extern struct host host;

struct cmd_node cfg_stats_node = {
	CFG_STATS_NODE,
	"%s(config-stats)# ",
	1
};

static const struct value_string stats_class_strs[] = {
	{ OSMO_STATS_CLASS_GLOBAL,     "global" },
	{ OSMO_STATS_CLASS_PEER,       "peer" },
	{ OSMO_STATS_CLASS_SUBSCRIBER, "subscriber" },
	{ 0, NULL }
};

static struct osmo_stats_reporter *osmo_stats_vty2srep(struct vty *vty)
{
	if (vty->node == CFG_STATS_NODE)
		return vty->index;

	return NULL;
}

static int set_srep_parameter_str(struct vty *vty,
	int (*fun)(struct osmo_stats_reporter *, const char *),
	const char *val, const char *param_name)
{
	int rc;
	struct osmo_stats_reporter *srep = osmo_stats_vty2srep(vty);
	OSMO_ASSERT(srep);

	rc = fun(srep, val);
	if (rc < 0) {
		vty_out(vty, "%% Unable to set %s: %s%s",
			param_name, strerror(-rc), VTY_NEWLINE);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

static int set_srep_parameter_int(struct vty *vty,
	int (*fun)(struct osmo_stats_reporter *, int),
	const char *val, const char *param_name)
{
	int rc;
	int int_val;
	struct osmo_stats_reporter *srep = osmo_stats_vty2srep(vty);
	OSMO_ASSERT(srep);

	int_val = atoi(val);

	rc = fun(srep, int_val);
	if (rc < 0) {
		vty_out(vty, "%% Unable to set %s: %s%s",
			param_name, strerror(-rc), VTY_NEWLINE);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(cfg_stats_reporter_local_ip, cfg_stats_reporter_local_ip_cmd,
	"local-ip ADDR",
	"Set the IP address to which we bind locally\n"
	"IP Address\n")
{
	return set_srep_parameter_str(vty, osmo_stats_reporter_set_local_addr,
		argv[0], "local address");
}

DEFUN(cfg_no_stats_reporter_local_ip, cfg_no_stats_reporter_local_ip_cmd,
	"no local-ip",
	NO_STR
	"Set the IP address to which we bind locally\n")
{
	return set_srep_parameter_str(vty, osmo_stats_reporter_set_local_addr,
		NULL, "local address");
}

DEFUN(cfg_stats_reporter_remote_ip, cfg_stats_reporter_remote_ip_cmd,
	"remote-ip ADDR",
	"Set the remote IP address to which we connect\n"
	"IP Address\n")
{
	return set_srep_parameter_str(vty, osmo_stats_reporter_set_remote_addr,
		argv[0], "remote address");
}

DEFUN(cfg_stats_reporter_remote_port, cfg_stats_reporter_remote_port_cmd,
	"remote-port <1-65535>",
	"Set the remote port to which we connect\n"
	"Remote port number\n")
{
	return set_srep_parameter_int(vty, osmo_stats_reporter_set_remote_port,
		argv[0], "remote port");
}

DEFUN(cfg_stats_reporter_mtu, cfg_stats_reporter_mtu_cmd,
	"mtu <100-65535>",
	"Set the maximum packet size\n"
	"Size in byte\n")
{
	return set_srep_parameter_int(vty, osmo_stats_reporter_set_mtu,
		argv[0], "mtu");
}

DEFUN(cfg_no_stats_reporter_mtu, cfg_no_stats_reporter_mtu_cmd,
	"no mtu",
	NO_STR "Set the maximum packet size\n")
{
	return set_srep_parameter_int(vty, osmo_stats_reporter_set_mtu,
		"0", "mtu");
}

DEFUN(cfg_stats_reporter_prefix, cfg_stats_reporter_prefix_cmd,
	"prefix PREFIX",
	"Set the item name prefix\n"
	"The prefix string\n")
{
	return set_srep_parameter_str(vty, osmo_stats_reporter_set_name_prefix,
		argv[0], "prefix string");
}

DEFUN(cfg_no_stats_reporter_prefix, cfg_no_stats_reporter_prefix_cmd,
	"no prefix",
	NO_STR
	"Set the item name prefix\n")
{
	return set_srep_parameter_str(vty, osmo_stats_reporter_set_name_prefix,
		"", "prefix string");
}

DEFUN(cfg_stats_reporter_level, cfg_stats_reporter_level_cmd,
	"level (global|peer|subscriber)",
	"Set the maximum group level\n"
	"Report global groups only\n"
	"Report global and network peer related groups\n"
	"Report global, peer, and subscriber groups\n")
{
	int level = get_string_value(stats_class_strs, argv[0]);
	int rc;
	struct osmo_stats_reporter *srep = osmo_stats_vty2srep(vty);

	OSMO_ASSERT(srep);
	rc = osmo_stats_reporter_set_max_class(srep, level);
	if (rc < 0) {
		vty_out(vty, "%% Unable to set level: %s%s",
			strerror(-rc), VTY_NEWLINE);
		return CMD_WARNING;
	}

	return 0;
}

DEFUN(cfg_stats_reporter_enable, cfg_stats_reporter_enable_cmd,
	"enable",
	"Enable the reporter\n")
{
	int rc;
	struct osmo_stats_reporter *srep = osmo_stats_vty2srep(vty);
	OSMO_ASSERT(srep);

	rc = osmo_stats_reporter_enable(srep);
	if (rc < 0) {
		vty_out(vty, "%% Unable to enable the reporter: %s%s",
			strerror(-rc), VTY_NEWLINE);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(cfg_stats_reporter_disable, cfg_stats_reporter_disable_cmd,
	"disable",
	"Disable the reporter\n")
{
	int rc;
	struct osmo_stats_reporter *srep = osmo_stats_vty2srep(vty);
	OSMO_ASSERT(srep);

	rc = osmo_stats_reporter_disable(srep);
	if (rc < 0) {
		vty_out(vty, "%% Unable to disable the reporter: %s%s",
			strerror(-rc), VTY_NEWLINE);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(cfg_stats_reporter_flush_period, cfg_stats_reporter_flush_period_cmd,
	"flush-period <0-65535>",
	CFG_STATS_STR "Send all stats even if they have not changed (i.e. force the flush)"
	              "every N-th reporting interval. Set to 0 to disable regular flush (default).\n"
	"0 to disable regular flush (default), 1 to flush every time, 2 to flush every 2nd time, etc\n")
{
	int rc;
	unsigned int period = atoi(argv[0]);
	struct osmo_stats_reporter *srep = osmo_stats_vty2srep(vty);
	OSMO_ASSERT(srep);

	rc = osmo_stats_reporter_set_flush_period(srep, period);
	if (rc < 0) {
		vty_out(vty, "%% Unable to set force flush period: %s%s",
			strerror(-rc), VTY_NEWLINE);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(cfg_stats_reporter_statsd, cfg_stats_reporter_statsd_cmd,
	"stats reporter statsd",
	CFG_STATS_STR CFG_REPORTER_STR "Report to a STATSD server\n")
{
	struct osmo_stats_reporter *srep;

	srep = osmo_stats_reporter_find(OSMO_STATS_REPORTER_STATSD, NULL);
	if (!srep) {
		srep = osmo_stats_reporter_create_statsd(NULL);
		if (!srep) {
			vty_out(vty, "%% Unable to create statsd reporter%s",
				VTY_NEWLINE);
			return CMD_WARNING;
		}
		srep->max_class = OSMO_STATS_CLASS_GLOBAL;
		/* TODO: if needed, add osmo_stats_add_reporter(srep); */
	}

	vty->index = srep;
	vty->node = CFG_STATS_NODE;

	return CMD_SUCCESS;
}

DEFUN(cfg_no_stats_reporter_statsd, cfg_no_stats_reporter_statsd_cmd,
	"no stats reporter statsd",
	NO_STR CFG_STATS_STR CFG_REPORTER_STR "Report to a STATSD server\n")
{
	struct osmo_stats_reporter *srep;

	srep = osmo_stats_reporter_find(OSMO_STATS_REPORTER_STATSD, NULL);
	if (!srep) {
		vty_out(vty, "%% No statsd logging active%s",
			VTY_NEWLINE);
		return CMD_WARNING;
	}

	osmo_stats_reporter_free(srep);

	return CMD_SUCCESS;
}

DEFUN(cfg_stats_reporter_log, cfg_stats_reporter_log_cmd,
	"stats reporter log",
	CFG_STATS_STR CFG_REPORTER_STR "Report to the logger\n")
{
	struct osmo_stats_reporter *srep;

	srep = osmo_stats_reporter_find(OSMO_STATS_REPORTER_LOG, NULL);
	if (!srep) {
		srep = osmo_stats_reporter_create_log(NULL);
		if (!srep) {
			vty_out(vty, "%% Unable to create log reporter%s",
				VTY_NEWLINE);
			return CMD_WARNING;
		}
		srep->max_class = OSMO_STATS_CLASS_GLOBAL;
		/* TODO: if needed, add osmo_stats_add_reporter(srep); */
	}

	vty->index = srep;
	vty->node = CFG_STATS_NODE;

	return CMD_SUCCESS;
}

DEFUN(cfg_no_stats_reporter_log, cfg_no_stats_reporter_log_cmd,
	"no stats reporter log",
	NO_STR CFG_STATS_STR CFG_REPORTER_STR "Report to the logger\n")
{
	struct osmo_stats_reporter *srep;

	srep = osmo_stats_reporter_find(OSMO_STATS_REPORTER_LOG, NULL);
	if (!srep) {
		vty_out(vty, "%% No log reporting active%s",
			VTY_NEWLINE);
		return CMD_WARNING;
	}

	osmo_stats_reporter_free(srep);

	return CMD_SUCCESS;
}

DEFUN(cfg_stats_interval, cfg_stats_interval_cmd,
	"stats interval <0-65535>",
	CFG_STATS_STR "Set the reporting interval\n"
	"Interval in seconds (0 disables the reporting interval)\n")
{
	int rc;
	int interval = atoi(argv[0]);
	rc = osmo_stats_set_interval(interval);
	if (rc < 0) {
		vty_out(vty, "%% Unable to set interval: %s%s",
			strerror(-rc), VTY_NEWLINE);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(show_stats,
      show_stats_cmd,
      "show stats",
      SHOW_STR SHOW_STATS_STR)
{
	vty_out_statistics_full(vty, "");

	return CMD_SUCCESS;
}

DEFUN(show_stats_level,
      show_stats_level_cmd,
      "show stats level (global|peer|subscriber)",
      SHOW_STR SHOW_STATS_STR
      "Set the maximum group level\n"
      "Show global groups only\n"
      "Show global and network peer related groups\n"
      "Show global, peer, and subscriber groups\n")
{
	int level = get_string_value(stats_class_strs, argv[0]);
	vty_out_statistics_partial(vty, "", level);

	return CMD_SUCCESS;
}

static int asciidoc_handle_counter(struct osmo_counter *counter, void *sctx_)
{
	struct vty *vty = sctx_;
	char *name = osmo_asciidoc_escape(counter->name);
	char *description = osmo_asciidoc_escape(counter->description);

	/* | name | This document & | description | */
	vty_out(vty, "| %s | <<ungroup_counter_%s>> | %s%s",
		name,
		name,
		description ? description : "",
		VTY_NEWLINE);

	talloc_free(name);
	talloc_free(description);

	return 0;
}

static void asciidoc_counter_generate(struct vty *vty)
{
	vty_out(vty, "// ungrouped osmo_counters%s", VTY_NEWLINE);
	vty_out(vty, ".ungrouped osmo counters%s", VTY_NEWLINE);
	vty_out(vty, "[options=\"header\"]%s", VTY_NEWLINE);
	vty_out(vty, "|===%s", VTY_NEWLINE);
	vty_out(vty, "| Name | Reference | Description%s", VTY_NEWLINE);
	osmo_counters_for_each(asciidoc_handle_counter, vty);
	vty_out(vty, "|===%s", VTY_NEWLINE);
}

static int asciidoc_rate_ctr_handler(
	struct rate_ctr_group *ctrg, struct rate_ctr *ctr,
	const struct rate_ctr_desc *desc, void *sctx_)
{
	struct vty *vty = sctx_;
	char *name = osmo_asciidoc_escape(desc->name);
	char *description = osmo_asciidoc_escape(desc->description);
	char *group_name_prefix = osmo_asciidoc_escape(ctrg->desc->group_name_prefix);

	/* | Name | This document & | Description | */
	vty_out(vty, "| %s | <<%s_%s>> | %s%s",
		name,
		group_name_prefix,
		name,
		description ? description : NULL,
		VTY_NEWLINE);

	/* description seems to be optional */
	talloc_free(name);
	talloc_free(group_name_prefix);
	talloc_free(description);

	return 0;
}

static int asciidoc_rate_ctr_group_handler(struct rate_ctr_group *ctrg, void *sctx_)
{
	struct vty *vty = sctx_;

	char *group_description = osmo_asciidoc_escape(ctrg->desc->group_description);
	char *group_name_prefix = osmo_asciidoc_escape(ctrg->desc->group_name_prefix);

	vty_out(vty, "// rate_ctr_group table %s%s", group_description, VTY_NEWLINE);
	vty_out(vty, ".%s - %s%s", group_name_prefix, group_description, VTY_NEWLINE);
	vty_out(vty, "[options=\"header\"]%s", VTY_NEWLINE);
	vty_out(vty, "|===%s", VTY_NEWLINE);
	vty_out(vty, "| Name | Reference | Description%s", VTY_NEWLINE);
	rate_ctr_for_each_counter(ctrg, asciidoc_rate_ctr_handler, sctx_);
	vty_out(vty, "|===%s", VTY_NEWLINE);

	talloc_free(group_name_prefix);
	talloc_free(group_description);

	return 0;
}

static int asciidoc_osmo_stat_item_handler(
	struct osmo_stat_item_group *statg, struct osmo_stat_item *item, void *sctx_)
{
	struct vty *vty = sctx_;

	char *name = osmo_asciidoc_escape(item->desc->name);
	char *description = osmo_asciidoc_escape(item->desc->description);
	char *group_name_prefix = osmo_asciidoc_escape(statg->desc->group_name_prefix);
	char *unit = osmo_asciidoc_escape(item->desc->unit);

	/* | Name | Reference | Description | Unit | */
	vty_out(vty, "| %s | <<%s_%s>> | %s | %s%s",
		name,
		group_name_prefix,
		name,
		description ? description : "",
		unit ? unit : "",
		VTY_NEWLINE);

	talloc_free(name);
	talloc_free(group_name_prefix);
	talloc_free(description);
	talloc_free(unit);

	return 0;
}

static int asciidoc_osmo_stat_item_group_handler(struct osmo_stat_item_group *statg, void *sctx_)
{
	char *group_name_prefix = osmo_asciidoc_escape(statg->desc->group_name_prefix);
	char *group_description = osmo_asciidoc_escape(statg->desc->group_description);

	struct vty *vty = sctx_;
	vty_out(vty, "%s%s", group_description ? group_description : "" , VTY_NEWLINE);

	vty_out(vty, "// osmo_stat_item_group table %s%s", group_description ? group_description : "", VTY_NEWLINE);
	vty_out(vty, ".%s - %s%s", group_name_prefix, group_description ? group_description : "", VTY_NEWLINE);
	vty_out(vty, "[options=\"header\"]%s", VTY_NEWLINE);
	vty_out(vty, "|===%s", VTY_NEWLINE);
	vty_out(vty, "| Name | Reference | Description | Unit%s", VTY_NEWLINE);
	osmo_stat_item_for_each_item(statg, asciidoc_osmo_stat_item_handler, sctx_);
	vty_out(vty, "|===%s", VTY_NEWLINE);

	talloc_free(group_name_prefix);
	talloc_free(group_description);

	return 0;
}

DEFUN(show_stats_asciidoc_table,
      show_stats_asciidoc_table_cmd,
      "show asciidoc counters",
      SHOW_STR "Asciidoc generation\n" "Generate table of all registered counters\n")
{
	vty_out(vty, "// autogenerated by show asciidoc counters%s", VTY_NEWLINE);
	vty_out(vty, "These counters and their description are based on %s %s (%s).%s%s",
		host.app_info->name,
		host.app_info->version,
		host.app_info->name ? host.app_info->name : "", VTY_NEWLINE, VTY_NEWLINE);
	/* 2x VTY_NEWLINE are intentional otherwise it would interpret the first table header
	 * as usual text*/
	vty_out(vty, "=== Rate Counters%s%s", VTY_NEWLINE, VTY_NEWLINE);
	vty_out(vty, "// generating tables for rate_ctr_group%s", VTY_NEWLINE);
	rate_ctr_for_each_group(asciidoc_rate_ctr_group_handler, vty);

	vty_out(vty, "=== Osmo Stat Items%s%s", VTY_NEWLINE, VTY_NEWLINE);
	vty_out(vty, "// generating tables for osmo_stat_items%s", VTY_NEWLINE);
	osmo_stat_item_for_each_group(asciidoc_osmo_stat_item_group_handler, vty);

	if (osmo_counters_count() == 0)
	{
		vty_out(vty, "// there are no ungrouped osmo_counters%s",
			VTY_NEWLINE);
	} else {
		vty_out(vty, "=== Osmo Counters%s%s", VTY_NEWLINE, VTY_NEWLINE);
		vty_out(vty, "// generating tables for osmo_counters%s", VTY_NEWLINE);
		asciidoc_counter_generate(vty);
	}
	return CMD_SUCCESS;
}

static int rate_ctr_group_handler(struct rate_ctr_group *ctrg, void *sctx_)
{
	struct vty *vty = sctx_;
	vty_out(vty, "%s %u", ctrg->desc->group_description, ctrg->idx);
	if (ctrg->name != NULL)
		vty_out(vty, " (%s)", ctrg->name);
	vty_out(vty, ":%s", VTY_NEWLINE);
	vty_out_rate_ctr_group_fmt(vty, "%25n: %10c (%S/s %M/m %H/h %D/d) %d", ctrg);
	return 0;
}

DEFUN(show_rate_counters,
      show_rate_counters_cmd,
      "show rate-counters",
      SHOW_STR "Show all rate counters\n")
{
	rate_ctr_for_each_group(rate_ctr_group_handler, vty);
	return CMD_SUCCESS;
}

DEFUN(stats_report,
      stats_report_cmd,
      "stats report",
      STATS_STR "Manurally trigger reporting of stats\n")
{
        osmo_stats_report();
	return CMD_SUCCESS;
}

static int reset_rate_ctr_group_handler(struct rate_ctr_group *ctrg, void *sctx_)
{
        rate_ctr_group_reset(ctrg);
        return 0;
}

static int reset_osmo_stat_item_group_handler(struct osmo_stat_item_group *statg, void *sctx_)
{
        osmo_stat_item_group_reset(statg);
        return 0;
}

DEFUN(stats_reset,
      stats_reset_cmd,
      "stats reset",
      STATS_STR "Reset all stats\n")
{
        rate_ctr_for_each_group(reset_rate_ctr_group_handler, NULL);
        osmo_stat_item_for_each_group(reset_osmo_stat_item_group_handler, NULL);
	return CMD_SUCCESS;
}

static int config_write_stats_reporter(struct vty *vty, struct osmo_stats_reporter *srep)
{
	if (srep == NULL)
		return 0;

	switch (srep->type) {
	case OSMO_STATS_REPORTER_STATSD:
		vty_out(vty, "stats reporter statsd%s", VTY_NEWLINE);
		break;
	case OSMO_STATS_REPORTER_LOG:
		vty_out(vty, "stats reporter log%s", VTY_NEWLINE);
		break;
	}

	vty_out(vty, "  disable%s", VTY_NEWLINE);

	if (srep->have_net_config) {
		if (srep->dest_addr_str)
			vty_out(vty, "  remote-ip %s%s",
				srep->dest_addr_str, VTY_NEWLINE);
		if (srep->dest_port)
			vty_out(vty, "  remote-port %d%s",
				srep->dest_port, VTY_NEWLINE);
		if (srep->bind_addr_str)
			vty_out(vty, "  local-ip %s%s",
				srep->bind_addr_str, VTY_NEWLINE);
		if (srep->mtu)
			vty_out(vty, "  mtu %d%s",
				srep->mtu, VTY_NEWLINE);
	}

	if (srep->max_class)
		vty_out(vty, "  level %s%s",
			get_value_string(stats_class_strs, srep->max_class),
			VTY_NEWLINE);

	if (srep->name_prefix && *srep->name_prefix)
		vty_out(vty, "  prefix %s%s",
			srep->name_prefix, VTY_NEWLINE);
	else
		vty_out(vty, "  no prefix%s", VTY_NEWLINE);

	if (srep->flush_period > 0)
		vty_out(vty, "  flush-period %d%s",
			srep->flush_period, VTY_NEWLINE);

	if (srep->enabled)
		vty_out(vty, "  enable%s", VTY_NEWLINE);

	return 1;
}

static int config_write_stats(struct vty *vty)
{
	struct osmo_stats_reporter *srep;

	/* TODO: loop through all reporters */
	srep = osmo_stats_reporter_find(OSMO_STATS_REPORTER_STATSD, NULL);
	config_write_stats_reporter(vty, srep);
	srep = osmo_stats_reporter_find(OSMO_STATS_REPORTER_LOG, NULL);
	config_write_stats_reporter(vty, srep);

	vty_out(vty, "stats interval %d%s", osmo_stats_config->interval, VTY_NEWLINE);

	return 1;
}

/*! Add stats related commands to the VTY
 *  Call this once during your application initialization if you would
 *  like to have stats VTY commands enabled.
 */
void osmo_stats_vty_add_cmds()
{
	install_lib_element_ve(&show_stats_cmd);
	install_lib_element_ve(&show_stats_level_cmd);

	install_lib_element(CONFIG_NODE, &cfg_stats_reporter_statsd_cmd);
	install_lib_element(CONFIG_NODE, &cfg_no_stats_reporter_statsd_cmd);
	install_lib_element(CONFIG_NODE, &cfg_stats_reporter_log_cmd);
	install_lib_element(CONFIG_NODE, &cfg_no_stats_reporter_log_cmd);
	install_lib_element(CONFIG_NODE, &cfg_stats_interval_cmd);

	install_node(&cfg_stats_node, config_write_stats);

	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_local_ip_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_no_stats_reporter_local_ip_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_remote_ip_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_remote_port_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_mtu_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_no_stats_reporter_mtu_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_prefix_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_no_stats_reporter_prefix_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_level_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_enable_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_disable_cmd);
	install_lib_element(CFG_STATS_NODE, &cfg_stats_reporter_flush_period_cmd);

	install_lib_element_ve(&show_stats_asciidoc_table_cmd);
	install_lib_element_ve(&show_rate_counters_cmd);

        install_lib_element(ENABLE_NODE, &stats_report_cmd);
        install_lib_element(ENABLE_NODE, &stats_reset_cmd);
}
