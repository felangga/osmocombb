/*
 * (C) 2008 by Holger Hans Peter Freyther <zecke@selfish.org>
 * (C) 2011 by Harald Welte <laforge@gnumonks.org>
 * All Rights Reserved
 *
 * Authors: Holger Hans Peter Freyther <zecke@selfish.org>
 *	    Pablo Neira Ayuso <pablo@gnumonks.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>

#include <osmocom/core/talloc.h>
#include <osmocom/core/timer.h>
#include <osmocom/core/select.h>
#include <osmocom/core/linuxlist.h>

#include "../config.h"

static void main_timer_fired(void *data);
static void secondary_timer_fired(void *data);

static unsigned int main_timer_step = 0;
static struct osmo_timer_list main_timer;

static LLIST_HEAD(timer_test_list);

struct test_timer {
	struct llist_head head;
	struct osmo_timer_list timer;
	struct timeval start;
	struct timeval stop;
};

/* number of test steps. We add fact(steps) timers in the whole test. */
#define MAIN_TIMER_NSTEPS	8

/* time between two steps, in secs. */
#define TIME_BETWEEN_STEPS	1

/* how much time elapses between checks, in microsecs */
#define TIME_BETWEEN_TIMER_CHECKS 423210

static int timer_nsteps = MAIN_TIMER_NSTEPS;
static unsigned int expired_timers = 0;
static unsigned int total_timers = 0;
static unsigned int too_late = 0;
static unsigned int too_soon = 0;

static void main_timer_fired(void *data)
{
	unsigned int *step = data;
	unsigned int add_in_this_step;
	int i;
	printf("main_timer_fired()\n");

	if (*step == timer_nsteps) {
		printf("Main timer has finished, please, "
		       "wait a bit for the final report.\n");
		return;
	}
	/* add 2^step pair of timers per step. */
	add_in_this_step = (1 << *step);

	for (i=0; i<add_in_this_step; i++) {
		struct test_timer *v;

		v = talloc_zero(NULL, struct test_timer);
		if (v == NULL) {
			printf("timer_test: OOM!\n");
			return;
		}
		osmo_gettimeofday(&v->start, NULL);
		osmo_timer_setup(&v->timer, secondary_timer_fired, v);
		unsigned int seconds = (i & 0x7) + 1;
		v->stop.tv_sec = v->start.tv_sec + seconds;
		v->stop.tv_usec = v->start.tv_usec;
		osmo_timer_schedule(&v->timer, seconds, 0);
		llist_add(&v->head, &timer_test_list);
		printf("scheduled timer at %d.%06d\n",
		       (int)v->stop.tv_sec, (int)v->stop.tv_usec);
	}
	printf("added %d timers in step %u (expired=%u)\n",
		add_in_this_step, *step, expired_timers);
	total_timers += add_in_this_step;
	osmo_timer_schedule(&main_timer, TIME_BETWEEN_STEPS, 0);
	(*step)++;
}

static void secondary_timer_fired(void *data)
{
	struct test_timer *v = data, *this, *tmp;
	struct timeval current, res;
	struct timeval precision = { 0, TIME_BETWEEN_TIMER_CHECKS + 1};
	int i, deleted;

	osmo_gettimeofday(&current, NULL);

	timersub(&current, &v->stop, &res);
	if (timercmp(&res, &precision, >)) {
		printf("ERROR: timer has expired too late:"
		       " wanted %d.%06d now %d.%06d diff %d.%06d\n",
		       (int)v->stop.tv_sec, (int)v->stop.tv_usec,
		       (int)current.tv_sec, (int)current.tv_usec,
		       (int)res.tv_sec, (int)res.tv_usec);
		too_late++;
	}
	else if (timercmp(&current, &v->stop, <)) {
		printf("ERROR: timer has expired too soon:"
		       " wanted %d.%06d now %d.%06d diff %d.%06d\n",
		       (int)v->stop.tv_sec, (int)v->stop.tv_usec,
		       (int)current.tv_sec, (int)current.tv_usec,
		       (int)res.tv_sec, (int)res.tv_usec);
		too_soon++;
	}
	else
		printf("timer fired on time: %d.%06d (+ %d.%06d)\n",
		       (int)v->stop.tv_sec, (int)v->stop.tv_usec,
		       (int)res.tv_sec, (int)res.tv_usec);

	llist_del(&v->head);
	talloc_free(data);
	expired_timers++;
	if (expired_timers == total_timers) {
		printf("test over: added=%u expired=%u too_soon=%u too_late=%u\n",
		       total_timers, expired_timers, too_soon, too_late);
		exit(EXIT_SUCCESS);
	}

	/* "random" deletion of timers. */
	i = 0;
	deleted = 0;
	llist_for_each_entry_safe(this, tmp, &timer_test_list, head) {
		i ++;
		if (!(i & 0x3)) {
			osmo_timer_del(&this->timer);
			llist_del(&this->head);
			talloc_free(this);
			deleted++;
		}
	}
	expired_timers += deleted;
	printf("early deleted %d timers, %d still active\n", deleted,
	       total_timers - expired_timers);
}

int main(int argc, char *argv[])
{
	int c;
	int steps;

	osmo_gettimeofday_override = true;

	while ((c = getopt_long(argc, argv, "s:", NULL, NULL)) != -1) {
	switch(c) {
		case 's':
			timer_nsteps = atoi(optarg);
			if (timer_nsteps <= 0) {
				fprintf(stderr, "%s: steps must be > 0\n",
					argv[0]);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

	steps = ((MAIN_TIMER_NSTEPS * TIME_BETWEEN_STEPS + 20) * 1e6)
		/ TIME_BETWEEN_TIMER_CHECKS;

	printf("Running timer test for %d iterations,"
	       " %d steps of %d msecs each\n",
	       timer_nsteps, steps, TIME_BETWEEN_TIMER_CHECKS / 1000);

	osmo_timer_setup(&main_timer, main_timer_fired, &main_timer_step);
	osmo_timer_schedule(&main_timer, 1, 0);

#ifdef HAVE_SYS_SELECT_H
	while (steps--) {
		printf("%d.%06d\n", (int)osmo_gettimeofday_override_time.tv_sec,
		       (int)osmo_gettimeofday_override_time.tv_usec);
		osmo_timers_prepare();
		osmo_timers_update();
		osmo_gettimeofday_override_add(0, TIME_BETWEEN_TIMER_CHECKS);
	}
#else
	printf("Select not supported on this platform!\n");
#endif
	return 0;
}
