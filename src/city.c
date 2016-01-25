// Copyright (c) 2011 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// CityHash, by Geoff Pike and Jyrki Alakuijala
//
// This file provides a few functions for hashing strings. On x86-64
// hardware in 2011, CityHash64() is faster than other high-quality
// hash functions, such as Murmur.  This is largely due to higher
// instruction-level parallelism.  CityHash64() and CityHash128() also perform
// well on hash-quality tests.
//
// CityHash128() is optimized for relatively long strings and returns
// a 128-bit hash.  For strings more than about 2000 bytes it can be
// faster than CityHash64().
//
// Functions in the CityHash family are not suitable for cryptography.
//
// WARNING: This code has not been tested on big-endian platforms!
// It is known to work well on little-endian platforms that have a small penalty
// for unaligned reads, such as current Intel and AMD moderate-to-high-end CPUs.
//
// By the way, for some hash functions, given strings a and b, the hash
// of a+b is easily derived from the hashes of a and b.  This property
// doesn't hold for any hash functions in this file.
//
// It's probably possible to create even faster hash functions by
// writing a program that systematically explores some of the space of
// possible hash functions, by using SIMD instructions, or by
// compromising on hash quality.

#include "pghashlib.h"

#if defined __GNUC__ && __GNUC__ >= 3
#define HAVE_BUILTIN_EXPECT 1
#endif

struct city_uint128 {
	uint64_t first;
	uint64_t second;
};
typedef struct city_uint128 city_uint128;

static inline uint64_t Uint128Low64(const city_uint128 x) { return x.first; }
static inline uint64_t Uint128High64(const city_uint128 x) { return x.second; }

#define uint32_in_expected_order(x) le32toh(x)
#define uint64_in_expected_order(x) le64toh(x)

#if !defined(LIKELY)
#if HAVE_BUILTIN_EXPECT
#define LIKELY(x) (__builtin_expect(!!(x), 1))
#else
#define LIKELY(x) (x)
#endif
#endif

static uint64_t UNALIGNED_LOAD64(const char *p)
{
	uint64_t result;
	memcpy(&result, p, sizeof(result));
	return result;
}

static uint32_t UNALIGNED_LOAD32(const char *p)
{
	uint32_t result;
	memcpy(&result, p, sizeof(result));
	return result;
}

static inline uint64_t Fetch64(const char *p)
{
	return uint64_in_expected_order(UNALIGNED_LOAD64(p));
}

static inline uint32_t Fetch32(const char *p)
{
	return uint32_in_expected_order(UNALIGNED_LOAD32(p));
}

// Hash 128 input bits down to 64 bits of output.
// This is intended to be a reasonably good hash function.
static inline uint64_t Hash128to64(uint64_t u, uint64_t v)
{
	uint64_t a, b, kMul;
	// Murmur-inspired hashing.
	kMul = 0x9ddfea08eb382d69ULL;
	a = (u ^ v) * kMul;
	a ^= (a >> 47);
	b = (v ^ a) * kMul;
	b ^= (b >> 47);
	b *= kMul;
	return b;
}

// Some primes between 2^63 and 2^64 for various uses.
#define k0 0xc3a5c85c97cb3127ULL
#define k1 0xb492b66fbe98f273ULL
#define k2 0x9ae16a3b2f90404fULL
#define k3 0xc949d7c7509e6557ULL

// Bitwise right rotate.  Normally this will compile to a single
// instruction, especially if the shift is a manifest constant.
static inline uint64_t Rotate(uint64_t val, int shift)
{
	// Avoid shifting by 64: doing so yields an undefined result.
	return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

// Equivalent to Rotate(), but requires the second arg to be non-zero.
// On x86-64, and probably others, it's possible for this to compile
// to a single instruction if both args are already in registers.
static inline uint64_t RotateByAtLeast1(uint64_t val, int shift)
{
	return (val >> shift) | (val << (64 - shift));
}

static inline uint64_t ShiftMix(uint64_t val)
{
	return val ^ (val >> 47);
}

static inline uint64_t HashLen16(uint64_t u, uint64_t v)
{
	return Hash128to64(u, v);
}

static inline uint64_t HashLen0to16(const char *s, size_t len)
{
	if (len > 8) {
		uint64_t a = Fetch64(s);
		uint64_t b = Fetch64(s + len - 8);
		return HashLen16(a, RotateByAtLeast1(b + len, len)) ^ b;
	}
	if (len >= 4) {
		uint64_t a = Fetch32(s);
		return HashLen16(len + (a << 3), Fetch32(s + len - 4));
	}
	if (len > 0) {
		uint8_t a = s[0];
		uint8_t b = s[len >> 1];
		uint8_t c = s[len - 1];
		uint32_t y = (uint32_t) (a) + ((uint32_t) (b) << 8);
		uint32_t z = len + ((uint32_t) (c) << 2);
		return ShiftMix(y * k2 ^ z * k3) * k2;
	}
	return k2;
}

// This probably works well for 16-byte strings as well, but it may be overkill
// in that case.
static inline uint64_t HashLen17to32(const char *s, size_t len)
{
	uint64_t a = Fetch64(s) * k1;
	uint64_t b = Fetch64(s + 8);
	uint64_t c = Fetch64(s + len - 8) * k2;
	uint64_t d = Fetch64(s + len - 16) * k0;
	return HashLen16(Rotate(a - b, 43) + Rotate(c, 30) + d,
			 a + Rotate(b ^ k3, 20) - c + len);
}

// Return a 16-byte hash for 48 bytes.  Quick and dirty.
// Callers do best to use "random-looking" values for a and b.
static city_uint128 WeakHashLen32WithSeeds_raw(uint64_t w, uint64_t x,
					  uint64_t y, uint64_t z,
					  uint64_t a, uint64_t b)
{
	uint64_t c;
	city_uint128 res;

	a += w;
	b = Rotate(b + a + z, 21);
	c = a;
	a += x;
	a += y;
	b += Rotate(a, 44);
	res.first = a + z;
	res.second = b + c;
	return res;
}

// Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
static city_uint128 WeakHashLen32WithSeeds(const char *s, uint64_t a, uint64_t b)
{
	return WeakHashLen32WithSeeds_raw(Fetch64(s),
					  Fetch64(s + 8),
					  Fetch64(s + 16),
					  Fetch64(s + 24), a, b);
}

// Return an 8-byte hash for 33 to 64 bytes.
static uint64_t HashLen33to64(const char *s, size_t len)
{
	uint64_t vf, vs, wf, ws, r;
	uint64_t a, b, c, z;
	z = Fetch64(s + 24);
	a = Fetch64(s) + (len + Fetch64(s + len - 16)) * k0;
	b = Rotate(a + z, 52);
	c = Rotate(a, 37);
	a += Fetch64(s + 8);
	c += Rotate(a, 7);
	a += Fetch64(s + 16);
	vf = a + z;
	vs = b + Rotate(a, 31) + c;
	a = Fetch64(s + 16) + Fetch64(s + len - 32);
	z = Fetch64(s + len - 8);
	b = Rotate(a + z, 52);
	c = Rotate(a, 37);
	a += Fetch64(s + len - 24);
	c += Rotate(a, 7);
	a += Fetch64(s + len - 16);
	wf = a + z;
	ws = b + Rotate(a, 31) + c;
	r = ShiftMix((vf + ws) * k2 + (wf + vs) * k0);
	return ShiftMix(r * k0 + vs) * k2;
}

static uint64_t CityHash64(const char *s, size_t len)
{
	uint64_t x, y, z, tmp;
	city_uint128 v, w;

	if (len <= 32) {
		if (len <= 16) {
			return HashLen0to16(s, len);
		} else {
			return HashLen17to32(s, len);
		}
	} else if (len <= 64) {
		return HashLen33to64(s, len);
	}

	// For strings over 64 bytes we hash the end first, and then as we
	// loop we keep 56 bytes of state: v, w, x, y, and z.
	x = Fetch64(s + len - 40);
	y = Fetch64(s + len - 16) + Fetch64(s + len - 56);
	z = HashLen16(Fetch64(s + len - 48) + len, Fetch64(s + len - 24));
	v = WeakHashLen32WithSeeds(s + len - 64, len, z);
	w = WeakHashLen32WithSeeds(s + len - 32, y + k1, x);
	x = x * k1 + Fetch64(s);

	// Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
	len = (len - 1) & ~(size_t) (63);
	do {
		x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
		y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
		x ^= w.second;
		y += v.first + Fetch64(s + 40);
		z = Rotate(z + w.first, 33) * k1;
		v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
		w = WeakHashLen32WithSeeds(s + 32, z + w.second,
					   y + Fetch64(s + 16));
		tmp = z;
		z = x;
		x = tmp;	//std::swap(z, x);
		s += 64;
		len -= 64;
	} while (len != 0);

	return HashLen16(HashLen16(v.first, w.first) + ShiftMix(y) * k1 + z,
			 HashLen16(v.second, w.second) + x);
}

static uint64_t CityHash64WithSeeds(const char *s, size_t len, uint64_t seed0, uint64_t seed1)
{
	return HashLen16(CityHash64(s, len) - seed0, seed1);
}

static uint64_t CityHash64WithSeed(const char *s, size_t len, uint64_t seed)
{
	return CityHash64WithSeeds(s, len, k2, seed);
}

// A subroutine for CityHash128().  Returns a decent 128-bit hash for strings
// of any length representable in signed long.  Based on City and Murmur.
static city_uint128 CityMurmur(const char *s, size_t len, city_uint128 seed)
{
	uint64_t a = Uint128Low64(seed);
	uint64_t b = Uint128High64(seed);
	uint64_t c = 0;
	uint64_t d = 0;
	signed long l = len - 16;
	city_uint128 res;

	if (l <= 0) {		// len <= 16
		a = ShiftMix(a * k1) * k1;
		c = b * k1 + HashLen0to16(s, len);
		d = ShiftMix(a + (len >= 8 ? Fetch64(s) : c));
	} else {		// len > 16
		c = HashLen16(Fetch64(s + len - 8) + k1, a);
		d = HashLen16(b + len, c + Fetch64(s + len - 16));
		a += d;
		do {
			a ^= ShiftMix(Fetch64(s) * k1) * k1;
			a *= k1;
			b ^= a;
			c ^= ShiftMix(Fetch64(s + 8) * k1) * k1;
			c *= k1;
			d ^= c;
			s += 16;
			l -= 16;
		} while (l > 0);
	}
	a = HashLen16(a, c);
	b = HashLen16(d, b);
	res.first = a ^ b;
	res.second = HashLen16(b, a);
	return res;
}

static city_uint128 CityHash128WithSeed(const char *s, size_t len, city_uint128 seed)
{
	city_uint128 v, w;
	uint64_t x, y, z, tmp;
	size_t tail_done;
	city_uint128 res;

	if (len < 128)
		return CityMurmur(s, len, seed);

	// We expect len >= 128 to be the common case.  Keep 56 bytes of state:
	// v, w, x, y, and z.
	x = Uint128Low64(seed);
	y = Uint128High64(seed);
	z = len * k1;
	v.first = Rotate(y ^ k1, 49) * k1 + Fetch64(s);
	v.second = Rotate(v.first, 42) * k1 + Fetch64(s + 8);
	w.first = Rotate(y + z, 35) * k1 + x;
	w.second = Rotate(x + Fetch64(s + 88), 53) * k1;

	// This is the same inner loop as CityHash64(), manually unrolled.
	do {
		x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
		y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
		x ^= w.second;
		y += v.first + Fetch64(s + 40);
		z = Rotate(z + w.first, 33) * k1;
		v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
		w = WeakHashLen32WithSeeds(s + 32, z + w.second,
					   y + Fetch64(s + 16));
		tmp = x;
		x = z;
		z = tmp;	//std::swap(z, x);
		s += 64;
		x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
		y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
		x ^= w.second;
		y += v.first + Fetch64(s + 40);
		z = Rotate(z + w.first, 33) * k1;
		v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
		w = WeakHashLen32WithSeeds(s + 32, z + w.second,
					   y + Fetch64(s + 16));
		tmp = x;
		x = z;
		z = tmp;	//std::swap(z, x);
		s += 64;
		len -= 128;
	} while (LIKELY(len >= 128));

	x += Rotate(v.first + z, 49) * k0;
	z += Rotate(w.first, 37) * k0;
	// If 0 < len < 128, hash up to 4 chunks of 32 bytes each from the end of s.
	for (tail_done = 0; tail_done < len;) {
		tail_done += 32;
		y = Rotate(x + y, 42) * k0 + v.second;
		w.first += Fetch64(s + len - tail_done + 16);
		x = x * k0 + w.first;
		z += w.second + Fetch64(s + len - tail_done);
		w.second += v.first;
		v = WeakHashLen32WithSeeds(s + len - tail_done, v.first + z,
					   v.second);
	}

	// At this point our 56 bytes of state should contain more than
	// enough information for a strong 128-bit hash.  We use two
	// different 56-byte-to-8-byte hashes to get a 16-byte final result.
	x = HashLen16(x, v.first);
	y = HashLen16(y + z, w.first);
	res.first = HashLen16(x + v.second, w.second) + y;
	res.second = HashLen16(x + w.second, y + v.second);
	return res;
}

static city_uint128 CityHash128(const char *s, size_t len)
{
	city_uint128 seed;
	if (len >= 16) {
		seed.first = Fetch64(s) ^ k3;
		seed.second = Fetch64(s + 8);
		return CityHash128WithSeed(s + 16, len - 16, seed);
	} else if (len >= 8) {
		seed.first = Fetch64(s) ^ (len * k0);
		seed.second = Fetch64(s + len - 8) ^ k1;
		return CityHash128WithSeed(NULL, 0, seed);
	} else {
		seed.first = k0;
		seed.second = k1;
		return CityHash128WithSeed(s, len, seed);
	}
}

/*
 * pghashlib API
 */

void hlib_cityhash64(const void *s, size_t len, uint64_t *io)
{
	if (io[0])
		io[0] = CityHash64WithSeed(s, len, io[0]);
	else
		io[0] = CityHash64(s, len);
}

void hlib_cityhash128(const void *data, size_t len, uint64_t *io)
{
	city_uint128 res;
	if (io[0]) {
		res.first = io[0];
		res.second = io[1];
		res = CityHash128WithSeed(data, len, res);
	} else {
		res = CityHash128(data, len);
	}
	io[0] = res.first;
	io[1] = res.second;
}

