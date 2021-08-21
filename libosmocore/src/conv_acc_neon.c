/*! \file conv_acc_neon.c
 * Accelerated Viterbi decoder implementation
 * for architectures with only NEON available. */
/*
 * (C) 2020 by sysmocom - s.f.m.c. GmbH
 * Author: Eric Wild
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include "config.h"

#if defined(HAVE_NEON)
#include <arm_neon.h>
#endif

/* align req is 16 on android because google was confused, 8 on sane platforms */
#define NEON_ALIGN 8

#include <conv_acc_neon_impl.h>

/* Aligned Memory Allocator
 * NEON requires 8-byte memory alignment. We store relevant trellis values
 * (accumulated sums, outputs, and path decisions) as 16 bit signed integers
 * so the allocated memory is casted as such.
 */
__attribute__ ((visibility("hidden")))
int16_t *osmo_conv_neon_vdec_malloc(size_t n)
{
	return (int16_t *) memalign(NEON_ALIGN, sizeof(int16_t) * n);
}

__attribute__ ((visibility("hidden")))
void osmo_conv_neon_vdec_free(int16_t *ptr)
{
	free(ptr);
}

__attribute__ ((visibility("hidden")))
void osmo_conv_neon_metrics_k5_n2(const int8_t *val, const int16_t *out,
	int16_t *sums, int16_t *paths, int norm)
{
	const int16_t _val[4] = { val[0], val[1], val[0], val[1] };

	_neon_metrics_k5_n2(_val, out, sums, paths, norm);
}

__attribute__ ((visibility("hidden")))
void osmo_conv_neon_metrics_k5_n3(const int8_t *val, const int16_t *out,
	int16_t *sums, int16_t *paths, int norm)
{
	const int16_t _val[4] = { val[0], val[1], val[2], 0 };

	_neon_metrics_k5_n4(_val, out, sums, paths, norm);
}

__attribute__ ((visibility("hidden")))
void osmo_conv_neon_metrics_k5_n4(const int8_t *val, const int16_t *out,
	int16_t *sums, int16_t *paths, int norm)
{
	const int16_t _val[4] = { val[0], val[1], val[2], val[3] };

	_neon_metrics_k5_n4(_val, out, sums, paths, norm);
}

__attribute__ ((visibility("hidden")))
void osmo_conv_neon_metrics_k7_n2(const int8_t *val, const int16_t *out,
	int16_t *sums, int16_t *paths, int norm)
{
	const int16_t _val[4] = { val[0], val[1], val[0], val[1] };

	_neon_metrics_k7_n2(_val, out, sums, paths, norm);
}

__attribute__ ((visibility("hidden")))
void osmo_conv_neon_metrics_k7_n3(const int8_t *val, const int16_t *out,
	int16_t *sums, int16_t *paths, int norm)
{
	const int16_t _val[4] = { val[0], val[1], val[2], 0 };

	_neon_metrics_k7_n4(_val, out, sums, paths, norm);
}

__attribute__ ((visibility("hidden")))
void osmo_conv_neon_metrics_k7_n4(const int8_t *val, const int16_t *out,
	int16_t *sums, int16_t *paths, int norm)
{
	const int16_t _val[4] = { val[0], val[1], val[2], val[3] };

	_neon_metrics_k7_n4(_val, out, sums, paths, norm);
}
