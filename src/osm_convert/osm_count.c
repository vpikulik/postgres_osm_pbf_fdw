
#define _OSM_DEBUG

#include <stdio.h>

#include "osm_reader.h"


int main (int argc, const char * argv[]) {
    FILE *file = fopen(argv[1], "r");

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    printf("File size: %d\n", file_size);
    fseek(file, 0, SEEK_SET);

    int count = get_osm_item_count(file, file_size);
    printf("Items count: %d\n", count);

    fclose(file);
}