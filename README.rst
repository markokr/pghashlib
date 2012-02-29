pghashlib
=========

The goal of pghashlib is to provide stable hashes for PostgeSQL,
"stable" in the sense that their implementation does not change,
they are independent from PostgeSQL version.

Some string hashes may be dependent on CPU architecture,
so that they return different return on big-endian
architecture from little-endian architecture.
It you want to be architectures-independent, then
use algorithms that don't depend on endianess.


Installation
------------

You need PostgreSQL developent environment.  Then simply::

  $ make
  $ make install
  $ psql -d ... -f hashlib.sql


Functions
---------

hash_string
~~~~~~~~~~~

::

  hash_string(data text,  algo text [, initval int4]) returns int4
  hash_string(data bytea, algo text [, initval int4]) returns int4

This hashes the data with specified algorithm, returns 32-bit result.


hash64_string
~~~~~~~~~~~~~

::

  hash64_string(data text, algo text, [, initval int4]) returns int8
  hash64_string(data byte, algo text, [, initval int4]) returns int8

Uses same algorithms as `hash_string()` but returns 64-bit result.

hash128_string
~~~~~~~~~~~~~~

::

  hash64_string(data text, algo text, [, initval int4]) returns int8
  hash64_string(data byte, algo text, [, initval int4]) returns int8

Uses same algorithms as `hash_string()` but returns 128-bit result.


hash_int4
~~~~~~~~~

::

  hash_int4(val int4) returns int4

Reversible integer hash.

Supported aldorithms:

* `jenkins` - Jenkins integer hash with 6 shifts
* `wang`    - Thomas Wang's hash32shift()
* `wang2`   - Thomas Wang's hash32shiftmult()


hash_int8
~~~~~~~~~

::

  hash_int8(val int8) returns int8

Reversible integer hash.

Supported algorithms:

* `wang`     - Thomas Wang's hash64shift()
* `wang8to4` - Thomas Wang's hash6432shift(),
  creates 32bit hash from 64bit integer.  The result can be
  safely cast to int4.



String hashing algorithms
-------------------------

List of currently provided algorithms.

==============  =========  ======  =======  ==============================
 Algorithm      CPU-indep   Bits   Partial  Description
==============  =========  ======  =======  ==============================
 city64          no          64      no      CityHash64
 city128         no         128      no      CityHash128
 crc32           yes         32     yes      CRC32
 lookup2         no          64      no      Jenkins lookup2
 lookup3be       yes         64      no      Jenkins lookup3 big-endian
 lookup3le       yes         64      no      Jenkins lookup3 little-endian
 lookup3         no          64      no      Jenkins lookup3 CPU-native
 murmur3         no          32      no      MurmurHash v3, 32-bit variant
 md5             yes        128      no      MD5
 pgsql84         no          64      no      Hacked lookup3 in Postgres 8.4+
 spooky          no         128      no      SpookyHash
==============  =========  ======  =======  ==============================

CPU-independence
  Whether hash output is independent of CPU endianess.  If not, then
  hash result is different on little-endian machines (x86)
  and big-endian machines (sparc).

Bits
  Maximum number of output bits that hash can output.
  If longer result is requested, result will be
  zero-padded.

Partial hashing
  Whether long string can be hashed in smaller parts, by giving last
  value as initval to next hash call.



Links
-----

* `Lookup2/3 hashes`__ by Bob Jenkins.

.. __: http://burtleburtle.net/bob/hash/doobs.html

* `MurmurHash`__ by Austin Appleby.

.. __: http://code.google.com/p/smhasher/

* `Integer hashes`__ by Bob Jenkins.

.. __: http://burtleburtle.net/bob/hash/integer.html

* `Integer hashes`__ by Thomas Wang.

.. __: http://www.cris.com/~Ttwang/tech/inthash.htm

* Google's `CityHash`__.  64/128/256-bit output.

.. __: http://code.google.com/p/cityhash/

* `SpookyHash`__ by Bob Jenkins.  128-bit output.

.. __: http://www.burtleburtle.net/bob/hash/spooky.html

