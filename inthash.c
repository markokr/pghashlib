/*
 * Integer-to-integer hashes.
 *
 * - http://burtleburtle.net/bob/hash/integer.html
 * - http://www.cris.com/~Ttwang/tech/inthash.htm
 */

#include "pghashlib.h"

uint32_t hlib_int32_jenkins(uint32_t a)
{
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}

uint32_t hlib_wang32(uint32_t key)
{
	key = ~key + (key << 15);
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = (key + (key << 3)) + (key << 11);
	key = key ^ (key >> 16);
	return key;
}

uint32_t hlib_wang32mult(uint32_t key)
{
	key = (key ^ 61) ^ (key >> 16);
	key = key + (key << 3);
	key = key ^ (key >> 4);
	key = key * 0x27d4eb2d;
	key = key ^ (key >> 15);
	return key;
}

uint64_t hlib_int64_wang(uint64_t key)
{
	key = (~key) + (key << 21);
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8);
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4);
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}

uint64_t hlib_int64to32_wang(uint64_t key)
{
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = (key + (key << 2)) + (key << 4);
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	/* map it to signed int8 to allow conversion to int4 */
	return (int64_t)(int32_t)key;
}

