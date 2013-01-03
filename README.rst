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
  $ psql -d ... -c "create extension hashlib"


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

  hash64_string(data text, algo text, [, iv1 int8 [, iv2 int8]]) returns int8
  hash64_string(data byte, algo text, [, iv1 int8 [, iv2 int8]]) returns int8

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

Hash 32-bit integer.


hash_int8
~~~~~~~~~

::

  hash_int8(val int8) returns int8

Hash 64-bit integer.



String hashing algorithms
-------------------------

List of currently provided algorithms.

==============  =========  ======  =======  =======  ==============================
 Algorithm      CPU-indep   Bits   IV bits  Partial   Description
==============  =========  ======  =======  =======  ==============================
 city64          no          64       64       no     CityHash64
 city128         no         128      128       no     CityHash128
 crc32           yes         32       32      yes     CRC32
 lookup2         no          64       32       no      Jenkins lookup2
 lookup3be       yes         64       32       no      Jenkins lookup3 big-endian
 lookup3le       yes         64       32       no      Jenkins lookup3 little-endian
 lookup3         no          64       32       no      Jenkins lookup3 CPU-native
 murmur3         no          32       32       no      MurmurHash v3, 32-bit variant
 md5             yes        128      128       no      MD5
 pgsql84         no          64        0       no      Hacked lookup3 in Postgres 8.4+
 siphash24       yes         64      128       no      SipHash-2-4
 spooky          no         128      128       no      SpookyHash
==============  =========  ======  =======  =======  ==============================

CPU-independence
  Whether hash output is independent of CPU endianess.  If not, then
  hash result is different on little-endian machines (x86)
  and big-endian machines (sparc).

Bits
  Maximum number of output bits that hash can output.
  If longer result is requested, result will be
  zero-padded.

IV bits
  Maximum number of input bits for "initial value".

Partial hashing
  Whether long string can be hashed in smaller parts, by giving last
  value as initval to next hash call.

Integer hashing algorithms
--------------------------

==============  ======  ================================  ============================
 Algorithm       Bits    Description                       Notes
==============  ======  ================================  ============================
 wang32           32     Thomas Wang hash32shift
 wang32mult       32     Thomas Wang hash32shiftmult
 jenkins          32     Bob Jenkins hash with 6 shifts
 wang64           64     Thomas Wang hash64shift
 wang64to32       64     Thomas Wang hash6432shift         Result can be cast to int4
==============  ======  ================================  ============================

All algorithms here have the property that they are "reversible",
that means there is 1:1 mapping between input and output.

This propery is useful for creating well-defined "random" sort order over
unique integer id's.  Or picking up random row from table
with unique id's.


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

* `SipHash-2-4`__ by Jean-Philippe Aumasson and Daniel J. Bernstein.

.. __: https://131002.net/siphash/
