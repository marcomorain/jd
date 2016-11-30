#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parson.h"

void usage() {
}

void replace_op(JSON_Array* differences, const char* path, JSON_Value* with) {
	JSON_Value* value = json_value_init_object();
	JSON_Object* replace = json_value_get_object(value);
	json_object_set_string(replace, "op", "replace");
	json_object_set_string(replace, "path", path);
	json_object_set_value(replace, "value", json_value_deep_copy(with));
	json_array_append_value(differences, value);
}

static size_t smin(size_t a, size_t b)
{
	return a > b ? b : a;
}

static size_t smax(size_t a, size_t b)
{
	return a < b ? b : a;
}

void diff(const char* path, JSON_Value* a, JSON_Value* b, JSON_Array* differences)
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
		const JSON_Object* ao = json_value_get_object(a);
		const JSON_Object* bo = json_value_get_object(b);
		const size_t a_count = json_object_get_count(ao);
		for (size_t i = 0; i < a_count; i++)
		{
			const char* key = json_object_get_name(ao, i);
			JSON_Value* aval = json_object_get_value(ao, key);
			JSON_Value* bval = json_object_get_value(bo, key);

			// TODO - keys in B not in A.

			if (!json_value_equals(aval, bval))
			{
				char buffer[1024] = { 0 };
				sprintf(buffer, "%s%s/", path, key);
				diff(buffer, aval, bval, differences);
			}
		}
		break;
	}

	case JSONArray:
	{
		const JSON_Array* a_array = json_value_get_array(a);
		const JSON_Array* b_array = json_value_get_array(b);

		const size_t a_count = json_array_get_count(a_array);
		const size_t b_count = json_array_get_count(b_array);
		const size_t end = smax(a_count, b_count);

		for (size_t i = 0; i < end; i++)
		{
			JSON_Value* aval = json_array_get_value(a_array, i);
			JSON_Value* bval = json_array_get_value(b_array, i);

			if (json_value_equals(aval, bval))
			{
				continue;
			}

			char buffer[1024] = { 0 };
			sprintf(buffer, "%s%d/", path, i);
			diff(buffer, aval, bval, differences);
		}
		break;
	}
	}
}

int main(int argc, const char * argv[]) {

	if (argc != 3) {
		usage();
		return EXIT_FAILURE;
	}

	JSON_Value* root = json_value_init_array();
	JSON_Array* differences = json_value_get_array(root);

	JSON_Value* tree_a = json_parse_file(argv[1]);
	JSON_Value* tree_b = json_parse_file(argv[2]);

	diff("/", tree_a, tree_b, differences);

	char* output = json_serialize_to_string(root);
	puts("differences:");
	puts(output);

	json_value_free(root);
	json_value_free(tree_a);
	json_value_free(tree_b);


	return 0;
}
