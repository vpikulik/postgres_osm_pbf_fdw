
CURRENT_FOLDER = $(shell pwd)

CFLAGS = `pkg-config --cflags json-c` -I`pg_config --includedir-server` `pkg-config --cflags libprotobuf-c` `pkg-config --cflags zlib`
LDFLAGS = `pkg-config --libs json-c` `pkg-config --libs libprotobuf-c` `pkg-config --libs zlib`
OFLAGS = -g -fpic $(CFLAGS)

EXTENSIONS_FOLDER = `pg_config --sharedir`/extension
LIB_FOLDER = `pg_config --pkglibdir`

all: clean osm_fdw.so reader

clean:
	rm -rf *.pb-c.* *.o *.so reader zpipe out.res

prepare_proto:
	protoc-c --c_out=$(CURRENT_FOLDER) fileformat.proto
	protoc-c --c_out=$(CURRENT_FOLDER) osmformat.proto

fileformat.pb-c.o: prepare_proto
	gcc -c $(OFLAGS) fileformat.pb-c.c

osmformat.pb-c.o: prepare_proto
	gcc -c $(OFLAGS) osmformat.pb-c.c

zdecode.o:
	gcc -c $(OFLAGS) zdecode.c

type_defs.o:
	gcc -c $(OFLAGS) type_defs.c

json_encode.o:
	gcc -c $(OFLAGS) json_encode.c

osm_reader.o:
	gcc -c $(OFLAGS) osm_reader.c

reader.o:
	gcc -c $(OFLAGS) reader.c

reader: fileformat.pb-c.o osmformat.pb-c.o zdecode.o type_defs.o json_encode.o osm_reader.o reader.o
	gcc $(LDFLAGS) -o reader reader.o osm_reader.o type_defs.o json_encode.o zdecode.o fileformat.pb-c.o osmformat.pb-c.o

osm_fdw.o:
	gcc -c $(OFLAGS) osm_fdw.c

osm_fdw.so: fileformat.pb-c.o osmformat.pb-c.o zdecode.o type_defs.o json_encode.o osm_reader.o osm_fdw.o
	gcc -shared -fpic -dynamic $(LDFLAGS) -o osm_fdw.so osm_fdw.o osm_reader.o type_defs.o json_encode.o zdecode.o fileformat.pb-c.o osmformat.pb-c.o

zpipe:
	gcc -g -lz -o zpipe zpipe.c

install: osm_fdw.so
	rm -rf $(EXTENSIONS_FOLDER)/osm_fdw--1.0.sql $(EXTENSIONS_FOLDER)/osm_fdw.control $(LIB_FOLDER)/osm_fdw.so
	ln -s $(CURRENT_FOLDER)/osm_fdw--1.0.sql $(EXTENSIONS_FOLDER)/osm_fdw--1.0.sql
	ln -s $(CURRENT_FOLDER)/osm_fdw.control $(EXTENSIONS_FOLDER)/osm_fdw.control
	ln -s $(CURRENT_FOLDER)/osm_fdw.so $(LIB_FOLDER)/osm_fdw.so