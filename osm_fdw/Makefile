
CURRENT_FOLDER = $(shell pwd)
READER_FOLDER = $(CURRENT_FOLDER)/../osm_reader

FC = -g -fpic
F_JSON = $(shell pkg-config --cflags json-c)
F_PG = -I$(shell pg_config --includedir-server)
LDFLAGS = $(shell pkg-config --libs json-c) $(shell pkg-config --libs libprotobuf-c) $(shell pkg-config --libs zlib)

clean:
	rm -rf *.o *.so

osm_fdw.o:
	gcc -c $(FC) $(F_PG) $(F_JSON) -I$(READER_FOLDER) osm_fdw.c

osm_fdw.so: osm_fdw.o
	gcc -shared -dynamic $(LDFLAGS) -o osm_fdw.so osm_fdw.o $(READER_FOLDER)/osm_reader.o $(READER_FOLDER)/type_defs.o $(READER_FOLDER)/json_encode.o $(READER_FOLDER)/zdecode.o $(READER_FOLDER)/fileformat.pb-c.o $(READER_FOLDER)/osmformat.pb-c.o
