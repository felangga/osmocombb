/*
 * (C) 2014 by On-Waves
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

#include <stdlib.h>
#include <osmocom/core/application.h>
#include <osmocom/core/logging.h>
#include <osmocom/core/utils.h>
#include <osmocom/core/msgb.h>
#include <setjmp.h>

#include <errno.h>

#include <string.h>

#define CHECK_RC(rc)	\
	if (rc != 0) {	\
		printf("Operation failed rc=%d on %s:%d\n", rc, __FILE__, __LINE__); \
		abort(); \
	}

static jmp_buf jmp_env;
static int jmp_env_valid = 0;
static void osmo_panic_raise(const char *fmt, va_list args)
{
	/*
	 * The args can include pointer values which are not suitable for
	 * regression testing. So just write the (hopefully constant) format
	 * string to stdout and write the full message to stderr.
	 */
	printf("%s", fmt);
	vfprintf(stderr, fmt, args);
	if (!jmp_env_valid)
		abort();
	longjmp(jmp_env, 1);
}

/* Note that this does not nest */
#define OSMO_PANIC_TRY(pE) (osmo_panic_try(pE, setjmp(jmp_env)))

static int osmo_panic_try(volatile int *exception, int setjmp_result)
{
	jmp_env_valid = setjmp_result == 0;
	*exception = setjmp_result;

	if (setjmp_result)
		fprintf(stderr, "Exception caught: %d\n", setjmp_result);

	return *exception == 0;
}

static void test_msgb_api()
{
	struct msgb *msg = msgb_alloc_headroom(4096, 128, "data");
	unsigned char *cptr = NULL;
	int rc;

	printf("Testing the msgb API\n");

	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_test_invariant(msg));
	cptr = msg->l1h = msgb_put(msg, 4);
	printf("put(4) -> data%+td\n", cptr - msg->data);
	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_test_invariant(msg));
	cptr = msg->l2h = msgb_put(msg, 4);
	printf("put(4) -> data%+td\n", cptr - msg->data);
	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_test_invariant(msg));
	cptr = msg->l3h = msgb_put(msg, 4);
	printf("put(4) -> data%+td\n", cptr - msg->data);
	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_test_invariant(msg));
	cptr = msg->l4h = msgb_put(msg, 4);
	printf("put(4) -> data%+td\n", cptr - msg->data);
	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_test_invariant(msg));
	OSMO_ASSERT(msgb_length(msg) == 16);
	cptr = msgb_push(msg, 4);
	printf("push(4) -> data%+td\n", cptr - msg->data);
	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_test_invariant(msg));
	OSMO_ASSERT(msgb_length(msg) == 20);
	rc = msgb_trim(msg, 16);
	printf("trim(16) -> %d\n", rc);
	CHECK_RC(rc);
	OSMO_ASSERT(msgb_test_invariant(msg));
	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_length(msg) == 16);

	cptr = msgb_get(msg, 4);
	printf("get(4) -> data%+td\n", cptr - msg->data);
	printf("Buffer: %s\n", msgb_hexdump(msg));
	OSMO_ASSERT(msgb_test_invariant(msg));
	OSMO_ASSERT(msgb_length(msg) == 12);

	printf("Test msgb_hexdump\n");
	msg->l1h = msg->head;
	printf("Buffer: %s\n", msgb_hexdump(msg));
	msg->l3h = msg->data;
	printf("Buffer: %s\n", msgb_hexdump(msg));
	msg->l3h = msg->head - 1;
	printf("Buffer: %s\n", msgb_hexdump(msg));

	msgb_free(msg);
}

static void test_msgb_api_errors()
{
	struct msgb *msg = msgb_alloc_headroom(4096, 128, "data");
	volatile int e = 0;
	int rc;

	printf("Testing the msgb API error handling\n");

	osmo_set_panic_handler(osmo_panic_raise);

	if (OSMO_PANIC_TRY(&e))
		msgb_trim(msg, -1);
	OSMO_ASSERT(e != 0);

	rc = msgb_trim(msg, 4096 + 500);
	OSMO_ASSERT(rc == -1);

	msgb_free(msg);
	osmo_set_panic_handler(NULL);
}

static void test_msgb_copy()
{
	struct msgb *msg = msgb_alloc_headroom(4096, 128, "data");
	struct msgb *msg2;
	int i;

	printf("Testing msgb_copy\n");

	msg->l1h = msgb_put(msg, 20);
	msg->l2h = msgb_put(msg, 20);
	msg->l3h = msgb_put(msg, 20);
	msg->l4h = msgb_put(msg, 20);

	OSMO_ASSERT(msgb_length(msg) == 80);
	for (i = 0; i < msgb_length(msg); i++)
		msg->data[i] = (uint8_t)i;

	msg2 = msgb_copy(msg, "copy");

	OSMO_ASSERT(msgb_length(msg) == msgb_length(msg2));
	OSMO_ASSERT(msgb_l1len(msg) == msgb_l1len(msg2));
	OSMO_ASSERT(msgb_l2len(msg) == msgb_l2len(msg2));
	OSMO_ASSERT(msgb_l3len(msg) == msgb_l3len(msg2));
	OSMO_ASSERT(msg->tail - msg->l4h == msg2->tail - msg2->l4h);

	for (i = 0; i < msgb_length(msg2); i++)
		OSMO_ASSERT(msg2->data[i] == (uint8_t)i);

	printf("Src: %s\n", msgb_hexdump(msg));
	printf("Dst: %s\n", msgb_hexdump(msg));

	msgb_free(msg);
	msgb_free(msg2);
}

static void test_msgb_resize_area()
{
	struct msgb *msg = msgb_alloc_headroom(4096, 128, "data");
	int rc;
	volatile int e = 0;
	int i, saved_i;
	uint8_t *cptr, *old_l3h;

	osmo_set_panic_handler(osmo_panic_raise);

	rc = msgb_resize_area(msg, msg->data, 0, 0);
	OSMO_ASSERT(rc >= 0);

	if (OSMO_PANIC_TRY(&e))
		msgb_resize_area(msg, NULL, 0, 0);
	OSMO_ASSERT(e != 0);

	if (OSMO_PANIC_TRY(&e))
		msgb_resize_area(msg, NULL, 0, 0);
	OSMO_ASSERT(e != 0);

	if (OSMO_PANIC_TRY(&e))
		msgb_resize_area(msg, msg->data, 20, 0);
	OSMO_ASSERT(e != 0);

	if (OSMO_PANIC_TRY(&e))
		msgb_resize_area(msg, msg->data, -1, 0);
	OSMO_ASSERT(e != 0);

	if (OSMO_PANIC_TRY(&e))
		msgb_resize_area(msg, msg->data, 0, -1);
	OSMO_ASSERT(e != 0);

	printf("Testing msgb_resize_area\n");

	msg->l1h = msgb_put(msg, 20);
	msg->l2h = msgb_put(msg, 20);
	msg->l3h = msgb_put(msg, 20);
	msg->l4h = msgb_put(msg, 20);

	for (i = 0; i < msgb_length(msg); i++)
		msg->data[i] = (uint8_t)i;

	printf("Original: %s\n", msgb_hexdump(msg));

	/* Extend area */
	saved_i = msg->l3h[0];
	old_l3h = msg->l3h;

	rc = msgb_resize_area(msg, msg->l2h, 20, 20 + 30);

	/* Reset the undefined part to allow printing the buffer to stdout */
	memset(old_l3h, 0, msg->l3h - old_l3h);

	printf("Extended: %s\n", msgb_hexdump(msg));

	OSMO_ASSERT(rc >= 0);
	OSMO_ASSERT(msgb_length(msg) == 80 + 30);
	OSMO_ASSERT(msgb_l1len(msg) == 80 + 30);
	OSMO_ASSERT(msgb_l2len(msg) == 60 + 30);
	OSMO_ASSERT(msgb_l3len(msg) == 40);
	OSMO_ASSERT(msg->tail - msg->l4h == 20);

	for (cptr = msgb_data(msg), i = 0; cptr < old_l3h; cptr++, i++)
		OSMO_ASSERT(*cptr == (uint8_t)i);

	for (cptr = msg->l3h, i = saved_i; cptr < msg->tail; cptr++, i++)
		OSMO_ASSERT(*cptr == (uint8_t)i);

	rc = msgb_resize_area(msg, msg->l2h, 50, 8000);
	OSMO_ASSERT(rc == -1);

	/* Shrink area */
	saved_i = msg->l4h[0];
	OSMO_ASSERT(saved_i == (uint8_t)(msg->l4h[-1] + 1));

	rc = msgb_resize_area(msg, msg->l3h, 20, 10);

	printf("Shrinked: %s\n", msgb_hexdump(msg));

	OSMO_ASSERT(rc >= 0);
	OSMO_ASSERT(msgb_length(msg) == 80 + 30 - 10);
	OSMO_ASSERT(msgb_l1len(msg) == 80 + 30 - 10);
	OSMO_ASSERT(msgb_l2len(msg) == 60 + 30 - 10);
	OSMO_ASSERT(msgb_l3len(msg) == 40 - 10);
	OSMO_ASSERT(msg->tail - msg->l4h == 20);

	OSMO_ASSERT(msg->l4h[0] != msg->l4h[-1] - 1);

	for (cptr = msg->l4h, i = saved_i; cptr < msg->tail; cptr++, i++)
		OSMO_ASSERT(*cptr == (uint8_t)i);

	rc = msgb_resize_area(msg, msg->l2h, 50, 8000);
	OSMO_ASSERT(rc == -1);

	msgb_free(msg);

	osmo_set_panic_handler(NULL);
}

static void test_msgb_printf()
{
	struct msgb *msg;
	struct msgb *msg_ref;
	int rc;
	int total_len;

	msg = msgb_alloc(80, "data");

	/* Add normal text: */
	printf("Add normal text:\n");
	rc = msgb_printf(msg, "|this is a test %i, %s, %16x|", 4711, "testme",
			 0x4711);
	total_len = msgb_length(msg);
	printf("#1: rc=%i, total_len=%i, msg->data=%s\n", rc, total_len,
	       msg->data);
	OSMO_ASSERT(rc == 0);
	OSMO_ASSERT(msgb_tailroom(msg) == 33);

	/* Add normal text: */
	rc = msgb_printf(msg, "|some more text|");
	total_len = msgb_length(msg);
	printf("#2: rc=%i, total_len=%i, msg->data=%s\n", rc, total_len,
	       msg->data);
	OSMO_ASSERT(rc == 0);
	OSMO_ASSERT(msgb_tailroom(msg) == 17);

	/* Add normal text which will not fit: */
	rc = msgb_printf(msg, "|more %i %x %s|", 23, 0xfee,
			 "text will not fit");
	total_len = msgb_length(msg);
	printf("#3: rc=%i, total_len=%i, msg->data=%s\n", rc, total_len,
	       msg->data);
	OSMO_ASSERT(rc == -EINVAL);

	/* Check if we got the right amount of characters in the message buffer
	 * until here, so that we can be sure that the following cornercase
	 * tests yield plausible results */
	OSMO_ASSERT(msgb_tailroom(msg) == 17);

	/* Add normal text which just does not fit by one character, this should not
	 * alter the message buffers tail pointer */
	rc = msgb_printf(msg, "|more 123456 ABC|");
	total_len = msgb_length(msg);
	printf("#4: rc=%i, total_len=%i, msg->data=%s\n", rc, total_len,
	       msg->data);
	OSMO_ASSERT(rc == -EINVAL);

	/* Make sure the tailroom, nor the contained string length did change */
	OSMO_ASSERT(msgb_tailroom(msg) == 17);
	OSMO_ASSERT(msgb_length(msg) == 63);

	/* Add normal text which just fits */
	rc = msgb_printf(msg, "|more 123456 AB|");
	total_len = msgb_length(msg);
	printf("#5: rc=%i, total_len=%i, msg->data=%s\n", rc, total_len,
	       msg->data);
	OSMO_ASSERT(rc == 0);

	/* Make sure that the string in the bufer takes up all available Space.
	 * Also make sure that the available tailroom has space for one byte,
	 * which actually holds the string terminator */
	OSMO_ASSERT(msgb_tailroom(msg) == 1);
	OSMO_ASSERT(msgb_length(msg) == 79);

	/* Try to add a nullstring to the already full buffer, this should
	 * be ok, since we still have one byte tailroom */
	rc = msgb_printf(msg, "");
	total_len = msgb_length(msg);
	printf("#6: rc=%i, total_len=%i, msg->data=%s\n", rc, total_len,
	       msg->data);
	OSMO_ASSERT(rc == 0);

	/* Make sure that we still have the same conditions as before */
	OSMO_ASSERT(msgb_tailroom(msg) == 1);
	OSMO_ASSERT(msgb_length(msg) == 79);

	msgb_free(msg);

	/* Test behaviour when a completely full buffer is passed to
	 * msgb_printf(). We should get rc == -EINVAL and the message
	 * buffer must not be changed at all */
	msg = msgb_alloc(15, "data");
	msg_ref = msgb_alloc(msgb_tailroom(msg), "data");
	memset(msg->data, 0x41, msgb_tailroom(msg));
	memset(msg_ref->data, 0x41, msgb_tailroom(msg_ref));
	msgb_put(msg, msgb_tailroom(msg));
	msgb_put(msg_ref, msgb_tailroom(msg_ref));

	printf("#7: before: %s", msgb_hexdump(msg));
	rc = msgb_printf(msg, "ABCDEF");
	printf(" after: rc=%i, %s", rc, msgb_hexdump(msg));
	if (memcmp(msg->data, msg_ref->data, msgb_length(msg)) == 0)
		printf(" ==> ok, no change\n");
	else
		printf(" ==> error, change detected\n");
	OSMO_ASSERT(rc == -EINVAL);
	OSMO_ASSERT(msgb_tailroom(msg) == 0);
	OSMO_ASSERT(msgb_length(msg) == msgb_length(msg_ref));

	msgb_free(msg);
	msgb_free(msg_ref);
}

static struct log_info info = {};

int main(int argc, char **argv)
{
	void *ctx = talloc_named_const(NULL, 0, "msgb_test");
	osmo_init_logging2(ctx, &info);
	msgb_talloc_ctx_init(ctx, 0);

	test_msgb_api();
	test_msgb_api_errors();
	test_msgb_copy();
	test_msgb_resize_area();
	test_msgb_printf();

	printf("Success.\n");

	return 0;
}
