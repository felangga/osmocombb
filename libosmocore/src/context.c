/*! \file context.c
 * talloc context handling.
 *
 * (C) 2019 by Harald Welte <laforge@gnumonks.org>
 * All Rights Reserverd.
 *
 * SPDX-License-Identifier: GPL-2.0+
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301, USA.
 */
#include <string.h>
#include <errno.h>
#include <osmocom/core/talloc.h>
#include <osmocom/core/utils.h>

__thread struct osmo_talloc_contexts *osmo_ctx;

int osmo_ctx_init(const char *id)
{
	osmo_ctx = talloc_named(NULL, sizeof(*osmo_ctx), "global-%s", id);
	if (!osmo_ctx)
		return -ENOMEM;
	memset(osmo_ctx, 0, sizeof(*osmo_ctx));
	osmo_ctx->global = osmo_ctx;
	osmo_ctx->select = talloc_named_const(osmo_ctx->global, 0, "select");
	if (!osmo_ctx->select) {
		talloc_free(osmo_ctx);
		return -ENOMEM;
	}
	return 0;
}

/* initialize osmo_ctx on main tread */
static __attribute__((constructor)) void on_dso_load_ctx(void)
{
	OSMO_ASSERT(osmo_ctx_init("main") == 0);
}

/*! @} */
