EXTENSION    = osm_fdw
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

DATA         = $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql))
DOCS         = $(wildcard doc/*.md)
# TESTS        = $(wildcard test/sql/*.sql)
# REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
# REGRESS_OPTS = --inputdir=test --load-language=plpgsql
MODULE_big      = $(EXTENSION)

#
# Uncoment the MODULES line if you are adding C files
# to your extention.
#
#MODULES      = $(patsubst %.c,%,$(wildcard src/*.c))
PG_CONFIG    = pg_config

TEST_DATABASE = osm_test_db
TEST_PORT = 5432


DATA = $(wildcard sql/*--*.sql) sql/$(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN = sql/$(EXTENSION)--$(EXTVERSION).sql


CURRENT_FOLDER = $(shell pwd)
READER_FOLDER = $(CURRENT_FOLDER)/src/osm_reader
FDW_FOLDER = $(CURRENT_FOLDER)/src/osm_fdw
CONVERTER_FOLDER = $(CURRENT_FOLDER)/src/osm_convert

FC = -g -fpic
#F_PROTO = $(shell pkg-config --cflags libprotobuf-c)
F_Z = $(shell pkg-config --cflags zlib)
F_JSON = $(shell pkg-config --cflags json-c)
F_PG = -I$(shell $(PG_CONFIG) --includedir-server)

F_LD = $(shell pkg-config --libs json-c)
#F_LD += $(shell pkg-config --libs libprotobuf-c)
F_LD += -lprotobuf-c
F_LD += $(shell pkg-config --libs zlib)
SHLIB_LINK = $(F_LD)

OBJS = osm_reader.o
OBJS += type_defs.o
OBJS += zdecode.o
OBJS += fileformat.pb-c.o
OBJS += osmformat.pb-c.o
OBJS += jsonb_encode.o
OBJS += osm_fdw.o

UTILS_FILE = sql/utils.sql

EXTRA_CLEAN += $(READER_FOLDER)/fileformat.pb-c.c $(READER_FOLDER)/fileformat.pb-c.h
EXTRA_CLEAN += $(READER_FOLDER)/osmformat.pb-c.c $(READER_FOLDER)/osmformat.pb-c.h
EXTRA_CLEAN += json_encode.o jsonb_encode.o
EXTRA_CLEAN += osm_to_json.o osm_to_json
EXTRA_CLEAN += osm_count.o osm_count
EXTRA_CLEAN += /tmp/monaco.osm.pbf

build_all: sql/$(EXTENSION)--$(EXTVERSION).sql all

test: /tmp/monaco.osm.pbf
	pg_prove -p $(TEST_PORT) -d $(TEST_DATABASE) tests/smoke.sql

/tmp/monaco.osm.pbf:
	rm -rf /tmp/monaco.osm.pbf
	cp data/monaco.osm.pbf /tmp/

sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
	cat $< $(UTILS_FILE) > $@

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

jsonb_encode.o:
	gcc -c $(FC) $(F_PG) $(ENV_VARS) -I$(READER_FOLDER) $(FDW_FOLDER)/jsonb_encode.c

osm_reader.o: $(READER_FOLDER)/fileformat.pb-c.c $(READER_FOLDER)/osmformat.pb-c.c
	gcc -c $(FC) $(READER_FOLDER)/osm_reader.c

osm_fdw.o:
	gcc -c $(FC) $(F_PG) $(F_JSON) $(ENV_VARS) -I$(READER_FOLDER) $(FDW_FOLDER)/osm_fdw.c

CONVERTER_OBJS = osm_reader.o
CONVERTER_OBJS += type_defs.o
CONVERTER_OBJS += zdecode.o
CONVERTER_OBJS += fileformat.pb-c.o
CONVERTER_OBJS += osmformat.pb-c.o
CONVERTER_OBJS += json_encode.o

osm_to_json.o:
	gcc -c $(FC) $(F_JSON) -I$(READER_FOLDER) $(CONVERTER_FOLDER)/osm_to_json.c

osm_to_json: osm_to_json.o $(CONVERTER_OBJS)
	gcc $(FC) $(F_LD) -o osm_to_json -I$(READER_FOLDER) osm_to_json.o $(CONVERTER_OBJS)

osm_count.o:
	gcc -c $(FC) $(F_JSON) -I$(READER_FOLDER) $(CONVERTER_FOLDER)/osm_count.c

osm_count: osm_count.o $(CONVERTER_OBJS)
	gcc $(FC) $(F_LD) -o osm_count -I$(READER_FOLDER) osm_count.o $(CONVERTER_OBJS)


PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
