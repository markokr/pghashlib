# configurable variables
PG_CONFIG = pg_config
RST2HTML = rst2html

# module description
MODULE_big = hashlib
SRCS = src/pghashlib.c src/crc32.c src/lookup2.c src/lookup3.c src/inthash.c \
       src/murmur3.c src/pgsql84.c src/city.c src/spooky.c src/md5.c
OBJS = $(SRCS:.c=.o)
EXTENSION = $(MODULE_big)

DOCS = hashlib.html
EXTRA_CLEAN = hashlib.html

REGRESS_OPTS = --inputdir=test

# different vars for extension and plain module

Regress_noext = test_init_noext test_hash
Regress_ext   = test_init_ext   test_hash

Data_noext = sql/hashlib.sql sql/uninstall_hashlib.sql
Data_ext = sql/hashlib--1.0.sql sql/hashlib--unpackaged--1.0.sql


# Work around PGXS deficiencies - switch variables based on
# whether extensions are supported.
PgMajor = $(if $(MAJORVERSION),$(MAJORVERSION),8.3)
PgHaveExt = $(if $(filter 8.% 9.0,$(PgMajor)),noext,ext)
DATA = $(Data_$(PgHaveExt))
REGRESS = $(Regress_$(PgHaveExt))


# launch PGXS
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

install: $(DOCS)

test: install
	make installcheck || { filterdiff --format=unified regression.diffs | less; exit 1; }

ack:
	cp results/*.out test/expected/

tags:
	cscope -I . -b -f .cscope.out src/*.c

%.s: %.c
	$(CC) -S -fverbose-asm -o - $< $(CFLAGS) $(CPPFLAGS) | cleanasm > $@

html: hashlib.html

hashlib.html: README.rst
	$(RST2HTML) $< > $@

deb:
	rm -f debian/control
	make -f debian/rules debian/control
	debuild -uc -us -b

debclean: clean
	$(MAKE) -f debian/rules realclean
	rm -f lib* hashlib.so* hashlib.a
	rm -rf .deps

