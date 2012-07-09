
MODULE_big = hashlib
SRCS = src/pghashlib.c src/crc32.c src/lookup2.c src/lookup3.c src/inthash.c \
       src/murmur3.c src/pgsql84.c src/city.c src/spooky.c src/md5.c
OBJS = $(SRCS:.c=.o)
DATA = uninstall_hashlib.sql
DATA_built = hashlib.sql

REGRESS = test_hash
REGRESS_OPTS = --inputdir=test

PG_CPPFLAGS = -msse4

DOCS = pghashlib.html

EXTRA_CLEAN = pghashlib.html

PGXS = $(shell pg_config --pgxs)
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

html: pghashlib.html

pghashlib.html: README.rst
	rst2html $< > $@

