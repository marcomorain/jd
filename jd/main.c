#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parson.h"

void usage(void) {
}

static void value_op(const char* op, JSON_Array* differences, const char* path, JSON_Value* with) {
    JSON_Value* value = json_value_init_object();
    JSON_Object* replace = json_value_get_object(value);
    json_object_set_string(replace, "op", op);
    json_object_set_string(replace, "path", path);
    if (with)
    {
        json_object_set_value(replace, "value", json_value_deep_copy(with));
    }
    json_array_append_value(differences, value);
}

static void replace_op(JSON_Array* differences, const char* path, JSON_Value* with)
{
    value_op("replace", differences, path, with);
}

static void add_op(JSON_Array* differences, const char* path, JSON_Value* with)
{
    value_op("add", differences, path, with);
}

static void remove_op(JSON_Array* differences, const char* path)
{
    value_op("remove", differences, path, 0);
}

static size_t smin(size_t a, size_t b)
{
	return a > b ? b : a;
}

static size_t smax(size_t a, size_t b)
{
	return a < b ? b : a;
}

static void diff(const char* path, JSON_Value* a, JSON_Value* b, JSON_Array* differences);


static void diff_object(const char* path, const JSON_Object* a, const JSON_Object* b, JSON_Array* differences)
{
    const size_t a_count = json_object_get_count(a);
    const size_t b_count = json_object_get_count(b);
    
    // For all keys in A
    for (size_t i = 0; i < a_count; i++)
    {
        const char* key = json_object_get_name(a, i);
        JSON_Value* aval = json_object_get_value(a, key);
        JSON_Value* bval = json_object_get_value(b, key);
        
        if (json_value_equals(aval, bval))
        {
            continue;
        }
        
        char buffer[1024] = { 0 };
        sprintf(buffer, "%s%s/", path, key);
        
        if (!bval)
        {
            remove_op(differences, buffer);
        }
        else
        {
            diff(buffer, aval, bval, differences);
        }
        
    }
    
    // For all keys in B
    for (size_t i = 0; i < b_count; i++)
    {
        const char* key = json_object_get_name(b, i);
        JSON_Value* aval = json_object_get_value(a, key);
        JSON_Value* bval = json_object_get_value(b, key);
        
        // We only want to find keys in B that are not in A.
        // All keys in A are already covered.
        if (aval)
        {
            continue;
        }
        
        char buffer[1024] = { 0 };
        sprintf(buffer, "%s%s/", path, key);
        
        add_op(differences, buffer, bval);
        
    }
}

static void diff_array(const char* path, const JSON_Array* a, const JSON_Array* b, JSON_Array* differences)
{
    const size_t a_count = json_array_get_count(a);
    const size_t b_count = json_array_get_count(b);
    const size_t end = smax(a_count, b_count);
    
    for (size_t i = 0; i < end; i++)
    {
        JSON_Value* aval = json_array_get_value(a, i);
        JSON_Value* bval = json_array_get_value(b, i);
        
        if (json_value_equals(aval, bval))
        {
            continue;
        }
        
        char buffer[1024] = { 0 };
        sprintf(buffer, "%s%lu/", path, i);
        
        if (!aval)
        {
            add_op(differences, buffer, bval);
        }
		else if (!bval)
        {
            remove_op(differences, buffer);
        }
        else
        {
            diff(buffer, aval, bval, differences);
        }
        
    }
}

static void diff(const char* path, JSON_Value* a, JSON_Value* b, JSON_Array* differences)
{
	if (json_value_equals(a, b))
	{
		return;
	}

	const JSON_Value_Type type_a = json_value_get_type(a);
	const JSON_Value_Type type_b = json_value_get_type(b);

	if (type_a != type_b)
	{
		replace_op(differences, path, b);
		return;
	}

	switch (type_a)
	{

	case JSONNull:
	case JSONString:
	case JSONNumber:
	case JSONBoolean:
		replace_op(differences, path, b);
		return;

	case JSONObject:
	{
        diff_object(path, json_value_get_object(a), json_value_get_object(b), differences);
		break;
	}

	case JSONArray:
	{
        diff_array(path, json_value_get_array(a), json_value_get_array(b), differences);
        break;
    }
            
    default:
    {
        assert(0);
    }
    }
}

static int check_arg(const char* arg, const char* short_name, const char* long_name)
{
	return (0 == strcmp(short_name, arg)) ||
		   (0 == strcmp(long_name, arg));
}

enum
{
	// From diff(1) man page:
	// Exit status is 0 if inputs are the same, 1 if different, 2 if trouble.

	JD_STATUS_SAME = 0,
	JD_STATUS_DIFFERENT = 1,
	JD_STATUS_TROUBLE = 2
};

int main(int argc, const char * argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (check_arg(argv[i], "-h", "--help"))
		{
			usage();
			return EXIT_SUCCESS;
		}
	}

	if (argc != 3)
	{
		usage();
		return JD_STATUS_TROUBLE;
	}

	JSON_Value* root = json_value_init_array();
	JSON_Array* differences = json_value_get_array(root);

	JSON_Value* tree_a = json_parse_file(argv[1]);
	JSON_Value* tree_b = json_parse_file(argv[2]);

	diff("/", tree_a, tree_b, differences);

	// Print nothing and return success if the documents were the same.
	if (json_array_get_count(differences) == 0)
	{
		return JD_STATUS_SAME;
	}

	char* output = json_serialize_to_string_pretty(root);
	puts(output);
    json_free_serialized_string(output);

	json_value_free(root);
	json_value_free(tree_a);
	json_value_free(tree_b);

	return JD_STATUS_DIFFERENT;
}
