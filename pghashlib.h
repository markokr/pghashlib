#ifndef _PGHASHLIB_H_
#define _PGHASHLIB_H_

#include <postgres.h>
#include <fmgr.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#if !defined(PG_VERSION_NUM) || (PG_VERSION_NUM < 80300)
#error "PostgreSQL 8.3+ required"
#endif

/*
 * Does architecture support unaligned reads?
 *
 * If not, we copy the data to aligned location, because not all
 * hash implementation are fully portable.
 */
#if defined(__i386__) || defined(__x86_64__)
#define HLIB_UNALIGNED_READ_OK
#endif

/* hash function signatures */
typedef uint32_t (*hlib_str_hash_fn)(const void *data, uint32_t len, uint32_t initval);
typedef uint32_t (*hlib_int32_hash_fn)(uint32_t data);
typedef uint64_t (*hlib_int64_hash_fn)(uint64_t data);

/* string hashes */
uint32_t hlib_crc32(const void *data, uint32_t len, uint32_t initval);
uint32_t hlib_lookup2_hash(const void *data, uint32_t len, uint32_t initval);
uint32_t hlib_lookup3_hashlittle(const void *data, uint32_t len, uint32_t initval);
uint32_t hlib_lookup3_hashbig(const void *data, uint32_t len, uint32_t initval);
uint32_t hlib_pgsql84(const void *data, uint32_t keylen, uint32_t seed);
uint32_t hlib_murmur3(const void *data, uint32_t len, uint32_t seed);

/* integer hashes */
uint32_t hlib_int32_jenkins(uint32_t data);
uint32_t hlib_int32_wang(uint32_t data);
uint32_t hlib_int32_wang2(uint32_t data);
uint64_t hlib_int64_wang(uint64_t data);
uint64_t hlib_int64to32_wang(uint64_t data);

/* SQL function */
Datum pg_hash_string(PG_FUNCTION_ARGS);
Datum pg_hash_int32(PG_FUNCTION_ARGS);
Datum pg_hash_int32from64(PG_FUNCTION_ARGS);
Datum pg_hash_int64(PG_FUNCTION_ARGS);

#endif

