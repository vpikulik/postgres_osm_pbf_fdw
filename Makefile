
.DEFAULT_GOAL := all

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

PG_CPPFLAGS = -Isrc/osm_reader
SHLIB_LINK = -lprotobuf-c $(shell pkg-config --libs json-c) $(shell pkg-config --libs zlib)

EXTRA_CLEAN = sql/$(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN += src/osm_reader/fileformat.pb-c.c src/osm_reader/fileformat.pb-c.h
EXTRA_CLEAN += src/osm_reader/osmformat.pb-c.c src/osm_reader/osmformat.pb-c.h
EXTRA_CLEAN += /tmp/monaco.osm.pbf


sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
	cat $< sql/utils.sql > $@

src/osm_reader/fileformat.pb-c.c:
	make -C src/osm_reader fileformat.pb-c.c

src/osm_reader/osmformat.pb-c.c:
	make -C src/osm_reader osmformat.pb-c.c

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
