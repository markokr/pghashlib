
#include "pghashlib.h"

#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_hash_string);
PG_FUNCTION_INFO_V1(pg_hash_int32);
PG_FUNCTION_INFO_V1(pg_hash_int32from64);
PG_FUNCTION_INFO_V1(pg_hash_int64);

/*
 * Algorithm data
 */

struct StrHashDesc {
	int namelen;
	const char name[12];
	hlib_str_hash_fn hash;
	uint32_t initval;
};

struct Int32HashDesc {
	int namelen;
	const char name[12];
	hlib_int32_hash_fn hash;
};

struct Int64HashDesc {
	int namelen;
	const char name[12];
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
	{ 7, "pgsql84",		hlib_pgsql84, 0 },
	{ 5, "crc32",		hlib_crc32, 0 },
	{ 0 },
};

static const struct Int32HashDesc int32_hash_list[] = {
	{ 4, "wang",		hlib_int32_wang },
	{ 5, "wang2",		hlib_int32_wang2 },
	{ 7, "jenkins",		hlib_int32_jenkins },
	{ 0 },
};

static const struct Int64HashDesc int64_hash_list[] = {
	{ 4, "wang",		hlib_int64_wang },
	{ 8, "wang8to4",	hlib_int64to32_wang },
	{ 0 },
};

/*
 * Lookup functions.
 */

static const struct StrHashDesc *
find_string_hash(const char *name, unsigned nlen)
{
	const struct StrHashDesc *desc;
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
	for (desc = int32_hash_list; desc->name; desc++) {
		if (desc->namelen == nlen && !memcmp(desc->name, name, nlen))
			return desc;
	}
	return NULL;
}

static const struct Int64HashDesc *
find_int64_hash(const char *name, unsigned nlen)
{
	const struct Int64HashDesc *desc;
	for (desc = int64_hash_list; desc->name; desc++) {
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

/*
 * Public functions
 */

Datum
pg_hash_string(PG_FUNCTION_ARGS)
{
	struct varlena *data;
	text *hashname = PG_GETARG_TEXT_PP(1);
	uint32_t initval;
	uint32_t res;
	const struct StrHashDesc *desc;

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
	if (PG_NARGS() == 3)
		initval = PG_GETARG_INT32(2);
	else
		initval = desc->initval;

	/* do hash */
	res = desc->hash(VARDATA_ANY(data), VARSIZE_ANY_EXHDR(data), initval);

	PG_FREE_IF_COPY(data, 0);
	PG_FREE_IF_COPY(hashname, 1);

	PG_RETURN_INT32(res);
}

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

