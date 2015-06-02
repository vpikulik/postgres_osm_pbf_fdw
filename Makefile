EXTENSION    = osm_fdw
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

DATA         = $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql))
DOCS         = $(wildcard doc/*.md)
TESTS        = $(wildcard test/sql/*.sql)
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-language=plpgsql
MODULE_big      = $(EXTENSION)


OBJS         =  $(patsubst %.c,%.o,$(wildcard src/osm_reader/*.c))

#
# Uncoment the MODULES line if you are adding C files
# to your extention.
#
#MODULES      = $(patsubst %.c,%,$(wildcard src/*.c))
PG_CONFIG    = pg_config
PG94         = $(shell $(PG_CONFIG) --version | grep -qE " 8\.| 9\.0| 9\.1 | 9\.2| 9\.2| 9\.3" && echo no || echo yes)

#ifeq ($(PG94),yes)
#all: sql/$(EXTENSION)--$(EXTVERSION).sql

# sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
# 	cp $< $@

# DATA = $(wildcard sql/*--*.sql) sql/$(EXTENSION)--$(EXTVERSION).sql
# EXTRA_CLEAN = sql/$(EXTENSION)--$(EXTVERSION).sql


#endif

FC = -g -fpic
F_PROTO = $(shell pkg-config --cflags libprotobuf-c)
F_Z = $(shell pkg-config --cflags zlib)
F_JSON = $(shell pkg-config --cflags json-c)


prepare_proto:
	protoc-c --c_out=./ src/osm_reader/fileformat.proto
	protoc-c --c_out=./ src/osm_reader/osmformat.proto

fileformat.pb-c.o: prepare_proto
	gcc -c $(FC) $(F_PROTO) fileformat.pb-c.c

osmformat.pb-c.o: prepare_proto
	gcc -c $(FC) $(F_PROTO) osmformat.pb-c.c


PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
