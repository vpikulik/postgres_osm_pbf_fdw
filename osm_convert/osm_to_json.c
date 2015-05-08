
#define _OSM_DEBUG

#include <stdio.h>

#include "type_defs.h"
#include "osm_reader.h"
#include "json_encode.h"


void print_progress(FILE *file, int file_size) {
    int current_pos = ftell(file);
    printf("Progress: %d from %d\n", current_pos, file_size);
};


int main (int argc, const char * argv[]) {
    FILE *fl = fopen(argv[1], "r");
    FILE *out = fopen("/tmp/out.json", "w");

    fseek(fl, 0, SEEK_END);
    int file_size = ftell(fl);
    printf("File size: %d\n", file_size);
    fseek(fl, 0, SEEK_SET);

    int index = 0;
    Cursor* cursor = alloc_cursor();
    clear_cursor(cursor);
    cursor->position = -1;
    read_osm_header(cursor, fl);

    OsmItem* item;
    do {
        item = read_osm_item(cursor, fl, file_size);
        if (item) {
            char* json_item_txt = encode_item(item);
            fputs(json_item_txt, out);
            free(json_item_txt);
            fputs("\n", out);
        }
        index += 1;
    } while (item != NULL);

    free_cursor(cursor);

    fclose(fl);
    fclose(out);
}
