/*! \file comp128.h
 * COMP128 header.
 *
 * See comp128.c for details
 */

#pragma once

#include <stdint.h>

#include <osmocom/core/defs.h>

/*
 * Performs the COMP128v1 algorithm (used as A3/A8)
 * ki    : uint8_t [16]
 * srand : uint8_t [16]
 * sres  : uint8_t [4]
 * kc    : uint8_t [8]
 */
void comp128v1(const uint8_t *ki, const uint8_t *srand, uint8_t *sres, uint8_t *kc);

void comp128(const uint8_t *ki, const uint8_t *srand, uint8_t *sres, uint8_t *kc) OSMO_DEPRECATED("Use generic API from osmocom/crypt/auth.h instead");
