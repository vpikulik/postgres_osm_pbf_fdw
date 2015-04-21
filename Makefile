
CURRENT_FOLDER = $(shell pwd)

FLAGS = -g -lz -lprotobuf-c

all: clean prepare_proto reader 

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

reader.o:
	gcc -c $(FLAGS) reader.c

reader: zdecode.o type_defs.o reader.o fileformat.pb-c.o osmformat.pb-c.o
	gcc -g -lprotobuf-c -lz -o reader reader.o type_defs.o zdecode.o fileformat.pb-c.o osmformat.pb-c.o

zpipe:
	gcc -g -lz -o zpipe zpipe.c
