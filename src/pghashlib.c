
#include "pghashlib.h"

#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_hash_string);
PG_FUNCTION_INFO_V1(pg_hash64_string);
PG_FUNCTION_INFO_V1(pg_hash128_string);
PG_FUNCTION_INFO_V1(pg_hash_int32);
PG_FUNCTION_INFO_V1(pg_hash_int32from64);
PG_FUNCTION_INFO_V1(pg_hash_int64);

/*
 * Algorithm data
 */

#define HASHNAMELEN 12

struct StrHashDesc {
	int namelen;
	const char name[HASHNAMELEN];
	hlib_str_hash_fn hash;
	uint64_t initval;
};

struct Int32HashDesc {
	int namelen;
	const char name[HASHNAMELEN];
	hlib_int32_hash_fn hash;
};

struct Int64HashDesc {
	int namelen;
	const char name[HASHNAMELEN];
	hlib_int64_hash_fn hash;
};

static const struct StrHashDesc string_hash_list[] = {
	{ 7, "lookup2",		hlib_lookup2_hash, 3923095 },
#ifdef WORDS_BIGENDIAN
	{ 7, "lookup3",		hlib_lookup3_hashbig, 0 },
#else
	{ 7, "lookup3",		hlib_lookup3_hashlittle, 0 },
#endif
	{ 9, "lookup3le",	hlib_lookup3_hashlittle, 0 },
	{ 9, "lookup3be",	hlib_lookup3_hashbig,	0 },
	{ 7, "murmur3",		hlib_murmur3, 0 },
	{ 6, "city64",		hlib_cityhash64, 0 },
	{ 7, "city128",		hlib_cityhash128, 0 },
	{ 6, "spooky",		hlib_spookyhash, 0 },
	{ 7, "pgsql84",		hlib_pgsql84, 0 },
	{ 3, "md5",		hlib_md5, 0 },
	{ 5, "crc32",		hlib_crc32, 0 },
	{ 0 },
};

static const struct Int32HashDesc int32_hash_list[] = {
	{ 6, "wang32",		hlib_wang32 },
	{ 10, "wang32mult",	hlib_wang32mult },
	{ 7, "jenkins",		hlib_int32_jenkins },
	{ 0 },
};

static const struct Int64HashDesc int64_hash_list[] = {
	{ 6, "wang64",		hlib_int64_wang },
	{ 10, "wang64to32",	hlib_int64to32_wang },
	{ 0 },
};

/*
 * Lookup functions.
 */

static const struct StrHashDesc *
find_string_hash(const char *name, unsigned nlen)
{
	const struct StrHashDesc *desc;
	char buf[HASHNAMELEN];

	if (nlen >= HASHNAMELEN)
		return NULL;
	memset(buf, 0, sizeof(buf));
	memcpy(buf, name, nlen);

	for (desc = string_hash_list; desc->namelen; desc++) {
		if (desc->namelen != nlen)
			continue;
		if (name[0] != desc->name[0])
			continue;
		if (memcmp(desc->name, name, nlen) == 0)
			return desc;
	}
	return NULL;
}

static const struct Int32HashDesc *
find_int32_hash(const char *name, unsigned nlen)
{
	const struct Int32HashDesc *desc;
	char buf[HASHNAMELEN];

	if (nlen >= HASHNAMELEN)
		return NULL;
	memset(buf, 0, sizeof(buf));
	memcpy(buf, name, nlen);

	for (desc = int32_hash_list; desc->namelen; desc++) {
		if (desc->namelen == nlen && !memcmp(desc->name, name, nlen))
			return desc;
	}
	return NULL;
}

static const struct Int64HashDesc *
find_int64_hash(const char *name, unsigned nlen)
{
	const struct Int64HashDesc *desc;
	char buf[HASHNAMELEN];

	if (nlen >= HASHNAMELEN)
		return NULL;
	memset(buf, 0, sizeof(buf));
	memcpy(buf, name, nlen);

	for (desc = int64_hash_list; desc->namelen; desc++) {
		if (desc->namelen == nlen && !memcmp(desc->name, name, nlen))
			return desc;
	}
	return NULL;
}

/*
 * Utility functions.
 */

static void
err_nohash(text *hashname)
{
	const char *name;
	name = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(hashname)));
	elog(ERROR, "hash '%s' not found", name);
}

static inline uint32_t swap32(uint32_t x)
{
#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)))
	return __builtin_bswap32(x);
#else
	x = ((x << 8) & 0xFF00FF00) | ((x >> 8) & 0x00FF00FF);
	return (x << 16) | (x >> 16);
#endif
}

static inline uint64_t swap64(uint64_t x)
{
	return ((uint64_t)swap32(x) << 32) | swap32(x >> 32);
}

/*
 * Public functions
 */

/* hash_string(bytea, text [, int4]) returns int4 */
Datum
pg_hash_string(PG_FUNCTION_ARGS)
{
	struct varlena *data;
	text *hashname = PG_GETARG_TEXT_PP(1);
	const struct StrHashDesc *desc;
	uint64_t io[MAX_IO_VALUES];

	memset(io, 0, sizeof(io));

	/* request aligned data on weird architectures */
#ifdef HLIB_UNALIGNED_READ_OK
	data = PG_GETARG_VARLENA_PP(0);
#else
	data = PG_GETARG_VARLENA_P(0);
#endif

	/* load hash */
	desc = find_string_hash(VARDATA_ANY(hashname), VARSIZE_ANY_EXHDR(hashname));
	if (desc == NULL)
		err_nohash(hashname);

	/* decide initval */
	if (PG_NARGS() >= 3)
		io[0] = PG_GETARG_INT32(2);
	else
		io[0] = desc->initval;

	/* do hash */
	desc->hash(VARDATA_ANY(data), VARSIZE_ANY_EXHDR(data), io);

	PG_FREE_IF_COPY(data, 0);
	PG_FREE_IF_COPY(hashname, 1);

	PG_RETURN_INT32(io[0]);
}

/* hash64_string(bytea, text [, int8 [, int8]]) returns int8 */
Datum
pg_hash64_string(PG_FUNCTION_ARGS)
{
	struct varlena *data;
	text *hashname = PG_GETARG_TEXT_PP(1);
	uint64_t io[MAX_IO_VALUES];
	const struct StrHashDesc *desc;

	memset(io, 0, sizeof(io));

	/* request aligned data on weird architectures */
#ifdef HLIB_UNALIGNED_READ_OK
	data = PG_GETARG_VARLENA_PP(0);
#else
	data = PG_GETARG_VARLENA_P(0);
#endif

	/* load hash */
	desc = find_string_hash(VARDATA_ANY(hashname), VARSIZE_ANY_EXHDR(hashname));
	if (desc == NULL)
		err_nohash(hashname);

	/* decide initvals */
	if (PG_NARGS() >= 4)
		io[1] = PG_GETARG_INT64(3);
	if (PG_NARGS() >= 3)
		io[0] = PG_GETARG_INT64(2);
	else
		io[0] = desc->initval;

	/* do hash */
	desc->hash(VARDATA_ANY(data), VARSIZE_ANY_EXHDR(data), io);

	PG_FREE_IF_COPY(data, 0);
	PG_FREE_IF_COPY(hashname, 1);

	PG_RETURN_INT64(io[0]);
}

/* hash128_string(bytea, text [, int8 [, int8]]) returns bytea */
Datum
pg_hash128_string(PG_FUNCTION_ARGS)
{
	struct varlena *data;
	text *hashname = PG_GETARG_TEXT_PP(1);
	const struct StrHashDesc *desc;
	uint64_t io[MAX_IO_VALUES];
	bytea *res;

	memset(io, 0, sizeof(io));

	/* request aligned data on weird architectures */
#ifdef HLIB_UNALIGNED_READ_OK
	data = PG_GETARG_VARLENA_PP(0);
#else
	data = PG_GETARG_VARLENA_P(0);
#endif

	/* load hash */
	desc = find_string_hash(VARDATA_ANY(hashname), VARSIZE_ANY_EXHDR(hashname));
	if (desc == NULL)
		err_nohash(hashname);

	/* decide initval */
	if (PG_NARGS() > 2)
		io[0] = PG_GETARG_INT64(2);
	if (PG_NARGS() > 3)
		io[1] = PG_GETARG_INT64(3);

	/* do hash */
	desc->hash(VARDATA_ANY(data), VARSIZE_ANY_EXHDR(data), io);

	PG_FREE_IF_COPY(data, 0);
	PG_FREE_IF_COPY(hashname, 1);

	/* always output little-endian */
#ifdef WORDS_BIGENDIAN
	io[0] = swap64(io[0]);
	io[1] = swap64(io[1]);
#endif

	res = palloc(VARHDRSZ + 16);
	SET_VARSIZE(res, VARHDRSZ + 16);
	memcpy(VARDATA(res), io, 16);

	PG_RETURN_BYTEA_P(res);
}

/*
 * Integer hashing
 */

/* hash_int4(int4, text) returns int4 */
Datum
pg_hash_int32(PG_FUNCTION_ARGS)
{
	int32 data = PG_GETARG_INT32(0);
	text *hashname = PG_GETARG_TEXT_PP(1);
	const struct Int32HashDesc *desc;

	desc = find_int32_hash(VARDATA_ANY(hashname), VARSIZE_ANY_EXHDR(hashname));
	if (desc == NULL)
		err_nohash(hashname);

	PG_FREE_IF_COPY(hashname, 1);

	PG_RETURN_INT32(desc->hash(data));
}

/* hash_int4(int8, text) returns int4 */
Datum
pg_hash_int32from64(PG_FUNCTION_ARGS)
{
	uint64_t data = PG_GETARG_INT64(0);
	text *hashname = PG_GETARG_TEXT_PP(1);
	const struct Int32HashDesc *desc;

	desc = find_int32_hash(VARDATA_ANY(hashname), VARSIZE_ANY_EXHDR(hashname));
	if (desc == NULL)
		err_nohash(hashname);
	PG_FREE_IF_COPY(hashname, 1);

	data = ((data >> 32) ^ data) & 0xFFFFFFFF;
	PG_RETURN_INT32(desc->hash(data));
}

/* hash_int8(int8, text) returns int8 */
Datum
pg_hash_int64(PG_FUNCTION_ARGS)
{
	int64 data = PG_GETARG_INT64(0);
	text *hashname = PG_GETARG_TEXT_PP(1);
	const struct Int64HashDesc *desc;

	desc = find_int64_hash(VARDATA_ANY(hashname), VARSIZE_ANY_EXHDR(hashname));
	if (desc == NULL)
		err_nohash(hashname);
	PG_FREE_IF_COPY(hashname, 1);

	PG_RETURN_INT64(desc->hash(data));
}

