/*
 * Copyright (c) 2012  Marko Kreen
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "pghashlib.h"

#include <endian.h>

static inline uint64_t rol64(uint64_t v, int s)
{
	return (v << s) | (v >> (64 - s));
}

static inline uint64_t sip_le64dec(const void *p)
{
	uint64_t v;
	memcpy(&v, p, 8);
	return le64toh(v);
}

#define SIP_ROUND1 \
    v0 += v1; v1 = rol64(v1, 13); v1 ^= v0; v0 = rol64(v0, 32);	\
    v2 += v3; v3 = rol64(v3, 16); v3 ^= v2;			\
    v0 += v3; v3 = rol64(v3, 21); v3 ^= v0;			\
    v2 += v1; v1 = rol64(v1, 17); v1 ^= v2; v2 = rol64(v2, 32)
#define SIP_ROUND2	SIP_ROUND1; SIP_ROUND1
#define SIP_ROUND4	SIP_ROUND2; SIP_ROUND2
#define SIP_ROUNDS(n)	SIP_ROUND ## n

#define sip_compress(n)		\
	do {			\
		v3 ^= m;	\
		SIP_ROUNDS(n);	\
		v0 ^= m;	\
	} while (0)

#define sip_finalize(n)		\
	do {			\
		v2 ^= 0xff;	\
		SIP_ROUNDS(n);	\
	} while (0)

static uint64_t siphash24(const void *data, size_t len, uint64_t k0, uint64_t k1)
{
	const uint8_t *s = data;
	const uint8_t *end = s + len - (len % 8);
	uint64_t v0 = k0 ^ UINT64_C(0x736f6d6570736575);
	uint64_t v1 = k1 ^ UINT64_C(0x646f72616e646f6d);
	uint64_t v2 = k0 ^ UINT64_C(0x6c7967656e657261);
	uint64_t v3 = k1 ^ UINT64_C(0x7465646279746573);
	uint64_t m;

	for (; s < end; s += 8) {
		m = sip_le64dec(s);
		sip_compress(2);
	}

	m = (uint64_t)len << 56;
	switch (len & 7) {
	case 7: m |= (uint64_t)s[6] << 48;
	case 6: m |= (uint64_t)s[5] << 40;
	case 5: m |= (uint64_t)s[4] << 32;
	case 4: m |= (uint64_t)s[3] << 24;
	case 3: m |= (uint64_t)s[2] << 16;
	case 2: m |= (uint64_t)s[1] <<  8;
	case 1: m |= (uint64_t)s[0]; break;
	case 0: break;
	}
	sip_compress(2);

	sip_finalize(4);
	return (v0 ^ v1 ^ v2 ^ v3);
}

void hlib_siphash24(const void *data, size_t len, uint64_t *io)
{
	io[0] = siphash24(data, len, io[0], io[1]);
}

