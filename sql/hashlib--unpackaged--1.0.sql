
ALTER EXTENSION hashlib ADD FUNCTION hash_string(text, text);
ALTER EXTENSION hashlib ADD FUNCTION hash_string(bytea, text);
ALTER EXTENSION hashlib ADD FUNCTION hash_string(text, text, int4);
ALTER EXTENSION hashlib ADD FUNCTION hash_string(bytea, text, int4);
ALTER EXTENSION hashlib ADD FUNCTION hash64_string(text, text);
ALTER EXTENSION hashlib ADD FUNCTION hash64_string(bytea, text);
ALTER EXTENSION hashlib ADD FUNCTION hash64_string(text, text, int8);
ALTER EXTENSION hashlib ADD FUNCTION hash64_string(bytea, text, int8);
ALTER EXTENSION hashlib ADD FUNCTION hash128_string(text, text);
ALTER EXTENSION hashlib ADD FUNCTION hash128_string(bytea, text);
ALTER EXTENSION hashlib ADD FUNCTION hash_int4(int4, text);
ALTER EXTENSION hashlib ADD FUNCTION hash_int4(int8, text);
ALTER EXTENSION hashlib ADD FUNCTION hash_int8(int8, text);

