/*! \file telnet_interface.h
 * minimalistic telnet/network interface it might turn into a wire interface */
/*
 * (C) 2009 by Holger Hans Peter Freyther <zecke@selfish.org>
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

#pragma once

#include <osmocom/core/logging.h>
#include <osmocom/core/select.h>

#include <osmocom/vty/vty.h>

/*! \defgroup telnet_interface Telnet Interface
 *  @{
 * \file telnet_interface.h */

/*! A telnet connection */
struct telnet_connection {
	/*! linked list header for internal management */
	struct llist_head entry;
	/*! private data pointer passed through */
	void *priv;
	/*! filedsecriptor (socket ) */
	struct osmo_fd fd;
	/*! VTY instance associated with telnet connection */
	struct vty *vty;
	/*! logging target associated with this telnet connection */
	struct log_target *dbg;
};

int telnet_init(void *tall_ctx, void *priv, int port);
int telnet_init_dynif(void *tall_ctx, void *priv, const char *ip, int port);
int telnet_init_default(void *tall_ctx, void *priv, int default_port);

void telnet_exit(void);

/*! @} */