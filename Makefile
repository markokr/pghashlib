
MODULE_big = hashlib
SRCS = pghashlib.c crc32.c lookup2.c lookup3.c inthash.c murmur3.c \
       pgsql84.c city.c spooky.c md5.c
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
	cscope -I . -b -f .cscope.out *.c

%.s: %.c
	$(CC) -S -fverbose-asm -o - $< $(CFLAGS) $(CPPFLAGS) | cleanasm > $@

html: pghashlib.html

pghashlib.html: README.rst
	rst2html $< > $@

