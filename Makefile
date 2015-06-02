EXTENSION    = osm_fdw
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

DATA         = $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql))
DOCS         = $(wildcard doc/*.md)
TESTS        = $(wildcard test/sql/*.sql)
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-language=plpgsql
MODULE_big      = $(EXTENSION)

READER_OBJ = fileformat.pb-c.o osmformat.pb-c.o zdecode.o type_defs.o json_encode.o osm_reader.o
OBJS = $(READER_OBJ)

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

CURRENT_FOLDER = $(shell pwd)
READER_FOLDER = $(CURRENT_FOLDER)/src/osm_reader

FC = -g -fpic
F_PROTO = $(shell pkg-config --cflags libprotobuf-c)
F_Z = $(shell pkg-config --cflags zlib)
F_JSON = $(shell pkg-config --cflags json-c)


$(READER_FOLDER)/fileformat.pb-c.c:
	make -C $(READER_FOLDER) fileformat.pb-c.c

fileformat.pb-c.o: $(READER_FOLDER)/fileformat.pb-c.c
	gcc -c $(FC) $(F_PROTO) $(READER_FOLDER)/fileformat.pb-c.c

$(READER_FOLDER)/osmformat.pb-c.c:
	make -C $(READER_FOLDER) osmformat.pb-c.c

osmformat.pb-c.o: $(READER_FOLDER)/osmformat.pb-c.c
	gcc -c $(FC) $(F_PROTO) $(READER_FOLDER)/osmformat.pb-c.c

zdecode.o:
	gcc -c $(FC) $(F_Z) $(READER_FOLDER)/zdecode.c

type_defs.o:
	gcc -c $(FC) $(READER_FOLDER)/type_defs.c

json_encode.o:
	gcc -c $(FC) $(F_JSON) $(READER_FOLDER)/json_encode.c

osm_reader.o:
	gcc -c $(FC) $(READER_FOLDER)/osm_reader.c


PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
