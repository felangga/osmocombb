#include <stdio.h>
#include <stdlib.h>

#include <osmocom/core/conv.h>
#include <osmocom/gsm/gsm0503.h>

#include "conv.h"

/* ------------------------------------------------------------------------ */
/* Test codes                                                               */
/* ------------------------------------------------------------------------ */

/* GMR-1 TCH3 Speech -> Non recursive code, tail-biting, punctured */
static const uint8_t conv_gmr1_tch3_speech_next_output[][2] = {
	{  0,  3 }, {  1,  2 }, {  3,  0 }, {  2,  1 },
	{  3,  0 }, {  2,  1 }, {  0,  3 }, {  1,  2 },
	{  0,  3 }, {  1,  2 }, {  3,  0 }, {  2,  1 },
	{  3,  0 }, {  2,  1 }, {  0,  3 }, {  1,  2 },
	{  2,  1 }, {  3,  0 }, {  1,  2 }, {  0,  3 },
	{  1,  2 }, {  0,  3 }, {  2,  1 }, {  3,  0 },
	{  2,  1 }, {  3,  0 }, {  1,  2 }, {  0,  3 },
	{  1,  2 }, {  0,  3 }, {  2,  1 }, {  3,  0 },
	{  3,  0 }, {  2,  1 }, {  0,  3 }, {  1,  2 },
	{  0,  3 }, {  1,  2 }, {  3,  0 }, {  2,  1 },
	{  3,  0 }, {  2,  1 }, {  0,  3 }, {  1,  2 },
	{  0,  3 }, {  1,  2 }, {  3,  0 }, {  2,  1 },
	{  1,  2 }, {  0,  3 }, {  2,  1 }, {  3,  0 },
	{  2,  1 }, {  3,  0 }, {  1,  2 }, {  0,  3 },
	{  1,  2 }, {  0,  3 }, {  2,  1 }, {  3,  0 },
	{  2,  1 }, {  3,  0 }, {  1,  2 }, {  0,  3 },
};

static const uint8_t conv_gmr1_tch3_speech_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
};

static const int conv_gmr1_tch3_speech_puncture[] = {
	 3,  7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47,
	51, 55, 59, 63, 67, 71, 75, 79, 83, 87, 91, 95,
	-1, /* end */
};

static const struct osmo_conv_code conv_gmr1_tch3_speech = {
	.N = 2,
	.K = 7,
	.len = 48,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_gmr1_tch3_speech_next_output,
	.next_state  = conv_gmr1_tch3_speech_next_state,
	.puncture = conv_gmr1_tch3_speech_puncture,
};


/* WiMax FCH -> Non recursive code, tail-biting, non-punctured */
static const uint8_t conv_wimax_fch_next_output[][2] = {
	{  0,  3 }, {  2,  1 }, {  3,  0 }, {  1,  2 },
	{  3,  0 }, {  1,  2 }, {  0,  3 }, {  2,  1 },
	{  0,  3 }, {  2,  1 }, {  3,  0 }, {  1,  2 },
	{  3,  0 }, {  1,  2 }, {  0,  3 }, {  2,  1 },
	{  1,  2 }, {  3,  0 }, {  2,  1 }, {  0,  3 },
	{  2,  1 }, {  0,  3 }, {  1,  2 }, {  3,  0 },
	{  1,  2 }, {  3,  0 }, {  2,  1 }, {  0,  3 },
	{  2,  1 }, {  0,  3 }, {  1,  2 }, {  3,  0 },
	{  3,  0 }, {  1,  2 }, {  0,  3 }, {  2,  1 },
	{  0,  3 }, {  2,  1 }, {  3,  0 }, {  1,  2 },
	{  3,  0 }, {  1,  2 }, {  0,  3 }, {  2,  1 },
	{  0,  3 }, {  2,  1 }, {  3,  0 }, {  1,  2 },
	{  2,  1 }, {  0,  3 }, {  1,  2 }, {  3,  0 },
	{  1,  2 }, {  3,  0 }, {  2,  1 }, {  0,  3 },
	{  2,  1 }, {  0,  3 }, {  1,  2 }, {  3,  0 },
	{  1,  2 }, {  3,  0 }, {  2,  1 }, {  0,  3 },
};

static const uint8_t conv_wimax_fch_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
};

static const struct osmo_conv_code conv_wimax_fch = {
	.N = 2,
	.K = 7,
	.len = 48,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_wimax_fch_next_output,
	.next_state  = conv_wimax_fch_next_state,
};

/**
 * LTE PBCH
 * Non recursive code, tail-biting, non-punctured
 */
static const uint8_t conv_lte_pbch_next_output[][2] = {
	{  0,  7 }, {  3,  4 }, {  7,  0 }, { 4 ,  3 },
	{  6,  1 }, {  5,  2 }, {  1,  6 }, { 2 ,  5 },
	{  1,  6 }, {  2,  5 }, {  6,  1 }, { 5 ,  2 },
	{  7,  0 }, {  4,  3 }, {  0,  7 }, { 3 ,  4 },
	{  4,  3 }, {  7,  0 }, {  3,  4 }, { 0 ,  7 },
	{  2,  5 }, {  1,  6 }, {  5,  2 }, { 6 ,  1 },
	{  5,  2 }, {  6,  1 }, {  2,  5 }, { 1 ,  6 },
	{  3,  4 }, {  0,  7 }, {  4,  3 }, { 7 ,  0 },
	{  7,  0 }, {  4,  3 }, {  0,  7 }, { 3 ,  4 },
	{  1,  6 }, {  2,  5 }, {  6,  1 }, { 5 ,  2 },
	{  6,  1 }, {  5,  2 }, {  1,  6 }, { 2 ,  5 },
	{  0,  7 }, {  3,  4 }, {  7,  0 }, { 4 ,  3 },
	{  3,  4 }, {  0,  7 }, {  4,  3 }, { 7 ,  0 },
	{  5,  2 }, {  6,  1 }, {  2,  5 }, { 1 ,  6 },
	{  2,  5 }, {  1,  6 }, {  5,  2 }, { 6 ,  1 },
	{  4,  3 }, {  7,  0 }, {  3,  4 }, { 0 ,  7 },
};

static const uint8_t conv_lte_pbch_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
};

const struct osmo_conv_code conv_lte_pbch = {
	.N = 3,
	.K = 7,
	.len = 40,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_lte_pbch_next_output,
	.next_state  = conv_lte_pbch_next_state,
};

/* ------------------------------------------------------------------------ */
/* Main                                                                     */
/* ------------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
	const struct conv_test_vector *test;
	int rc;

	/* Random code -> Non recursive code, direct truncation, non-punctured */
	const struct osmo_conv_code conv_trunc = {
		.N = 2,
		.K = 5,
		.len = 224,
		.term = CONV_TERM_TRUNCATION,
		.next_output = gsm0503_xcch.next_output,
		.next_state  = gsm0503_xcch.next_state,
	};

	const struct conv_test_vector tests[] = {
		{
			.name = "GSM xCCH (non-recursive, flushed, not punctured)",
			.code = &gsm0503_xcch,
			.in_len  = 224,
			.out_len = 456,
			.has_vec = 1,
			.vec_in  = { 0xf3, 0x1d, 0xb4, 0x0c, 0x4d, 0x1d, 0x9d, 0xae,
						 0xc0, 0x0a, 0x42, 0x57, 0x13, 0x60, 0x80, 0x96,
						 0xef, 0x23, 0x7e, 0x4c, 0x1d, 0x96, 0x24, 0x19,
						 0x17, 0xf2, 0x44, 0x99 },
			.vec_out = { 0xe9, 0x4d, 0x70, 0xab, 0xa2, 0x87, 0xf0, 0xe7,
						 0x04, 0x14, 0x7c, 0xab, 0xaf, 0x6b, 0xa1, 0x16,
						 0xeb, 0x30, 0x00, 0xde, 0xc8, 0xfd, 0x0b, 0x85,
						 0x80, 0x41, 0x4a, 0xcc, 0xd3, 0xc0, 0xd0, 0xb6,
						 0x26, 0xe5, 0x4e, 0x32, 0x49, 0x69, 0x38, 0x17,
						 0x33, 0xab, 0xaf, 0xb6, 0xc1, 0x08, 0xf3, 0x9f,
						 0x8c, 0x75, 0x6a, 0x4e, 0x08, 0xc4, 0x20, 0x5f,
						 0x8f },
		},
		{
			.name = "GSM TCH/AFS 7.95 (recursive, flushed, punctured)",
			.code = &gsm0503_tch_afs_7_95,
			.in_len  = 165,
			.out_len = 448,
			.has_vec = 1,
			.vec_in  = { 0x87, 0x66, 0xc3, 0x58, 0x09, 0xd4, 0x06, 0x59,
						 0x10, 0xbf, 0x6b, 0x7f, 0xc8, 0xed, 0x72, 0xaa,
						 0xc1, 0x3d, 0xf3, 0x1e, 0xb0 },
			.vec_out = { 0x92, 0xbc, 0xde, 0xa0, 0xde, 0xbe, 0x01, 0x2f,
						 0xbe, 0xe4, 0x61, 0x32, 0x4d, 0x4f, 0xdc, 0x41,
						 0x43, 0x0d, 0x15, 0xe0, 0x23, 0xdd, 0x18, 0x91,
						 0xe5, 0x36, 0x2d, 0xb7, 0xd9, 0x78, 0xb8, 0xb1,
						 0xb7, 0xcb, 0x2f, 0xc0, 0x52, 0x8f, 0xe2, 0x8c,
						 0x6f, 0xa6, 0x79, 0x88, 0xed, 0x0c, 0x2e, 0x9e,
						 0xa1, 0x5f, 0x45, 0x4a, 0xfb, 0xe6, 0x5a, 0x9c },
		},
		{
			.name = "GMR-1 TCH3 Speech (non-recursive, tail-biting, punctured)",
			.code = &conv_gmr1_tch3_speech,
			.in_len  = 48,
			.out_len = 72,
			.has_vec = 1,
			.vec_in  = { 0x4d, 0xcb, 0xfc, 0x72, 0xf4, 0x8c },
			.vec_out = { 0xc0, 0x86, 0x63, 0x4b, 0x8b, 0xd4, 0x6a, 0x76, 0xb2 },
		},
		{
			.name = "WiMax FCH (non-recursive, tail-biting, not punctured)",
			.code = &conv_wimax_fch,
			.in_len  = 48,
			.out_len = 96,
			.has_vec = 1,
			.vec_in  = { 0xfc, 0xa0, 0xa0, 0xfc, 0xa0, 0xa0 },
			.vec_out = { 0x19, 0x42, 0x8a, 0xed, 0x21, 0xed, 0x19, 0x42,
						 0x8a, 0xed, 0x21, 0xed },
		},
		{
			.name = "LTE PBCH (non-recursive, tail-biting, non-punctured)",
			.code = &conv_lte_pbch,
			.in_len  = 40,
			.out_len = 120,
			.has_vec = 0,
			.vec_in  = { },
			.vec_out = { },
		},
		{
			.name = "??? (non-recursive, direct truncation, not punctured)",
			.code = &conv_trunc,
			.in_len  = 224,
			.out_len = 448,
			.has_vec = 1,
			.vec_in  = { 0xe5, 0xe0, 0x85, 0x7e, 0xf7, 0x08, 0x19, 0x5a,
						 0xb9, 0xad, 0x82, 0x37, 0x98, 0x8b, 0x26, 0xb9,
						 0x81, 0x26, 0x9c, 0x75, 0xaf, 0xf3, 0xcb, 0x07,
						 0xac, 0x63, 0xe2, 0x9c,
			},
			.vec_out = { 0xea, 0x3b, 0x55, 0x0c, 0xd3, 0xf7, 0x85, 0x69,
						 0xe5, 0x79, 0x83, 0xd3, 0xc3, 0x9f, 0xb8, 0x61,
						 0x21, 0x63, 0x51, 0x18, 0xac, 0xcd, 0x32, 0x49,
						 0x53, 0x5c, 0x13, 0x1d, 0xbe, 0x05, 0x11, 0x63,
						 0x5c, 0xc3, 0x42, 0x05, 0x1c, 0x68, 0x0a, 0xb4,
						 0x61, 0x15, 0xaa, 0x4d, 0x94, 0xed, 0xb3, 0x3a,
						 0x5d, 0x1b, 0x09, 0xc2, 0x99, 0x01, 0xec, 0x68 },
		},
		{ /* end */ },
	};

	for (test = tests; test->name; test++) {
		rc = do_check(test);
		if (rc)
			return rc;
	}

	return 0;
}
