
CURRENT_FOLDER = $(shell pwd)

FC = -g -fpic
F_PROTO = $(shell pkg-config --cflags libprotobuf-c)
F_Z = $(shell pkg-config --cflags zlib)
F_JSON = $(shell pkg-config --cflags json-c)


all: clean objects

objects: fileformat.pb-c.o osmformat.pb-c.o zdecode.o type_defs.o json_encode.o osm_reader.o

clean:
	rm -rf *.pb-c.* *.o

prepare_proto:
	protoc-c --c_out=$(CURRENT_FOLDER) fileformat.proto
	protoc-c --c_out=$(CURRENT_FOLDER) osmformat.proto

fileformat.pb-c.o: prepare_proto
	gcc -c $(FC) $(F_PROTO) fileformat.pb-c.c

osmformat.pb-c.o: prepare_proto
	gcc -c $(FC) $(F_PROTO) osmformat.pb-c.c

zdecode.o:
	gcc -c $(FC) $(F_Z) zdecode.c

type_defs.o:
	gcc -c $(FC) type_defs.c

json_encode.o:
	gcc -c $(FC) $(F_JSON) json_encode.c

osm_reader.o:
	gcc -c $(FC) osm_reader.c
