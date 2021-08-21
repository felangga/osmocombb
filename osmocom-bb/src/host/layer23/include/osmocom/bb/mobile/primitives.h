#pragma once

#include <osmocom/bb/mobile/gsm411_sms.h>

#include <osmocom/core/prim.h>

struct mobile_prim;

/**
 * Mobile Script<->App primitives. Application script will receive
 * indications and will send primitives to the lower layers. Here
 * we will convert from internal state/events to the primitives. In
 * the future the indications might be generated at lower levels
 * directly.
 */
enum mobile_prims {
	PRIM_MOB_TIMER,
	PRIM_MOB_TIMER_CANCEL,
	PRIM_MOB_STARTED,
	PRIM_MOB_SHUTDOWN,
	PRIM_MOB_SMS,
	PRIM_MOB_MM,
	PRIM_MOB_NETWORK_RESELECT,
};

struct mobile_prim_intf {
	struct osmocom_ms *ms;
	void (*indication)(struct mobile_prim_intf *, struct mobile_prim *prim);

	/* Internal state */
	struct llist_head entry;
	struct llist_head timers;
};

/**
 * Primitive to create timers and get indication once they have
 * expired. Currently there is no way to cancel timers.
 */
struct mobile_timer_param {
	uint64_t timer_id;	  	/*!< Unique Id identifying the timer */
	int seconds;			/*!< Seconds the timer should fire in */
};

/**
 * Primitive to indicate starting of the mobile.
 */
struct mobile_started_param {
	bool started;
};

/**
 * Primitive to indicate shutdown of the mobile. It will go through
 * various states.
 */
struct mobile_shutdown_param {
	int old_state;
	int new_state;
};

/**
 * SMS related configs.
 */
struct mobile_sms_param {
	struct gsm_sms sms;

	char sca[20+1]; /*<! Service Centre Address. For SubmitMS */

	bool cause_valid;
	int cause;
};

/**
 * Mobility Management (MM) state changes.
 */
struct mobile_mm_param {
	int state;			/*!< The new MM state */
	int substate;			/*!< The current substate */
	int prev_substate;		/*!< The previous substate */
};

struct mobile_prim {
	struct osmo_prim_hdr hdr;	/*!< Primitive base class */
	union {
		struct mobile_timer_param timer;
		struct mobile_started_param started;
		struct mobile_shutdown_param shutdown;
		struct mobile_sms_param sms;
		struct mobile_mm_param mm;
	} u;
};


struct mobile_prim_intf *mobile_prim_intf_alloc(struct osmocom_ms *ms);
int mobile_prim_intf_req(struct mobile_prim_intf *intf, struct mobile_prim *hdr);
void mobile_prim_intf_free(struct mobile_prim_intf *intf);

struct mobile_prim *mobile_prim_alloc(unsigned int primitive, enum osmo_prim_operation op);

void mobile_prim_ntfy_started(struct osmocom_ms *ms, bool started);
void mobile_prim_ntfy_shutdown(struct osmocom_ms *ms, int old_state, int new_state);
void mobile_prim_ntfy_sms_new(struct osmocom_ms *ms, struct gsm_sms *sms);
void mobile_prim_ntfy_sms_status(struct osmocom_ms *ms, struct gsm_sms *sms, uint8_t cause);
void mobile_prim_ntfy_mm_status(struct osmocom_ms *ms, int state, int subs, int old_subs);
