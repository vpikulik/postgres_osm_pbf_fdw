#include <stdio.h>
#include <stdlib.h>

#include "fileformat.pb-c.h"
#include "osmformat.pb-c.h"

#include "type_defs.h"
#include "zdecode.h"

#define FILENAME_SIZE 11


OSMPBF__BlobHeader* read_blob_header(FILE *file, int header_size) {
    void *buf = malloc(header_size);
    fread(buf, header_size, 1, file);
    OSMPBF__BlobHeader *header;
    header = osmpbf__blob_header__unpack(NULL, header_size, buf);
    printf("type: %s\n", header->type);
    printf("datasize: %d\n", header->datasize);
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
    printf("Table size: %d\n", stringtable->n_s);
    char** strings = malloc(sizeof(char*) * stringtable->n_s);
    int i;
    for (i=0; i<stringtable->n_s; i++) {
        ProtobufCBinaryData item = stringtable->s[i];
        strings[i] = (char*) item.data;
    }
    return strings;
};


OSMPBF__HeaderBlock* read_osm_header_block(ResizedBuffer *data){
    OSMPBF__HeaderBlock *header_block = osmpbf__header_block__unpack(NULL, data->size, data->data);
    printf("Count of required features: %d\n", header_block->n_required_features);
    return header_block;
};


void read_osm_dense_nodes(OSMPBF__DenseNodes *dense, char** strings) {
    printf("Dense ids: %d, %d, %d, %d\n", dense->n_id, dense->n_lat, dense->n_lon, dense->n_keys_vals);
    if (dense->n_id == 0) return;

    Node** nodes = malloc(sizeof(Node*) * dense->n_id);
    int i;
    for (i=0; i<dense->n_id; i++) {
        Node* node = malloc(sizeof(Node));
        node->id = dense->id[i];
        node->lat = dense->lat[i];
        node->lon = dense->lon[i];
        node->tags_count = 0;
        nodes[i] = node;
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
            Tag* tag = malloc(sizeof(Tag));
            tag->key = strings[key_index];
            tag->value = strings[val_index];
            node_add_tag(nodes[node_index], tag);
        }
    } while (i < dense->n_keys_vals);
};


void read_osm_primitive_group(OSMPBF__PrimitiveGroup *primitive_group, char** strings) {
    read_osm_dense_nodes(primitive_group->dense, strings);
};


read_osm_primitive_block(ResizedBuffer *data){
    OSMPBF__PrimitiveBlock* primitive_block = osmpbf__primitive_block__unpack(NULL, data->size, data->data);
    printf("Count of primitive groups: %d\n", primitive_block->n_primitivegroup);
    char** strings = read_osm_string_table(primitive_block->stringtable);
    int i;
    for (i=0; i<primitive_block->n_primitivegroup; i++) {
        read_osm_primitive_group(primitive_block->primitivegroup[i], strings);
    }
    free(strings);
    osmpbf__primitive_block__free_unpacked(primitive_block, NULL);
};


void debug_data(int index, void* data, int size) {
    char filename[FILENAME_SIZE];
    snprintf(filename, FILENAME_SIZE, "/tmp/res_%d", index);

    printf("Write to %s\n", filename);
    FILE *rfl = fopen(filename, "w");
    fwrite(data, 1, size, rfl);
    fclose(rfl);
};


void print_progress(FILE *file, int file_size) {
    int current_pos = ftell(file);
    printf("Progress: %d from %d\n", current_pos, file_size);
};

int main (int argc, const char * argv[]) {
    FILE *fl = fopen("/home/promo/Downloads/belarus-latest.osm.pbf", "r");

    fseek(fl, 0, SEEK_END);
    int file_size = ftell(fl);
    fseek(fl, 0, SEEK_SET);

    OSMPBF__BlobHeader *header;
    ResizedBuffer *blob_data;

    int index = 0;
    int header_size;
    int current_pos;
    for (index=0;index<10;index++){
        printf("Index: %d\n", index);
        print_progress(fl, file_size);
        current_pos = ftell(fl);

        if (current_pos >= file_size) {
            printf("Enf of file\n");
            break;
        } 

        // Read size of header
        fread(&header_size, 4, 1, fl);
        header_size = ntohl(header_size);
        printf("Count of bytes %d\n", header_size);
        header = read_blob_header(fl, header_size);

        blob_data = read_blob(fl, header);
        osmpbf__blob_header__free_unpacked(header, NULL);

        if (index == 0) {
            read_osm_header_block(blob_data);
        } else {
            read_osm_primitive_block(blob_data);
        }

        // debug_data(index, blob_data->data, blob_data->size);

        free_resized_buffer(blob_data);        
    }

    fclose(fl);
}
