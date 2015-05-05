
CURRENT_FOLDER = $(shell pwd)

CFLAGS = $(pkg-config --cflags json-c) -I$(pkg_config --includedir-server) $(pkg-config --cflags libprotobuf-c) $(pkg-config --cflags zlib)
LDFLAGS = $(pkg-config --libs json-c) $(pkg-config --libs libprotobuf-c) $(pkg-config --libs zlib)
OFLAGS = -g -fpic $(CFLAGS)

EXTENSIONS_FOLDER = $(pg_config --sharedir)/extension
LIB_FOLDER = $(pg_config --pkglibdir)

all: clean osm_fdw.so osm_to_json

clean:
	rm -rf osm_to_json osm_fdw.so osm_fdw--1.0.sql osm_fdw.control
	make -C ./osm_reader clean
	make -C ./osm_convert clean

osm_to_json:
	make -C ./osm_reader objects
	make -C ./osm_convert osm_to_json
	mv ./osm_convert/osm_to_json ./osm_to_json

osm_fdw.so:
	make -C ./osm_reader objects
	make -C ./osm_fdw osm_fdw.so
	mv ./osm_fdw/osm_fdw.so ./osm_fdw.so
	ln -s ./osm_fdw/osm_fdw--1.0.sql
	ln -s ./osm_fdw/osm_fdw.control

install: osm_fdw.so
	rm -rf $(EXTENSIONS_FOLDER)/osm_fdw--1.0.sql $(EXTENSIONS_FOLDER)/osm_fdw.control $(LIB_FOLDER)/osm_fdw.so
	ln -s $(CURRENT_FOLDER)/osm_fdw--1.0.sql $(EXTENSIONS_FOLDER)/osm_fdw--1.0.sql
	ln -s $(CURRENT_FOLDER)/osm_fdw.control $(EXTENSIONS_FOLDER)/osm_fdw.control
	ln -s $(CURRENT_FOLDER)/osm_fdw.so $(LIB_FOLDER)/osm_fdw.so