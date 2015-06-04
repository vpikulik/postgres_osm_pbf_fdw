
CURRENT_FOLDER = $(shell pwd)

fileformat.pb-c.c:
	protoc-c --c_out=$(CURRENT_FOLDER) fileformat.proto

osmformat.pb-c.c:
	protoc-c --c_out=$(CURRENT_FOLDER) osmformat.proto
