/*
	pibeatsaber - Beat Saber historian application that tracks players
	Copyright (C) 2019-2019 Johannes Bauer

	This file is part of pibeatsaber.

	pibeatsaber is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; this program is ONLY licensed under
	version 3 of the License, later versions are explicitly excluded.

	pibeatsaber is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	Johannes Bauer <JohannesBauer@gmx.de>
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <yajl_parse.h>
#include "jsondom.h"

static struct jsondom_t *jsondom_new(enum jsondom_type_t elementtype, struct jsondom_t *parent);

struct yajl_parsing_ctx_t {
	struct jsondom_t **next;
	struct jsondom_t *current;
	struct jsondom_t *root;
};

static char *yajl_strdup(const unsigned char *string, unsigned int length) {
	for (unsigned int i = 0; i < length; i++) {
		if (!string[i]) {
			fprintf(stderr, "Strings containing 0x00 are not supported.\n");
			return NULL;
		}
	}
	char *result = malloc(length + 1);
	if (!result) {
		perror("malloc");
		return NULL;
	}
	memcpy(result, string, length);
	result[length] = 0;
	return result;
}

static struct jsondom_t **array_add_element(struct jsondom_array_t *array) {
	struct jsondom_t **new_elements = realloc(array->elements, sizeof(*array->elements) * (array->element_cnt + 1));
	if (new_elements) {
		array->elements = new_elements;
	}
	array->elements[array->element_cnt] = NULL;
	array->element_cnt++;
	return &array->elements[array->element_cnt - 1];
}

static int yajl_add_primitive(struct yajl_parsing_ctx_t *ctx, struct jsondom_t *new_primitive) {
	if (!new_primitive) {
		return 0;
	}
	if ((ctx->next == NULL) && (ctx->current) && (ctx->current->elementtype == JD_ARRAY)) {
		/* Create new element */
		ctx->next = array_add_element(&ctx->current->element.array);
	}

	if (ctx->next) {
		(*ctx->next) = new_primitive;
		ctx->next = NULL;
		return 1;
	}
	jsondom_free(new_primitive);
	return 0;
}


static int yajl_parse_null(void *vctx) {
	struct yajl_parsing_ctx_t* ctx = (struct yajl_parsing_ctx_t*)vctx;
	struct jsondom_t *new_element = jsondom_new(JD_NULLVAL, ctx->current);
	return yajl_add_primitive(ctx, new_element);
}

static int yajl_parse_boolean(void *vctx, int boolean) {
	struct yajl_parsing_ctx_t* ctx = (struct yajl_parsing_ctx_t*)vctx;
	struct jsondom_t *new_element = jsondom_new(JD_BOOLEAN, ctx->current);
	if (new_element) {
		new_element->element.boolean_value = (boolean != 0);
	}
	return yajl_add_primitive(ctx, new_element);
}

static int yajl_parse_double(void *vctx, double dblvalue) {
	struct yajl_parsing_ctx_t* ctx = (struct yajl_parsing_ctx_t*)vctx;
	struct jsondom_t *new_element = jsondom_new(JD_DOUBLE, ctx->current);
	if (new_element) {
		new_element->element.double_value = dblvalue;
	}
	return yajl_add_primitive(ctx, new_element);
}

static int yajl_parse_integer(void *vctx, long long integer) {
	struct yajl_parsing_ctx_t* ctx = (struct yajl_parsing_ctx_t*)vctx;
	struct jsondom_t *new_element = jsondom_new(JD_INTEGER, ctx->current);
	if (new_element) {
		new_element->element.int_value = integer;
	}
	return yajl_add_primitive(ctx, new_element);
}

static int yajl_parse_string(void *vctx, const unsigned char *string, size_t str_length) {
	struct yajl_parsing_ctx_t* ctx = (struct yajl_parsing_ctx_t*)vctx;
	struct jsondom_t *new_element = jsondom_new(JD_STRING, ctx->current);
	if (new_element) {
		new_element->element.str_value = yajl_strdup(string, str_length);
	}
	return yajl_add_primitive(ctx, new_element) && (new_element->element.str_value);
}

static struct jsondom_t **dict_add_key(struct jsondom_dict_t *dict, const unsigned char *key, unsigned int keylen) {
	char **new_keys = realloc(dict->keys, sizeof(*dict->keys) * (dict->element_cnt + 1));
	if (new_keys) {
		dict->keys = new_keys;
	}
	struct jsondom_t **new_elements = realloc(dict->elements, sizeof(*dict->elements) * (dict->element_cnt + 1));
	if (new_elements) {
		dict->elements = new_elements;
	}
	char *new_key = yajl_strdup(key, keylen);
	if (new_keys && new_elements && new_key) {
		dict->keys[dict->element_cnt] = new_key;
		dict->elements[dict->element_cnt] = NULL;
		dict->element_cnt++;
		return &dict->elements[dict->element_cnt - 1];
	} else {
		return NULL;
	}
}

static int yajl_parse_map_key(void *vctx, const unsigned char *key, size_t key_length) {
	struct yajl_parsing_ctx_t *ctx = (struct yajl_parsing_ctx_t*)vctx;
	if (ctx->current && (!ctx->next) && (ctx->current->elementtype == JD_DICT)) {
		ctx->next = dict_add_key(&ctx->current->element.dict, key, key_length);
		if (!ctx->next) {
			return 0;
		}
	}
	return 1;
}

static int yajl_parse_start_map(void *vctx) {
	struct yajl_parsing_ctx_t* ctx = (struct yajl_parsing_ctx_t*)vctx;
	struct jsondom_t *new_element = jsondom_new(JD_DICT, ctx->current);
	if (new_element) {
		ctx->current = new_element;
	}
	return yajl_add_primitive(ctx, new_element);
}

static int yajl_parse_end_map(void *vctx) {
	struct yajl_parsing_ctx_t *ctx = (struct yajl_parsing_ctx_t*)vctx;
	ctx->current = ctx->current->parent;
	return 1;
}

static int yajl_parse_start_array(void *vctx) {
	struct yajl_parsing_ctx_t* ctx = (struct yajl_parsing_ctx_t*)vctx;
	struct jsondom_t *new_element = jsondom_new(JD_ARRAY, ctx->current);
	int success = yajl_add_primitive(ctx, new_element);
	if (new_element) {
		ctx->current = new_element;
	}
	return success;
}

static int yajl_parse_end_array(void *vctx) {
	struct yajl_parsing_ctx_t *ctx = (struct yajl_parsing_ctx_t*)vctx;
	ctx->current = ctx->current->parent;
	return 1;
}

static struct jsondom_t *jsondom_new(enum jsondom_type_t elementtype, struct jsondom_t *parent) {
	struct jsondom_t *element = calloc(sizeof(struct jsondom_t), 1);
	if (element) {
		element->elementtype = elementtype;
		element->parent = parent;
	}
	return element;
}

struct jsondom_t *jsondom_parse(const char *json_text) {
	/* Now try to parse the JSON message that we received */
	yajl_callbacks ycallbacks = {
		.yajl_start_map = yajl_parse_start_map,
		.yajl_end_map = yajl_parse_end_map,
		.yajl_start_array = yajl_parse_start_array,
		.yajl_end_array = yajl_parse_end_array,
		.yajl_map_key = yajl_parse_map_key,
		.yajl_boolean = yajl_parse_boolean,
		.yajl_integer = yajl_parse_integer,
		.yajl_double = yajl_parse_double,
		.yajl_null = yajl_parse_null,
		.yajl_string = yajl_parse_string,
	};
	struct yajl_parsing_ctx_t parsing_ctx = {
		.root = NULL,
		.next = &parsing_ctx.root,
	};
	yajl_handle yhandle = yajl_alloc(&ycallbacks, NULL, &parsing_ctx);
	if (!yhandle) {
		perror("yajl_alloc");
		return NULL;
	}
	yajl_status parse_status = yajl_parse(yhandle, (unsigned char*)json_text, strlen(json_text));
	yajl_free(yhandle);
	if (parse_status != yajl_status_ok) {
		return NULL;
	}

	return parsing_ctx.root;
}

static void jsondom_dump_indent(const struct jsondom_t *element, unsigned int indent) {
	if (!element) {
		printf("[!]");
	} else if (element->elementtype == JD_NULLVAL) {
		printf("null");
	} else if (element->elementtype == JD_DICT) {
		printf("{\n");
		for (unsigned int i = 0; i < element->element.dict.element_cnt; i++) {
			printf("   \"%s\": ", element->element.dict.keys[i]);
			jsondom_dump_indent(element->element.dict.elements[i], indent + 1);
			printf(",\n");
		}
		printf("}\n");
	} else if (element->elementtype == JD_ARRAY) {
		printf("[\n");
		for (unsigned int i = 0; i < element->element.array.element_cnt; i++) {
			jsondom_dump_indent(element->element.array.elements[i], indent + 1);
			printf(",\n");
		}
		printf("]\n");
	} else if (element->elementtype == JD_STRING) {
		printf("\"%s\"", element->element.str_value);
	} else if (element->elementtype == JD_INTEGER) {
		printf("%ld", element->element.int_value);
	} else if (element->elementtype == JD_DOUBLE) {
		printf("%f", element->element.double_value);
	} else if (element->elementtype == JD_BOOLEAN) {
		printf("%s", element->element.boolean_value ? "true" : "false");
	} else {
		printf("???");
	}
}

void jsondom_dump(const struct jsondom_t *element) {
	jsondom_dump_indent(element, 0);
}

void jsondom_free(struct jsondom_t *element) {
	if (!element) {
		return;
	}
	if (element->elementtype == JD_ARRAY) {
		for (unsigned int i = 0; i < element->element.array.element_cnt; i++) {
			jsondom_free(element->element.array.elements[i]);
		}
		free(element->element.array.elements);
	} else if (element->elementtype == JD_DICT) {
		for (unsigned int i = 0; i < element->element.dict.element_cnt; i++) {
			free(element->element.dict.keys[i]);
			jsondom_free(element->element.dict.elements[i]);
		}
		free(element->element.dict.keys);
		free(element->element.dict.elements);
	} else if (element->elementtype == JD_STRING) {
		free(element->element.str_value);
	}
	free(element);
}


struct jsondom_t* jsondom_get_dict(struct jsondom_t *element, const char *key) {
	if (!element) {
		return NULL;
	}
	if (element->elementtype != JD_DICT) {
		return NULL;
	}
	for (unsigned int i = 0; i < element->element.dict.element_cnt; i++) {
		if (!strcmp(element->element.dict.keys[i], key)) {
			return element->element.dict.elements[i];
		}
	}
	return NULL;
}

char *jsondom_get_dict_str(struct jsondom_t *element, const char *key) {
	struct jsondom_t *value = jsondom_get_dict(element, key);
	if (value && (value->elementtype == JD_STRING)) {
		return value->element.str_value;
	} else {
		return NULL;
	}
}

#ifdef TEST_JSONDOM
// gcc -Wall -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500 -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=format -Wshadow -Wswitch -pthread -std=c11 -DTEST_JSONDOM jsondom.c -o jsondom -ggdb3 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer -D_FORTITY_SOURCE=2 `pkg-config --cflags --libs yajl` && ./jsondom

int main(void) {
	struct jsondom_t *root = jsondom_parse("{ \"foo\": \"bar\", \"blah\": 12345, \"muh\": { \"x\": null, \"y\": null, \"z\": 123.456, \"yes\": true, \"no\": false, \"array\": [ null, 123, \"foo\", [ 3,2,1 ], true, false ] } }");
	if (root) {
		jsondom_dump(root);
	}
	jsondom_free(root);
	return 0;
}
#endif
