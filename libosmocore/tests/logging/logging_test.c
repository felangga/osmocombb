/* simple test for the debug interface */
/*
 * (C) 2008, 2009 by Holger Hans Peter Freyther <zecke@selfish.org>
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

#include <osmocom/core/logging.h>
#include <osmocom/core/utils.h>

#include <stdlib.h>

enum {
	DRLL,
	DCC,
	DMM,
};

static int filter_called = 0;
static int select_output = 0;

static const struct log_info_cat default_categories[] = {
	[DRLL] = {
		.name = "DRLL",
		.description = "A-bis Radio Link Layer (RLL)",
		.color = OSMO_LOGCOLOR_RED,
		.enabled = 1, .loglevel = LOGL_NOTICE,
	},
	[DCC] = {
		.name = "DCC",
		.description = "Layer3 Call Control (CC)",
		.color = OSMO_LOGCOLOR_GREEN,
		.enabled = 1, .loglevel = LOGL_NOTICE,
	},
	[DMM] = {
		.name = NULL,
		.description = "Layer3 Mobility Management (MM)",
		.color = OSMO_LOGCOLOR_YELLOW,
		.enabled = 1, .loglevel = LOGL_NOTICE,
	},
};

static int test_filter(const struct log_context *ctx, struct log_target *target)
{
	filter_called += 1;
	/* omit everything */
	return select_output;
}

const struct log_info log_info = {
	.cat = default_categories,
	.num_cat = ARRAY_SIZE(default_categories),
	.filter_fn = test_filter,
};

extern struct log_info *osmo_log_info;

int main(int argc, char **argv)
{
	struct log_target *stderr_target;

	log_init(&log_info, NULL);
	stderr_target = log_target_create_stderr();
	log_add_target(stderr_target);
	log_set_all_filter(stderr_target, 1);
	log_set_print_filename2(stderr_target, LOG_FILENAME_NONE);
	log_set_print_category_hex(stderr_target, 0);
	log_set_print_category(stderr_target, 1);
	log_set_use_color(stderr_target, 0);

	log_parse_category_mask(stderr_target, "DRLL:DCC");
	log_parse_category_mask(stderr_target, "DRLL");

	select_output = 0;

	DEBUGP(DCC, "You should not see this\n");
	if (log_check_level(DMM, LOGL_DEBUG) != 0)
		fprintf(stderr, "log_check_level did not catch this case\n");

	log_parse_category_mask(stderr_target, "DRLL:DCC");
	DEBUGP(DRLL, "You should see this\n");
	OSMO_ASSERT(log_check_level(DRLL, LOGL_DEBUG) != 0);
	DEBUGP(DCC, "You should see this\n");
	OSMO_ASSERT(log_check_level(DCC, LOGL_DEBUG) != 0);
	DEBUGP(DMM, "You should not see this\n");

	OSMO_ASSERT(log_check_level(DMM, LOGL_DEBUG) == 0);
	OSMO_ASSERT(filter_called == 0);

	log_set_all_filter(stderr_target, 0);
	DEBUGP(DRLL, "You should not see this and filter is called\n");
	OSMO_ASSERT(filter_called == 1);
	OSMO_ASSERT(log_check_level(DRLL, LOGL_DEBUG) == 0);
	OSMO_ASSERT(filter_called == 2);

	DEBUGP(DRLL, "You should not see this\n");
	OSMO_ASSERT(filter_called == 3);
	select_output = 1;
	DEBUGP(DRLL, "You should see this\n");
	OSMO_ASSERT(filter_called == 5); /* called twice on output */

	/* Make sure out-of-bounds category maps to DLGLOBAL */
	log_parse_category_mask(stderr_target, "DLGLOBAL,1");
	/* For IDs out of bounds of the overall osmo_log_info array */
	DEBUGP(osmo_log_info->num_cat + 1, "You should see this on DLGLOBAL (a)\n");
	DEBUGP(osmo_log_info->num_cat + 100, "You should see this on DLGLOBAL (b)\n");
	DEBUGP(osmo_log_info->num_cat, "You should see this on DLGLOBAL (c)\n");
	/* For IDs out of bounds of the user categories part */
	DEBUGP(log_info.num_cat + 1, "You should see this on DLGLOBAL (d)\n");
	DEBUGP(log_info.num_cat, "You should see this on DLGLOBAL (e)\n");

	/* Check log_set_category_filter() with internal categories */
	log_parse_category_mask(stderr_target, "DLGLOBAL,3");
	DEBUGP(DLGLOBAL, "You should not see this (DLGLOBAL not on DEBUG)\n");
	log_set_category_filter(stderr_target, DLGLOBAL, 1, LOGL_DEBUG);
	DEBUGP(DLGLOBAL, "You should see this (DLGLOBAL on DEBUG)\n");

	return 0;
}
