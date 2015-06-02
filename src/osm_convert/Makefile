
CURRENT_FOLDER = $(shell pwd)
READER_FOLDER = $(CURRENT_FOLDER)/../osm_reader

FC = -g -fpic
F_JSON = $(shell pkg-config --cflags json-c)
LDFLAGS = $(shell pkg-config --libs json-c) $(shell pkg-config --libs libprotobuf-c) $(shell pkg-config --libs zlib)

clean:
	rm -rf *.o osm_to_json

osm_to_json.o:
	gcc -c $(FC) $(F_JSON) -I$(READER_FOLDER) osm_to_json.c

osm_to_json: osm_to_json.o
	gcc $(LDFLAGS) -o osm_to_json osm_to_json.o $(READER_FOLDER)/osm_reader.o $(READER_FOLDER)/type_defs.o $(READER_FOLDER)/json_encode.o $(READER_FOLDER)/zdecode.o $(READER_FOLDER)/fileformat.pb-c.o $(READER_FOLDER)/osmformat.pb-c.o
