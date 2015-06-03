EXTENSION    = osm_fdw
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

DATA         = $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql))
DOCS         = $(wildcard doc/*.md)
TESTS        = $(wildcard test/sql/*.sql)
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-language=plpgsql
MODULE_big      = $(EXTENSION)

#
# Uncoment the MODULES line if you are adding C files
# to your extention.
#
#MODULES      = $(patsubst %.c,%,$(wildcard src/*.c))
PG_CONFIG    = pg_config
PG93         = $(shell $(PG_CONFIG) --version | grep -qE " 8\.| 9\.0| 9\.1 | 9\.2| 9\.2| 9\.4" && echo no || echo yes)
PG94         = $(shell $(PG_CONFIG) --version | grep -qE " 8\.| 9\.0| 9\.1 | 9\.2| 9\.2| 9\.3" && echo no || echo yes)

#ifeq ($(PG94),yes)
#all: sql/$(EXTENSION)--$(EXTVERSION).sql

# sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
# 	cp $< $@

# DATA = $(wildcard sql/*--*.sql) sql/$(EXTENSION)--$(EXTVERSION).sql
# EXTRA_CLEAN = sql/$(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN = 

#endif

CURRENT_FOLDER = $(shell pwd)
READER_FOLDER = $(CURRENT_FOLDER)/src/osm_reader
FDW_FOLDER = $(CURRENT_FOLDER)/src/osm_fdw

FC = -g -fpic
F_PROTO = $(shell pkg-config --cflags libprotobuf-c)
F_Z = $(shell pkg-config --cflags zlib)
F_JSON = $(shell pkg-config --cflags json-c)
F_PG = -I$(shell $(PG_CONFIG) --includedir-server)

OBJS = osm_reader.o
OBJS += type_defs.o
OBJS += zdecode.o
OBJS += fileformat.pb-c.o
OBJS += osmformat.pb-c.o
OBJS += osm_fdw.o

ifeq ($(PG93), yes)
ENV_VARS += -DUSE_LIBJSONC
objects += json_encode.o
else
ENV_VARS += -DUSE_JSONB
objects += jsonb_encode.o
endif

EXTRA_CLEAN += json_encode.o jsonb_encode.o


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
	gcc -c $(FC) $(F_PG) $(ENV_VARS) -I$(READER_FOLDER) jsonb_encode.c

osm_reader.o:
	gcc -c $(FC) $(READER_FOLDER)/osm_reader.c

osm_fdw.o:
	gcc -c $(FC) $(F_PG) $(F_JSON) $(ENV_VARS) -I$(READER_FOLDER) $(FDW_FOLDER)/osm_fdw.c

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
