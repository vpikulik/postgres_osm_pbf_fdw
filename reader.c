#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileformat.pb-c.h"
#include "osmformat.pb-c.h"

// #include "type_defs.h"
#include "json_encode.h"
#include "zdecode.h"

#define FILENAME_SIZE 11


OSMPBF__BlobHeader* read_blob_header(FILE *file, int header_size) {
    void *buf = malloc(header_size);
    fread(buf, header_size, 1, file);
    OSMPBF__BlobHeader *header;
    header = osmpbf__blob_header__unpack(NULL, header_size, buf);
    free(buf);

    return header;
};

ResizedBuffer* read_blob(FILE* file, OSMPBF__BlobHeader* header) {
    void* buf = malloc(header->datasize);
    fread(buf, header->datasize, 1, file);
    OSMPBF__Blob* blob = osmpbf__blob__unpack(NULL, header->datasize, buf);
    free(buf);

    if (blob->has_raw_size) {
        // printf("Raw size: %d\n", blob->raw_size);
    }

    if (blob->has_raw) {
        ProtobufCBinaryData *rdata = &blob->raw;
        // printf("Raw size: %d\n", rdata->len);
        ResizedBuffer *data = init_resized_buffer();
        append_data(data, rdata->len, rdata->data);
        osmpbf__blob__free_unpacked(blob, NULL);
        return data;

    } else if (blob->has_zlib_data) {
        ProtobufCBinaryData *zdata = &blob->zlib_data;
        // printf("Z size: %d\n", zdata->len);
        ResizedBuffer *data = zdecode(zdata->data, zdata->len);
        // printf("Out size: %d\n", data->size);
        osmpbf__blob__free_unpacked(blob, NULL);
        return data;

    } else if (blob->has_lzma_data) {
        printf("Do not support lzma\n");
    }
};


char** read_osm_string_table(OSMPBF__StringTable *stringtable) {
    char** strings = malloc(sizeof(char*) * stringtable->n_s);
    int i;
    for (i=0; i<stringtable->n_s; i++) {
        ProtobufCBinaryData item = stringtable->s[i];
        char* str = (char*) calloc(item.len + 1, sizeof(char));
        memcpy(str, item.data, item.len);
        strings[i] = str;
    }
    return strings;
};


OSMPBF__HeaderBlock* read_osm_header_block(Cursor* cursor, ResizedBuffer* data){
    OSMPBF__HeaderBlock *header_block = osmpbf__header_block__unpack(NULL, data->size, data->data);
    printf("Count of required features: %d\n", header_block->n_required_features);
    return header_block;
};


void read_osm_dense_nodes(Cursor* cursor, OSMPBF__DenseNodes *dense, char** strings) {
    printf("Dense ids: %d, %d, %d, %d\n", dense->n_id, dense->n_lat, dense->n_lon, dense->n_keys_vals);
    if (dense->n_id == 0) return;

    Node** nodes = malloc(sizeof(Node*) * dense->n_id);
    int i;
    for (i=0; i<dense->n_id; i++) {
        Node* node = init_node();
        node->id = dense->id[i];
        node->lat = dense->lat[i];
        node->lon = dense->lon[i];
        nodes[i] = node;
        cursor_add_node(cursor, node);
    };

    i = 0;
    int node_index = 0;
    do {
        int32_t key_index = dense->keys_vals[i];
        if (key_index == 0) {
            i += 1;
            node_index += 1;
        } else {
            int32_t val_index = dense->keys_vals[i+1];
            i += 2;
            Tag* tag = (Tag*)malloc(sizeof(Tag));
            tag->key = strings[key_index];
            tag->value = strings[val_index];
            node_add_tag(nodes[node_index], tag);
        }
    } while (i < dense->n_keys_vals);
    free(nodes);
};


void read_osm_primitive_group(Cursor* cursor, OSMPBF__PrimitiveGroup *primitive_group, char** strings) {
    if (primitive_group->dense) {
        read_osm_dense_nodes(cursor, primitive_group->dense, strings);
    };
};


void read_osm_primitive_block(Cursor* cursor, ResizedBuffer *data){
    OSMPBF__PrimitiveBlock* primitive_block = osmpbf__primitive_block__unpack(NULL, data->size, data->data);
    printf("Count of primitive groups: %d\n", primitive_block->n_primitivegroup);
    char** strings = read_osm_string_table(primitive_block->stringtable);
    int i;
    for (i=0; i<primitive_block->n_primitivegroup; i++) {
        read_osm_primitive_group(cursor, primitive_block->primitivegroup[i], strings);
    }
    free(strings);
    osmpbf__primitive_block__free_unpacked(primitive_block, NULL);
};


void fill_cursor(Cursor* cursor, FILE* file, short osm_header) {
    OSMPBF__BlobHeader *header;
    ResizedBuffer *blob_data;

    // Read size of header
    int header_size;
    fread(&header_size, 4, 1, file);
    header_size = ntohl(header_size);

    //Read header
    header = read_blob_header(file, header_size);

    //Read blob
    blob_data = read_blob(file, header);

    if (osm_header) {
        read_osm_header_block(cursor, blob_data);
    } else {
        read_osm_primitive_block(cursor, blob_data);
    }

    osmpbf__blob_header__free_unpacked(header, NULL);
    free_resized_buffer(blob_data);
};


void print_progress(FILE *file, int file_size) {
    int current_pos = ftell(file);
    printf("Progress: %d from %d\n", current_pos, file_size);
};


int main (int argc, const char * argv[]) {
    FILE *fl = fopen("/home/promo/Downloads/belarus-latest.osm.pbf", "r");
    FILE *out = fopen("/tmp/out.json", "w");

    fseek(fl, 0, SEEK_END);
    int file_size = ftell(fl);
    fseek(fl, 0, SEEK_SET);

    int index = 0;
    int current_pos;
    for (index=0; index<3; index++){
        printf("Index: %d\n", index);
        print_progress(fl, file_size);
        current_pos = ftell(fl);

        if (current_pos >= file_size) {
            printf("End of file\n");
            break;
        }

        Cursor* cursor = init_cursor();
        fill_cursor(cursor, fl, index == 0);

        int i;
        for(i=0; i<cursor->nodes_count; i++) {
            Node* node = cursor->nodes[i];
            json_t* jnode = encode_node(node);
            char* out_str = encode_json(jnode);
            fputs(out_str, out);
            fputs("\n", out);
        }

        free_cursor(cursor);
    }

    fclose(fl);
    fclose(out);
}
