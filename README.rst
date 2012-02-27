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

This hashes the data with specified algorithm.


hash64_string
~~~~~~~~~~~~~

::

  hash64_string(data text, algo text, [, initval int4]) returns int8
  hash64_string(data byte, algo text, [, initval int4]) returns int8

Uses same algorithms as `hash_string()` but returns 64-bit result.


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

..
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
 Algorithm      CPU-indep  64-bit  Partial  Description
==============  =========  ======  =======  ==============================
 lookup2         no         yes     no       Jenkins lookup2
 lookup3         no         yes     no       Jenkins lookup3 CPU-native
 lookup3le       yes        yes     no       Jenkins lookup3 little-endian
 lookup3be       yes        yes     no       Jenkins lookup3 big-endian
 pgsql84         no         yes     no       Hacked lookup3 in Postgres 8.4+
 murmur3         no         no      no       MurmurHash v3, 32-bit variant
 crc32           yes        no      yes      CRC32
==============  =========  ======  =======  ==============================

CPU-independence
  Whether hash output is independent of CPU endianess.  If not, then
  hash result is different on little-endian machines (x86)
  and big-endian machines (sparc).

64-bit output
  Whether hash can output 64-bit result.  If not, then `hash64_string()`
  simply returns 32-bit value.

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

