/*
 * SHA1 hash implementation and interface functions
 * Copyright (c) 2003-2009, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef SHA1_H
#define SHA1_H

#include "includes.h"

#define SHA1_MAC_LEN 20

int hmac_sha1_vector(const u8 *key, size_t key_len, size_t num_elem,
		     const u8 *addr[], const size_t *len, u8 *mac);
int sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len,
		u8 *mac);
int hmac_sha1(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
	       u8 *mac);
int pbkdf2_sha1(const char *passphrase, const u8 *ssid, size_t ssid_len,
		int iterations, u8 *buf, size_t buflen);
#endif /* SHA1_H */
