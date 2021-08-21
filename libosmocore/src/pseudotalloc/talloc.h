/*! \file talloc.h
 * overly simplistic talloc replacement for deeply embedded
 * microcontrollers.  Obviously this has none of the properties of real
 * talloc, it is particualrly not hierarchical at all.
 *
 * (C) 2017 by Harald Welte <laforge@gnumonks.org>
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
 */

#pragma once

#include <stdlib.h>
#include <stdarg.h>

/* those two functions have to be provided by the user/environment */
extern void *pseudotalloc_malloc(size_t size);
extern void  pseudotalloc_free(void *ptr);

typedef void TALLOC_CTX;

#define __TALLOC_STRING_LINE1__(s)    #s
#define __TALLOC_STRING_LINE2__(s)   __TALLOC_STRING_LINE1__(s)
#define __TALLOC_STRING_LINE3__  __TALLOC_STRING_LINE2__(__LINE__)
#define __location__ __FILE__ ":" __TALLOC_STRING_LINE3__

#define talloc_zero(ctx, type) (type *)_talloc_zero(ctx, sizeof(type), #type)
#define talloc_zero_size(ctx, size) _talloc_zero(ctx, size, __location__)
void *_talloc_zero(const void *ctx, size_t size, const char *name);

#define talloc_free(ctx) _talloc_free(ctx, __location__)
int _talloc_free(void *ptr, const char *location);

/* Unsupported! */
#define talloc(ctx, type) (type *)talloc_named_const(ctx, sizeof(type), #type)
#define talloc_size(ctx, size) talloc_named_const(ctx, size, __location__)
void *talloc_named_const(const void *context, size_t size, const char *name);
void *talloc_named(const void *context, size_t size, const char *fmt, ...);
void talloc_set_name_const(const void *ptr, const char *name);
char *talloc_strdup(const void *t, const char *p);
void *talloc_pool(const void *context, size_t size);
#define talloc_array(ctx, type, count) (type *)_talloc_array(ctx, sizeof(type), count, #type)
void *_talloc_array(const void *ctx, size_t el_size, unsigned count, const char *name);
#define talloc_zero_array(ctx, type, count) (type *)_talloc_zero_array(ctx, sizeof(type), count, #type)
void *_talloc_zero_array(const void *ctx,
			 size_t el_size,
			 unsigned count,
			 const char *name);
char *talloc_asprintf(const void *ctx, const char *fmt, ...);

#define talloc_steal(ctx, ptr) _talloc_steal_loc((ctx), (ptr), __location__)
void *_talloc_steal_loc(const void *new_ctx, const void *obj, const char *location);
char *talloc_vasprintf(const void *t, const char *fmt, va_list ap);
