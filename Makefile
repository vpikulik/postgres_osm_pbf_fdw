
.DEFAULT_GOAL := all
SHELL = /bin/bash
PG_CONFIG = pg_config

MODULE_big = osm_fdw

OBJS = src/osm_reader/fileformat.pb-c.o
OBJS += src/osm_reader/osmformat.pb-c.o
OBJS += src/osm_reader/osm_reader.o
OBJS += src/osm_reader/type_defs.o
OBJS += src/osm_reader/zdecode.o
OBJS += src/osm_fdw/jsonb_encode.o
OBJS += src/osm_fdw/osm_fdw.o

REGRESS = osm_fdw

EXTENSION = osm_fdw
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

DATA_built = sql/$(EXTENSION)--$(EXTVERSION).sql

ifeq ($(shell $(PG_CONFIG) --version | grep -q "PostgreSQL 10" && echo yes), yes)
PG_VERSION = 10
endif
ifeq ($(shell $(PG_CONFIG) --version | grep -q "PostgreSQL 11" && echo yes), yes)
PG_VERSION = 11
endif
ifeq ($(shell $(PG_CONFIG) --version | grep -q "PostgreSQL 12" && echo yes), yes)
PG_VERSION = 12
endif
ifeq ($(shell $(PG_CONFIG) --version | grep -q "PostgreSQL 13" && echo yes), yes)
PG_VERSION = 13
endif
ifeq ($(shell $(PG_CONFIG) --version | grep -q "PostgreSQL 14" && echo yes), yes)
PG_VERSION = 14
endif

PG_CPPFLAGS = -Isrc/osm_reader -DPGV=$(PG_VERSION)
SHLIB_LINK = -lprotobuf-c $(shell pkg-config --libs json-c) $(shell pkg-config --libs zlib)

EXTRA_CLEAN = sql/$(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN += src/osm_reader/fileformat.pb-c.c src/osm_reader/fileformat.pb-c.h
EXTRA_CLEAN += src/osm_reader/osmformat.pb-c.c src/osm_reader/osmformat.pb-c.h
EXTRA_CLEAN += /tmp/monaco.osm.pbf

TEST_DATABASE = osm_test_db
TEST_PORT = 5432
TEST_USER = tst


sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
	cat $< sql/utils.sql > $@

src/osm_reader/fileformat.pb-c.c:
	make -C src/osm_reader fileformat.pb-c.c

src/osm_reader/osmformat.pb-c.c:
	make -C src/osm_reader osmformat.pb-c.c

test: /tmp/monaco.osm.pbf
	pg_prove -p $(TEST_PORT) -U $(TEST_USER) -d $(TEST_DATABASE) tests/smoke.sql

/tmp/monaco.osm.pbf:
	rm -rf /tmp/monaco.osm.pbf
	cp data/monaco.osm.pbf /tmp/

ver:
	echo $(PG_VERSION)

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
