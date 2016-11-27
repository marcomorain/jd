#include <stdio.h>
#include <stdlib.h>
#include "json.h"

int read_file(const char* path, char** data, long* size) {
    
    // TODO - mmap the file.
    
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);
    
    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);
    
    string[fsize] = 0;
    
    *size = fsize;
    *data = string;
    
    return 0;
}


void usage() {
}

void diff(struct json_value_s* a, struct json_value_s* b)
{
    if (a->type != b->type) {
        puts("remove a");
        puts("insert b");
    }
}

int main(int argc, const char * argv[]) {
    
    if (argc != 3) {
        usage();
        return EXIT_FAILURE;
    }

    
    char *a, *b;
    long size_a, size_b;
    
    read_file(argv[1], &a, &size_a);
    read_file(argv[2], &b, &size_b);
    
    struct json_value_s* tree_a = json_parse(a, size_a);
    struct json_value_s* tree_b = json_parse(b, size_b);
    
    diff(tree_a, tree_b);
    
    
    puts(json_write_pretty(tree_a, "  ", "\n", 0));
    
    free(a);
    free(b);
    free(tree_a);
    free(tree_b);
    
    return 0;
}
