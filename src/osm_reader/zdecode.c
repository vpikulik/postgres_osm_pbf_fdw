
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zlib.h"

#include "zdecode.h"

#define CHUNK 100


void append_data(ResizedBuffer *data, int size, void *in) {
    if (data->size == 0) {
        data->data = (void*)malloc(size);
    }
    else {
        data->data = realloc(data->data, data->size + size);
    }
    memcpy(data->data + data->size, in, size);
    data->size = data->size + size;
};

ResizedBuffer* init_resized_buffer() {
    ResizedBuffer *out = (ResizedBuffer*)malloc(sizeof(ResizedBuffer));
    out->size = 0;
    return out;
};

void free_resized_buffer(ResizedBuffer *data) {
    free(data->data);
    free(data);
};

ResizedBuffer* zdecode(void* input, int input_size){
    void* out = malloc(CHUNK);
    int out_buffer_size;

    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    inflateInit(&strm);

    strm.avail_in = input_size;
    strm.next_in = input;

    ResizedBuffer *result = init_resized_buffer();

    do {
        strm.next_out = out;
        strm.avail_out = CHUNK;
        inflate(&strm, Z_NO_FLUSH);
        out_buffer_size = CHUNK - strm.avail_out;
        append_data(result, out_buffer_size, out);

    } while (strm.avail_out == 0);
    inflateEnd(&strm);

    free(out);
    return result;
}
