
CURRENT_FOLDER = $(shell pwd)
READER_FOLDER = $(CURRENT_FOLDER)/../osm_reader

FC = -g -fpic
F_JSON = $(shell pkg-config --cflags json-c)
F_PG = -I$(shell pg_config --includedir-server)
LDFLAGS = $(shell pkg-config --libs json-c) $(shell pkg-config --libs libprotobuf-c) $(shell pkg-config --libs zlib)
ENV_VARS =

objects = osm_fdw.o $(READER_FOLDER)/osm_reader.o
objects += $(READER_FOLDER)/type_defs.o
objects += $(READER_FOLDER)/zdecode.o
objects += $(READER_FOLDER)/fileformat.pb-c.o
objects += $(READER_FOLDER)/osmformat.pb-c.o

dependencies = osm_fdw.o

ifeq ($(VERSION), 9.3)
ENV_VARS += -DUSE_LIBJSONC
objects += $(READER_FOLDER)/json_encode.o
else
ENV_VARS += -DUSE_JSONB
objects += jsonb_encode.o
dependencies += jsonb_encode.o
endif


clean:
	rm -rf *.o *.so

jsonb_encode.o:
	gcc -c $(FC) $(F_PG) $(ENV_VARS) -I$(READER_FOLDER) jsonb_encode.c

osm_fdw.o:
	gcc -c $(FC) $(F_PG) $(F_JSON) $(ENV_VARS) -I$(READER_FOLDER) osm_fdw.c

osm_fdw.so: $(dependencies)
	gcc -shared -dynamic $(LDFLAGS) -o osm_fdw.so $(objects)
