
select hash_string('', 'crc32');
select hash_string('a', 'crc32');
select hash_string('abcdefg', 'crc32');
select hash_string('defg', 'crc32', hash_string('abc', 'crc32'));
select encode(hash128_string('abcdefg', 'crc32'), 'hex');

select hash_string('', 'lookup2');
select hash_string('a', 'lookup2');
select hash_string('abcdefg', 'lookup2');
select encode(hash128_string('abcdefg', 'lookup2'), 'hex');
select hash_string('01234567890', 'lookup2');
select hash_string('012345678901', 'lookup2');
select hash_string('0123456789012', 'lookup2');
select hash_string('01234567890123', 'lookup2');

select hash_string('', 'lookup3');
select hash_string('a', 'lookup3');
select hash_string('abcdefg', 'lookup3');
select hash_string('01234567890', 'lookup2');
select hash_string('012345678901', 'lookup2');
select hash_string('0123456789012', 'lookup2');
select hash_string('01234567890123', 'lookup2');

select hash_string('', 'lookup3be');
select hash_string('a', 'lookup3be');
select hash_string('abcdefg', 'lookup3be');
select encode(hash128_string('abcdefg', 'lookup3be'), 'hex');
select hash_string('01234567890', 'lookup2');
select hash_string('012345678901', 'lookup2');
select hash_string('0123456789012', 'lookup2');
select hash_string('01234567890123', 'lookup2');

select hash_string('', 'murmur3');
select hash_string('a', 'murmur3');
select hash_string('abcdefg', 'murmur3');
select encode(hash128_string('abcdefg', 'murmur3'), 'hex');

select hash_string('', 'pgsql84');
select hash_string('a', 'pgsql84');
select hash_string('abcdefg', 'pgsql84');
select encode(hash128_string('abcdefg', 'pgsql84'), 'hex');
select hash_string('01234567890', 'lookup2');
select hash_string('012345678901', 'lookup2');
select hash_string('0123456789012', 'lookup2');
select hash_string('01234567890123', 'lookup2');

select hash64_string('', 'lookup3le');
select hash64_string('a', 'lookup3le');
select hash64_string('abcdefg', 'lookup3le');
select encode(hash128_string('abcdefg', 'lookup3le'), 'hex');
select hash64_string('01234567890', 'lookup2');
select hash64_string('012345678901', 'lookup2');
select hash64_string('0123456789012', 'lookup2');
select hash64_string('01234567890123', 'lookup2');

select hash64_string('', 'city64');
select hash64_string('a', 'city64');
select hash64_string('abcdefg', 'city64');
select encode(hash128_string('abcdefg', 'city64'), 'hex');
select hash64_string('0123456789abcdef', 'city64');
select hash64_string('0123456789abcdef0', 'city64');
select hash64_string('0123456789abcdef0123456789abcdef', 'city64');
select hash64_string('0123456789abcdef0123456789abcdef0', 'city64');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde', 'city64');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef', 'city64');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0', 'city64');


select hash64_string('', 'city128');
select hash64_string('a', 'city128');
select hash64_string('abcdefg', 'city128');
select encode(hash128_string('abcdefg', 'city128'), 'hex');
select hash64_string('0123456789abcdef', 'city128');
select hash64_string('0123456789abcdef0', 'city128');
select hash64_string('0123456789abcdef0123456789abcdef', 'city128');
select hash64_string('0123456789abcdef0123456789abcdef0', 'city128');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde', 'city128');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef', 'city128');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0', 'city128');

select hash64_string('', 'spooky');
select hash64_string('a', 'spooky');
select hash64_string('abcdefg', 'spooky');
select encode(hash128_string('abcdefg', 'spooky'), 'hex');
select hash64_string('0123456789abcdef', 'spooky');
select hash64_string('0123456789abcdef0', 'spooky');
select hash64_string('0123456789abcdef0123456789abcdef', 'spooky');
select hash64_string('0123456789abcdef0123456789abcdef0', 'spooky');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde', 'spooky');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef', 'spooky');
select hash64_string('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0', 'spooky');

SELECT encode(hash128_string('', 'md5'), 'hex');
-- d41d8cd98f00b204e9800998ecf8427e
SELECT encode(hash128_string('a', 'md5'), 'hex');
-- 0cc175b9c0f1b6a831c399e269772661
SELECT encode(hash128_string('abc', 'md5'), 'hex');
-- 900150983cd24fb0d6963f7d28e17f72
SELECT encode(hash128_string('message digest', 'md5'), 'hex');
-- f96b697d7cb7938d525a2f31aaf161d0
SELECT encode(hash128_string('abcdefghijklmnopqrstuvwxyz', 'md5'), 'hex');
-- c3fcd3d76192e4007dfb496cca67e13b
SELECT encode(hash128_string('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789', 'md5'), 'hex');
-- d174ab98d277d9f5a5611c2c9f419d9f
SELECT encode(hash128_string('12345678901234567890123456789012345678901234567890123456789012345678901234567890', 'md5'), 'hex');
-- 57edf4a22be3c955ac49da2e2107b67a

--
-- integer hashes
--

select hash_int4(0, 'jenkins');
select hash_int4(12345678, 'jenkins');
select hash_int4(1234567890123456789::int8, 'jenkins');

select hash_int4(0, 'wang32');
select hash_int4(12345678, 'wang32');
select hash_int4(1234567890123456789::int8, 'wang32');

select hash_int4(0, 'wang32mult');
select hash_int4(12345678, 'wang32mult');
select hash_int4(1234567890123456789::int8, 'wang32mult');

select hash_int8(0, 'wang64');
select hash_int8(12345678, 'wang64');
select hash_int8(1234567890123456789::int8, 'wang64');

select hash_int8(0, 'wang64to32');
select hash_int8(12345678, 'wang64to32');
select hash_int8(1234567890123456789::int8, 'wang64to32');

-- make sure conversion to int4 works correctly
select count(hash_int8(x, 'wang64to32')::int4) from generate_series(1,1000) x;


select x, hash_int4(x + 6, 'jenkins') from generate_series(1, 10) x order by hash_int4(x + 6, 'jenkins');


