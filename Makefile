
CURRENT_FOLDER = $(shell pwd)

POSTGRES_FLAGS = -I `pg_config --includedir-server`
FLAGS = -g -fpic -lz -lprotobuf-c

EXTENSIONS_FOLDER = /usr/share/postgresql/9.3/extension
LIB_FOLDER = /usr/lib/postgresql/9.3/lib

all: clean prepare_proto reader osm_fdw.so

clean:
	rm -rf *.pb-c.* *.o *.so reader zpipe out.res

prepare_proto:
	protoc-c --c_out=$(CURRENT_FOLDER) fileformat.proto
	protoc-c --c_out=$(CURRENT_FOLDER) osmformat.proto

fileformat.pb-c.o:
	gcc -c $(FLAGS) fileformat.pb-c.c

osmformat.pb-c.o:
	gcc -c $(FLAGS) osmformat.pb-c.c

zdecode.o:
	gcc -c $(FLAGS) zdecode.c

type_defs.o:
	gcc -c $(FLAGS) type_defs.c

json_encode.o:
	gcc -c $(FLAGS) json_encode.c

osm_reader.o:
	gcc -c $(FLAGS) osm_reader.c

reader.o:
	gcc -c $(FLAGS) reader.c

reader: zdecode.o type_defs.o json_encode.o reader.o osm_reader.o fileformat.pb-c.o osmformat.pb-c.o
	gcc -g -lprotobuf-c -lz -ljansson -o reader reader.o osm_reader.o type_defs.o json_encode.o zdecode.o fileformat.pb-c.o osmformat.pb-c.o

osm_fdw.o:
	gcc -c $(FLAGS) $(POSTGRES_FLAGS) osm_fdw.c

osm_fdw.so: osm_fdw.o zdecode.o type_defs.o json_encode.o osm_reader.o fileformat.pb-c.o osmformat.pb-c.o
	gcc -shared -fpic -dynamic -ljansson -lz -lprotobuf-c -o osm_fdw.so osm_fdw.o zdecode.o type_defs.o json_encode.o osm_reader.o fileformat.pb-c.o osmformat.pb-c.o

zpipe:
	gcc -g -lz -o zpipe zpipe.c

install: osm_fdw.so
	rm -rf $(EXTENSIONS_FOLDER)/osm_fdw--1.0.sql $(EXTENSIONS_FOLDER)/osm_fdw.control $(LIB_FOLDER)/osm_fdw.so
	ln -s $(CURRENT_FOLDER)/osm_fdw--1.0.sql $(EXTENSIONS_FOLDER)/osm_fdw--1.0.sql
	ln -s $(CURRENT_FOLDER)/osm_fdw.control $(EXTENSIONS_FOLDER)/osm_fdw.control
	ln -s $(CURRENT_FOLDER)/osm_fdw.so $(LIB_FOLDER)/osm_fdw.so