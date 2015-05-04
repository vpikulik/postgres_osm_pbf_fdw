
#include "osm_reader.h"


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


void read_osm_string_table(Cursor* cursor, OSMPBF__StringTable *stringtable) {
    cursor->strings_count = stringtable->n_s;
    char** strings = malloc(sizeof(char*) * stringtable->n_s);
    int i;
    for (i=0; i<stringtable->n_s; i++) {
        ProtobufCBinaryData item = stringtable->s[i];
        char* str = (char*) calloc(item.len + 1, sizeof(char));
        memcpy(str, item.data, item.len);
        strings[i] = str;
    }
    cursor->strings = strings;
};


OSMPBF__HeaderBlock* read_osm_header_block(Cursor* cursor, ResizedBuffer* data){
    OSMPBF__HeaderBlock *header_block = osmpbf__header_block__unpack(NULL, data->size, data->data);
    printf("Count of required features: %d\n", header_block->n_required_features);
    return header_block;
};


double get_lat(int64_t lat, OSMPBF__PrimitiveBlock* primitive_block){
    int64_t lat_offset;
    int32_t granularity;
    if (primitive_block->has_lat_offset) {
        lat_offset = primitive_block->has_lat_offset;
    } else {
        lat_offset = 0;
    };
    if (primitive_block->has_granularity) {
        granularity = primitive_block->granularity;
    } else {
        granularity = 1;
    };
    return .000000001 * (lat_offset + (granularity * lat));
};


double get_lon(int64_t lon, OSMPBF__PrimitiveBlock* primitive_block){
    int64_t lon_offset;
    int32_t granularity;
    if (primitive_block->has_lon_offset) {
        lon_offset = primitive_block->has_lon_offset;
    } else {
        lon_offset = 0;
    };
    if (primitive_block->has_granularity) {
        granularity = primitive_block->granularity;
    } else {
        granularity = 1;
    };
    return .000000001 * (lon_offset + (granularity * lon));
};


void read_osm_dense_nodes(Cursor* cursor, OSMPBF__DenseNodes *dense, char** strings, OSMPBF__PrimitiveBlock* primitive_block) {
    printf("Dense ids: %d, %d, %d, %d\n", dense->n_id, dense->n_lat, dense->n_lon, dense->n_keys_vals);
    if (dense->n_id == 0) return;

    OsmItem** items = malloc(sizeof(OsmItem*) * dense->n_id);
    int i;
    int64_t id = 0;
    int64_t lat = 0;
    int64_t lon = 0;
    for (i=0; i<dense->n_id; i++) {
        id = id + dense->id[i];
        lat = lat + dense->lat[i];
        lon = lon + dense->lon[i];

        OsmItem* item = init_item();
        item->type = NODE;
        item->id = id;
        item->lat = get_lat(lat, primitive_block);
        item->lon = get_lon(lon, primitive_block);
        items[i] = item;
        cursor_add_item(cursor, item);
    };

    i = 0;
    int item_index = 0;
    do {
        int32_t key_index = dense->keys_vals[i];
        if (key_index == 0) {
            i += 1;
            item_index += 1;
        } else {
            int32_t val_index = dense->keys_vals[i+1];
            i += 2;
            OsmTag* tag = (OsmTag*)malloc(sizeof(OsmTag));
            tag->key = strings[key_index];
            tag->value = strings[val_index];
            item_add_tag(items[item_index], tag);
        }
    } while (i < dense->n_keys_vals);
    free(items);
};


void read_osm_way(Cursor* cursor, OSMPBF__Way *way, char** strings, OSMPBF__PrimitiveBlock* primitive_block) {
    int i;

    OsmItem* item = init_item();
    item->type = WAY;
    item->id = way->id;
    cursor_add_item(cursor, item);

    int32_t key_index, val_index;
    for (i=0;i<way->n_keys;i++) {
        key_index = way->keys[i];
        val_index= way->vals[i];

        OsmTag* tag = (OsmTag*)malloc(sizeof(OsmTag));
        tag->key = strings[key_index];
        tag->value = strings[val_index];
        item_add_tag(item, tag);
    }
    if (way->n_refs > 0) {
        item_copy_node_refs(item, way->n_refs, way->refs);
    }
};


void read_osm_relation(Cursor* cursor, OSMPBF__Relation *relation, char** strings, OSMPBF__PrimitiveBlock* primitive_block) {
    int i;
    OsmItem* item = init_item();
    item->type = RELATION;
    item->id = relation->id;
    cursor_add_item(cursor, item);

    int32_t key_index, val_index;
    for (i=0; i<relation->n_keys; i++) {
        key_index = relation->keys[i];
        val_index= relation->vals[i];

        OsmTag* tag = (OsmTag*)malloc(sizeof(OsmTag));
        tag->key = strings[key_index];
        tag->value = strings[val_index];
        item_add_tag(item, tag);
    }

    int64_t member_id = 0;
    for (i=0; i<relation->n_memids; i++) {
        member_id += relation->memids[i];
        OsmMember *member = (OsmMember*)malloc(sizeof(OsmMember));
        member->role = strings[relation->roles_sid[i]];
        member->id = member_id;
        member->type = UNDEFINED;
        if (relation->types[i] == OSMPBF__RELATION__MEMBER_TYPE__NODE) {
            member->type = NODE;
        } else if (relation->types[i] == OSMPBF__RELATION__MEMBER_TYPE__WAY) {
            member->type = WAY;
        } else if (relation->types[i] == OSMPBF__RELATION__MEMBER_TYPE__RELATION) {
            member->type = RELATION;
        }
        item_add_member(item, member);
    }
}


void read_osm_primitive_group(Cursor* cursor, OSMPBF__PrimitiveGroup *primitive_group, char** strings, OSMPBF__PrimitiveBlock* primitive_block) {
    int i;
    if (primitive_group->dense) {
        read_osm_dense_nodes(cursor, primitive_group->dense, strings, primitive_block);
    }
    if (primitive_group->n_ways > 0) {
        for (i=0; i<primitive_group->n_ways; i++) {
            read_osm_way(cursor, primitive_group->ways[i], strings, primitive_block);
        }
    }
    if (primitive_group->n_relations > 0) {
        for (i=0; i<primitive_group->n_relations; i++) {
            read_osm_relation(cursor, primitive_group->relations[i], strings, primitive_block);
        }
    }
};


void read_osm_primitive_block(Cursor* cursor, ResizedBuffer *data){
    OSMPBF__PrimitiveBlock* primitive_block = osmpbf__primitive_block__unpack(NULL, data->size, data->data);
    read_osm_string_table(cursor, primitive_block->stringtable);
    int i;
    for (i=0; i<primitive_block->n_primitivegroup; i++) {
        read_osm_primitive_group(cursor, primitive_block->primitivegroup[i], cursor->strings, primitive_block);
    }
    osmpbf__primitive_block__free_unpacked(primitive_block, NULL);
};


void _load_data_from_file(Cursor* cursor, FILE* file, short read_header) {
    // Read size of header
    int header_size;
    fread(&header_size, 4, 1, file);
    header_size = ntohl(header_size);

    //Read header
    OSMPBF__BlobHeader* header = read_blob_header(file, header_size);

    //Read blob
    ResizedBuffer* blob_data = read_blob(file, header);

    if (read_header) {
        // just ignore this header block
        // read_osm_header_block(cursor, blob_data);
    } else {
        read_osm_primitive_block(cursor, blob_data);
    }

    osmpbf__blob_header__free_unpacked(header, NULL);
    free_resized_buffer(blob_data);
};


void read_osm_header(Cursor* cursor, FILE* file) {
    _load_data_from_file(cursor, file, 1);
}


short int check_eof(FILE* file, int file_size) {
    if (ftell(file) >= file_size) {
        return 1;
    } else {
        return 0;
    }
}


OsmItem* read_osm_item(Cursor* cursor, FILE* file, int file_size) {

    if (cursor->position == -1) {
        if (check_eof(file,file_size)) {
            return NULL;
        }
        do {
            clear_cursor(cursor);
            _load_data_from_file(cursor, file, 0);
        } while (cursor->items_count == 0 && !check_eof(file,file_size));
        cursor->position = 0;
    }

    if (cursor->items_count == 0) {
        return NULL;
    }

    if (cursor->position < cursor->items_count - 1) {
        OsmItem* item = cursor->items[cursor->position];
        cursor->position ++;
        if (cursor->position >= cursor->items_count - 1) {
            cursor->position = -1;
        }
        return item;
    }
}
