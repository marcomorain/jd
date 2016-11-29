#include <stdio.h>
#include <stdlib.h>
#include "parson.h"


void usage() {
}

void diff(JSON_Value* a, JSON_Value* b, JSON_Array* differences)
{
}

int main(int argc, const char * argv[]) {
    
    if (argc != 3) {
        usage();
        return EXIT_FAILURE;
    }
    
    JSON_Value* root = json_value_init_object();
    JSON_Array* differences = json_value_get_array(root);
    
    JSON_Value* tree_a = json_parse_file(argv[1]);
    JSON_Value* tree_b = json_parse_file(argv[2]);
    
    diff(tree_a, tree_b, differences);
    
    char* output = json_serialize_to_string(root);
    puts("differences:");
    puts(output);

    json_value_free(tree_a);
    json_value_free(tree_b);
    json_value_free(root);
    
    return 0;
}
