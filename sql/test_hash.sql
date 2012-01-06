
\set ECHO none
\i hashlib.sql
\set ECHO all

select hash_string('', 'crc32');
select hash_string('a', 'crc32');
select hash_string('abcdefg', 'crc32');
select hash_string('defg', 'crc32', hash_string('abc', 'crc32'));

select hash_string('', 'lookup2');
select hash_string('a', 'lookup2');
select hash_string('abcdefg', 'lookup2');

select hash_string('', 'lookup3');
select hash_string('a', 'lookup3');
select hash_string('abcdefg', 'lookup3');

select hash_string('', 'lookup3be');
select hash_string('a', 'lookup3be');
select hash_string('abcdefg', 'lookup3be');

select hash_string('', 'murmur3');
select hash_string('a', 'murmur3');
select hash_string('abcdefg', 'murmur3');

select hash_int4(0, 'jenkins');
select hash_int4(12345678, 'jenkins');
select hash_int4(1234567890123456789::int8, 'jenkins');

select hash_int4(0, 'wang');
select hash_int4(12345678, 'wang');
select hash_int4(1234567890123456789::int8, 'wang');

select hash_int4(0, 'wang2');
select hash_int4(12345678, 'wang2');
select hash_int4(1234567890123456789::int8, 'wang2');

select hash_int8(0, 'wang');
select hash_int8(12345678, 'wang');
select hash_int8(1234567890123456789::int8, 'wang');

select hash_int8(0, 'wang8to4');
select hash_int8(12345678, 'wang8to4');
select hash_int8(1234567890123456789::int8, 'wang8to4');

-- make sure conversion to int4 works correctly
select count(hash_int8(x, 'wang8to4')::int4) from generate_series(1,1000) x;

