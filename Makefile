
MODULE_big = hashlib
SRCS = pghashlib.c crc32.c lookup2.c lookup3.c inthash.c murmur3.c pgsql84.c
OBJS = $(SRCS:.c=.o)
REGRESS = test_hash
DATA = uninstall_hashlib.sql
DATA_built = hashlib.sql
DOCS = README.hashlib

EXTRA_CLEAN = README.hashlib README.html

PGXS = $(shell pg_config --pgxs)
include $(PGXS)

# docs: rename on install
install: README.hashlib
README.hashlib: README.asciidoc
	cp $< $@

test: install
	make installcheck || { less regression.diffs; exit 1; }

ack:
	cp results/*.out expected/

tags:
	cscope -I . -b -f .cscope.out *.c

%.s: %.c
	$(CC) -S -fverbose-asm -o $@ $< $(CFLAGS) $(CPPFLAGS)

html:
	asciidoc README.asciidoc

