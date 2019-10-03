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

#ifndef __JSONDOM_H__
#define __JSONDOM_H__

#include <stdbool.h>
#include <stdint.h>

enum jsondom_type_t {
	JD_UNDEFINED = 0,
	JD_ARRAY,
	JD_DICT,
	JD_INTEGER,
	JD_DOUBLE,
	JD_STRING,
	JD_BOOLEAN,
	JD_NULLVAL,
};

struct jsondom_array_t {
	unsigned int element_cnt;
	struct jsondom_t **elements;
};

struct jsondom_dict_t {
	unsigned int element_cnt;
	char **keys;
	struct jsondom_t **elements;
};

struct jsondom_t {
	enum jsondom_type_t elementtype;
	struct jsondom_t *parent;
	union {
		struct jsondom_array_t array;
		struct jsondom_dict_t dict;
		int64_t int_value;
		double double_value;
		bool boolean_value;
		char *str_value;
	} element;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
struct jsondom_t *jsondom_parse(const char *json_text);
void jsondom_dump(const struct jsondom_t *element);
void jsondom_free(struct jsondom_t *element);
struct jsondom_t* jsondom_get_dict(struct jsondom_t *element, const char *key);
char *jsondom_get_dict_str(struct jsondom_t *element, const char *key);
int64_t jsondom_get_dict_int(struct jsondom_t *element, const char *key);
double jsondom_get_dict_float(struct jsondom_t *element, const char *key);
bool jsondom_get_dict_bool(struct jsondom_t *element, const char *key);
struct jsondom_t* jsondom_get_dict_dict(struct jsondom_t *element, const char *key);
struct jsondom_t* jsondom_get_dict_array(struct jsondom_t *element, const char *key);
struct jsondom_t* jsondom_get_array_item(struct jsondom_t *element, unsigned int index);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
