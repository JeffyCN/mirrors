/*
 *  Copyright (c) 2020, Rockchip Electronics Co., Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */
#define _GNU_SOURCE

#include "common.h"

#include <time.h>
#include <string.h>

/* j2s list helpers */
typedef struct {
	void *data;
	void *next;
	void *prev;
} j2s_list;

void j2s_list_add(j2s_list *head, void *data) {
	j2s_list *entry = malloc(sizeof(j2s_list));
	DASSERT(entry, exit(-1));

	entry->data = data;

	entry->prev = head->prev;
	entry->next = head;
	((j2s_list *)head->prev)->next = entry;
	head->prev = entry;
}

void j2s_list_del(j2s_list *entry) {
	((j2s_list *)entry->prev)->next = entry->next;
	((j2s_list *)entry->next)->prev = entry->prev;
	free(entry);
}

#define j2s_list_init(head) \
	(head)->prev = (head)->next = head; (head)->data = NULL;

#define j2s_list_empty(head) \
	((head)->prev == head)

#define j2s_list_walk_safe(head, entry, next, obj) \
	for (entry = (head)->next, next = entry->next, obj = entry->data; \
	     entry != head; \
	     entry = next, next = entry->next, obj = entry->data)

#define j2s_list_walk(head, entry, obj) \
	for (entry = (head)->next, obj = entry->data; entry != head; \
	     entry = entry->next, obj = entry->data)

#define j2s_list_free(head, entry, free_data) \
	for (entry = (head)->next; entry != head; \
	     entry = entry->next, free(entry->prev)) \
	     if (free_data) free(entry->data)

#define j2s_list_find(head, entry, obj, s) \
	j2s_list_walk(head, entry, obj) \
	if (!strcmp(obj->name, s)) break; \
	if (entry == head) obj = NULL;

/* Info about each enums */
typedef struct {
	int id;
	char name[MAX_NAME]; /* Enum name */
	j2s_list values; /* value list */
	int value_id; /* Id of the first enum value */
	int num_value; /* Number of enum values */
} j2s_enum;

/* Info about each enum values */
typedef struct {
	int id;
	char name[MAX_NAME]; /* Value name */
} j2s_enum_value;

/* Info about each structs */
typedef struct {
	int id;
	char name[MAX_NAME]; /* Struct name */
	j2s_list child; /* Child list */
	int child_id; /* Id of the first child */
} j2s_struct;

/* Info about each struct members */
typedef struct {
	int id;
	char name[MAX_NAME]; /* Member name */
	char type_name[MAX_NAME]; /* Origin type name */
	j2s_type type;
	j2s_flag flags;

	char *desc; /* Optional desc string */

	void *next; /* Next child of the parent */
	j2s_struct *parent; /* Parent struct */

	j2s_struct *struct_obj; /* Struct info(if type is struct) */
	j2s_enum *enum_obj; /* Enum info(if type is enum) */
	void *len_obj; /* Dynamic array needs a @<name>_len obj to store len */

	int line; /* Line of the header file */
} j2s_obj;

/* J2S parsing context */
typedef struct {
	j2s_list structs;
	j2s_list enums;
	j2s_list objs;

	int num_enum_value;
	int num_struct;
	int num_enum;
	int num_obj;

	int depth;

	bool pending_bool;
	bool pending_enum;
	bool pending_struct;
	bool pending_comment;

	j2s_struct *struct_obj;
	j2s_enum *enum_obj;

	char *desc;

	j2s_struct *root_struct;

	FILE *fp; /* Input file */
	int line; /* Line of the header file */
} j2s_ctx;

#define J2S_TYPE(t) [t] = #t
const char j2s_types[][MAX_NAME] = {
	J2S_TYPE(J2S_TYPE_INT_8),
	J2S_TYPE(J2S_TYPE_UINT_8),
	J2S_TYPE(J2S_TYPE_INT_16),
	J2S_TYPE(J2S_TYPE_UINT_16),
	J2S_TYPE(J2S_TYPE_INT_32),
	J2S_TYPE(J2S_TYPE_UINT_32),
	J2S_TYPE(J2S_TYPE_INT_64),
	J2S_TYPE(J2S_TYPE_UINT_64),
	J2S_TYPE(J2S_TYPE_FLOAT),
	J2S_TYPE(J2S_TYPE_DOUBLE),
	J2S_TYPE(J2S_TYPE_STRING),
	J2S_TYPE(J2S_TYPE_STRUCT),
};

static inline
j2s_type j2s_parse_type(const char *type, j2s_flag flags) {
	if (!strcmp(type, "char") &&
	    (flags & (J2S_FLAG_ARRAY | J2S_FLAG_POINTER)))
		return J2S_TYPE_STRING;
	if (!strcmp(type, "char") || !strcmp(type, "bool") ||
	    !strcmp(type, "int8_t"))
		return J2S_TYPE_INT_8;
	if (!strcmp(type, "unsigned char") || !strcmp(type, "uint8_t"))
		return J2S_TYPE_UINT_8;
	if (!strcmp(type, "short") || !strcmp(type, "int16_t"))
		return J2S_TYPE_INT_16;
	if (!strcmp(type, "unsigned short") || !strcmp(type, "uint16_t"))
		return J2S_TYPE_UINT_16;
	if (!strcmp(type, "int") || !strcmp(type, "int32_t"))
		return J2S_TYPE_INT_32;
	if (!strcmp(type, "unsigned int") || !strcmp(type, "uint32_t"))
		return J2S_TYPE_UINT_32;
	if (!strcmp(type, "long") || !strcmp(type, "long long") ||
	    !strcmp(type, "int64_t"))
		return J2S_TYPE_INT_64;
	if (!strcmp(type, "unsigned long") ||
	    !strcmp(type, "unsigned long long") || !strcmp(type, "uint64_t"))
		return J2S_TYPE_UINT_64;
	if (!strcmp(type, "float"))
		return J2S_TYPE_FLOAT;
	if (!strcmp(type, "double"))
		return J2S_TYPE_DOUBLE;

	/* Fallback to struct, unsupported types would be catched later */
	return J2S_TYPE_STRUCT;
}

static inline
void strip_spaces(char **buf, bool strip_tails) {
	if (!buf || !*buf)
		return;

#define IS_SPACE(c) ((c) == '\t' || (c) == ' ' || (c) == ',' || \
		     (c) == '\'' || (c) == ';' || (c) == '\n' || (c) == '\r')

	/* Strip leading spaces */
	while (IS_SPACE(*buf[0]))
		(*buf)++;

	if (!strip_tails)
		return;

	/* Strip tail spaces */
	for (int i = strlen(*buf) - 1; i >= 0 && IS_SPACE((*buf)[i]); i--)
		(*buf)[i] = '\0';
}

/* Strip spaces and compare and eat pattern */
static inline
int parse_pattern(char **buf, const char *pattern) {
	int ret = -1;

	if (!buf || !*buf)
		return -1;

	/* Strip spaces */
	strip_spaces(buf, false);

	/* Compare and eat pattern */
	if (pattern && !strncmp(*buf, pattern, strlen(pattern))) {
		*buf += strlen(pattern);
		ret = 0;
	}

	/* Strip spaces */
	strip_spaces(buf, false);

	return ret;
}

/* Return the first pos of a or b */
static inline
char *strchr_first(const char *buf, const char a, const char b) {
	char *p1, *p2;

	if (a == b)
		return strchr(buf, a);

	p1 = strchrnul(buf, a);
	p2 = strchrnul(buf, b);

	if (p1 == p2)
		return NULL;

	return p1 < p2 ? p1 : p2;
}

static inline
void j2s_handle_comment(j2s_ctx *ctx, char *buf) {
	int size;

	strip_spaces(&buf, true);
	DBG("comment: %s\n", buf);

	if (!ctx->struct_obj)
		return;

	if (parse_pattern(&buf, J2S_DESC_MAGIC) < 0)
		return;

	/* Handle member desc (comments started with magic) */

	if (!ctx->desc)
		ctx->desc = strdup("");
	else
		strcat(ctx->desc, ", ");

	size = strlen(ctx->desc) + strlen(buf) + 10;

	ctx->desc = realloc(ctx->desc, size);
	DASSERT_MSG(ctx->desc, exit(-1), "desc size too large: %d\n", size);

	strcat(ctx->desc, buf);

	DBG("new desc: %s\n", ctx->desc);
}

static inline
int j2s_parse(j2s_ctx *ctx, char *buf) {
	j2s_enum_value *enum_value = NULL;
	j2s_struct *struct_obj = NULL;
	j2s_enum *enum_obj = NULL;
	j2s_obj *obj = NULL;
	j2s_flag flags;

	char pending[MAX_LINE] = {0};
	char *ptr, *type, *name;

	DBG("parsing '%s' at %d\n", buf, ctx->line);

	strip_spaces(&buf, true);

	/* 1. Handle multiline comments */
	if (ctx->pending_comment) {
		ptr = strstr(buf, "*/");
		if (ptr) {
			/* End of multiline comment */
			*ptr = '\0';

			j2s_handle_comment(ctx, buf);

			buf = ptr + 2;
			ctx->pending_comment = false;
		} else {
			/* Multiline comments */
			strip_spaces(&buf, true);
			while (buf[0] == '*')
				buf++;

			j2s_handle_comment(ctx, buf);
			return 0;
		}
	}

	/* 2. Handle new comments */
	ptr = strstr(buf, "/*");
	if (ptr) {
		/* Start of comment */
		char *start = ptr + 2;

		*ptr = '\0';
		ptr = strstr(start, "*/");
		if (ptr) {
			/* Inline comment */
			*ptr = '\0';
			ptr += 2;

			j2s_handle_comment(ctx, start);

			/* Remove inline comment */
			start = buf + strlen(buf);
			for (int i = 0; i < strlen(ptr) + 1; i++)
				start[i] = ptr[i];

			return j2s_parse(ctx, buf);
		} else {
			/* Start of multiline comments */
			ctx->pending_comment = true;
			j2s_handle_comment(ctx, start);
		}
	}

	ptr = strstr(buf, "//");
	if (ptr) {
		/* One line comment */
		*ptr = '\0';
		ptr += 2;

		j2s_handle_comment(ctx, ptr);
	}

	/* 3. Split buf */
	strip_spaces(&buf, true);
	ptr = strchr_first(buf, ';', '{');
	if (ptr) {
		ptr++;

		if (*ptr) {
			strcpy(pending, ptr);
			*ptr = '\0';
			strip_spaces(&buf, true);

			DBG("parsing '%s', pending '%s'\n",
			    buf, pending);
		}
	}

	/* 4. Filter out empty lines */
	if (buf[0] == '\0' || buf[0] == '#')
		goto out;

	/* 5. Handle enum definations */
	if (!parse_pattern(&buf, "typedef enum")) {
		DASSERT_MSG(!ctx->struct_obj && !ctx->enum_obj && \
			    !ctx->pending_enum && !ctx->pending_struct && \
			    !ctx->depth, exit(-1),
			    "failed to parse enum at %d\n", ctx->line);

		ctx->pending_enum = true;
	}

	if (ctx->pending_enum) {
		if (!strchr(buf, '{')) {
			DBG("waiting for '{', skip \"%s\"\n", buf);
			goto out;
		}

		ctx->depth++;

		/* Handle new enum */
		DBG("found enum\n");

		ctx->pending_enum = false;
		ctx->enum_obj = malloc(sizeof(*enum_obj));
		DASSERT(ctx->enum_obj, exit(-1));

		j2s_list_init(&ctx->enum_obj->values);
		ctx->enum_obj->value_id = -1;
		ctx->enum_obj->num_value = 0;
		goto out;
	}

	if (ctx->enum_obj) {
		j2s_enum_value *enum_value;

		if (!parse_pattern(&buf, "}")) {
			ctx->depth--;

			/* End of enum defination */
			strip_spaces(&buf, true);

			INFO("adding enum %s\n", buf);

			/* Add to enum list */
			strncpy(ctx->enum_obj->name, buf,
				sizeof(ctx->enum_obj->name));
			ctx->enum_obj->id = ctx->num_enum++;
			j2s_list_add(&ctx->enums, ctx->enum_obj);

			ctx->enum_obj = NULL;
			goto out;
		}

		/* Parsing enum value */

		/* The first word is enum value name */
		strip_spaces(&buf, true);
		name = buf;
		ptr = strchr_first(name, ' ', '=');
		if (ptr)
			*ptr = '\0';

		/* Add enum value */
		enum_value = malloc(sizeof(*enum_value));
		DASSERT(enum_value, exit(-1));

		enum_value->id = ctx->num_enum_value++;
		strncpy(enum_value->name, buf,
			sizeof(enum_value->name));
		j2s_list_add(&ctx->enum_obj->values, enum_value);

		ctx->enum_obj->num_value++;

		if (ctx->enum_obj->value_id < 0)
			ctx->enum_obj->value_id = enum_value->id;

		DBG("adding enum value %s\n", enum_value->name);
		goto out;
	}

	/* 6. Handle struct definations */
	if (!parse_pattern(&buf, "typedef struct")) {
		DASSERT_MSG(!ctx->depth && !ctx->struct_obj, exit(-1),
			    "failed to parse struct at %d\n", ctx->line);
		ctx->pending_struct = true;
	}

	/* Waiting for struct defination */
	if (ctx->depth) {
		if (!ctx->struct_obj) {
			if (strchr(buf, '}'))
				ctx->depth--;

			DBG("waiting for struct, skip \"%s\"\n", buf);
			goto out;
		}
	} else {
		if (!strchr(buf, '{')) {
			DBG("waiting for '{', skip \"%s\"\n", buf);
			goto out;
		}

		ctx->depth++;

		if (!ctx->pending_struct) {
			DBG("waiting for struct, skip \"%s\"\n", buf);
			goto out;
		}

		/* Handle new struct */
		DBG("found struct\n");

		ctx->pending_struct = false;
		ctx->struct_obj = malloc(sizeof(*struct_obj));
		DASSERT(ctx->struct_obj, exit(-1));

		j2s_list_init(&ctx->struct_obj->child);
		ctx->struct_obj->child_id = -1;
		goto out;
	}

	/* Skip union's { */
	if (!parse_pattern(&buf, "union {")) {
		ctx->depth++;
		goto out;
	}

	/* union's } or end of struct defination */
	if (!parse_pattern(&buf, "}")) {
		DASSERT_MSG(ctx->depth > 0, exit(-1),
			    "} not paired at %d\n", ctx->line);

		ctx->depth--;

		/* End of struct defination */
		if (!ctx->depth && ctx->struct_obj) {
			/* Skip packed attribute */
			parse_pattern(&buf, "__attribute__((packed))");

			/* Parse struct name */
			strip_spaces(&buf, true);
			name = buf;
			ptr = strrchr(name, ' ');
			if (ptr)
				name = ptr + 1;

			INFO("adding struct %s\n", name);

			/* Add to struct list */
			strncpy(ctx->struct_obj->name, name,
				sizeof(ctx->struct_obj->name));
			ctx->struct_obj->id = ctx->num_struct++;
			j2s_list_add(&ctx->structs, ctx->struct_obj);

			/* Using the last struct as root struct */
			ctx->root_struct = ctx->struct_obj;

			ctx->struct_obj = NULL;

			DASSERT_MSG(!ctx->desc,
				    free(ctx->desc); ctx->desc = NULL,
				    "unhandled desc: '%s'\n", ctx->desc);
		}
		goto out;
	}

	/* 7. Handle struct members */

	/* Handle special bool defination */
	if (!strcmp(buf, "_Bool")) {
		DASSERT_MSG(!ctx->pending_bool, exit(-1),
			    "failed to parse bool at %d\n", ctx->line);

		ctx->pending_bool = true;
		goto out;
	}

	flags = 0;

	/* Detect array and strip it */
	ptr = strrchr(buf, '[');
	if (ptr) {
		DBG("'%s' is an array\n", buf);

		*ptr = '\0';
		flags |= J2S_FLAG_ARRAY;

		/* Detect dep array */
		ptr = strrchr(buf, '[');
		if (ptr) {
			DBG("\tand a dep array %s\n", buf);

			*ptr = '\0';
			flags |= J2S_FLAG_DEP_ARRAY;

			DASSERT_MSG(!strrchr(buf, '['), return -1,
				    "array too dep at %d\n", ctx->line);
		}
	}

	/* Detect pointer */
	ptr = strrchr(buf, '*');
	if (ptr) {
		DBG("'%s' is a pointer\n", buf);

		*ptr = ' '; /* Would be stripped later */
		flags |= J2S_FLAG_POINTER;

		ptr = strrchr(buf, '*');
		if (ptr) {
			DBG("\tand a dep pointer %s\n", buf);

			*ptr = ' '; /* Would be stripped later */
			flags |= J2S_FLAG_DEP_POINTER;

			DASSERT_MSG(!strrchr(buf, '*'), return -1,
				    "pointer too dep at %d\n", ctx->line);
		}
	}

	/* Extract type and name */
	ptr = strrchr(buf, ' ');
	if (ctx->pending_bool) {
		DASSERT_MSG(!ptr, exit(-1),
			    "failed to parse bool at %d\n", ctx->line);
		ctx->pending_bool = false;
		type = "bool";
		name = buf;
	} else {
		DASSERT_MSG(ptr, exit(-1),
			    "parse member error from '%s' at %d\n",
			    buf, ctx->line);
		*ptr = '\0';
		type = buf;
		name = ptr + 1;
	}

	parse_pattern(&type, "const");
	strip_spaces(&type, true);
	strip_spaces(&name, true);

	/* Detect typedef array pointer */
	if (!parse_pattern(&type, "array_ptr_")) {
		DBG("'%s' is an array pointer\n", name);

		/* Remove member desc */
		if (type[0] <= '9' && type[0] >= '0')
			type = strchr(type, '_') + 1;

		flags |= J2S_FLAG_ARRAY_POINTER;
	}

	/* Detect typedef array */
	if (!parse_pattern(&type, "array_")) {
		DBG("'%s' is an array\n", name);

		DASSERT_MSG(!(flags & J2S_FLAG_DEP_ARRAY), return -1,
			    "array too dep at %d\n", ctx->line);
		DASSERT_MSG(!(flags & J2S_FLAG_DEP_POINTER), return -1,
			    "pointer too dep at %d\n", ctx->line);

		/* Remove member desc */
		if (type[0] <= '9' && type[0] >= '0')
			type = strchr(type, '_') + 1;

		if (flags & J2S_FLAG_ARRAY) {
			flags |= J2S_FLAG_DEP_ARRAY;
		} else if (flags & J2S_FLAG_POINTER) {
			flags |= J2S_FLAG_ARRAY_POINTER;

			/* Set it later */
			flags &= ~J2S_FLAG_POINTER;
		} else {
			flags |= J2S_FLAG_ARRAY;
		}
	}

	/* Check for dep type limits */
	if (flags & J2S_FLAG_ARRAY_POINTER) {
		DASSERT_MSG(!(flags & J2S_FLAG_ARRAY), return -1,
			    "array too dep at %d\n", ctx->line);
		DASSERT_MSG(!(flags & J2S_FLAG_POINTER), return -1,
			    "pointer too dep at %d\n", ctx->line);
		DASSERT_MSG(!(flags & J2S_FLAG_DEP_ARRAY), return -1,
			    "array too dep at %d\n", ctx->line);
		DASSERT_MSG(!(flags & J2S_FLAG_DEP_POINTER), return -1,
			    "pointer too dep at %d\n", ctx->line);

		flags |= J2S_FLAG_ARRAY;
		flags |= J2S_FLAG_POINTER;
	} else if (flags & J2S_FLAG_DEP_ARRAY) {
		DASSERT_MSG(!(flags & J2S_FLAG_POINTER), return -1,
			    "array too dep at %d\n", ctx->line);
	} else if (flags & J2S_FLAG_DEP_POINTER) {
		DASSERT_MSG(!(flags & J2S_FLAG_ARRAY), return -1,
			    "pointer too dep at %d\n", ctx->line);
	}

	/* Prepare member obj */
	obj = calloc(1, sizeof(*obj));
	DASSERT(obj, exit(-1));

	obj->id = ctx->num_obj++;
	obj->line = ctx->line;

	strncpy(obj->type_name, type, sizeof(obj->type_name));
	strncpy(obj->name, name, sizeof(obj->name));
	obj->flags = flags;
	obj->desc = ctx->desc;
	ctx->desc = NULL;

	/* Parse obj type and check type limits */
	obj->type = j2s_parse_type(type, flags);

	static bool type_warn_once = false;
	if (!type_warn_once &&
	    (obj->type == J2S_TYPE_INT_64 ||
	     obj->type == J2S_TYPE_UINT_64)) {
		type_warn_once = true;

		WARN("64 bit type might have precision issue!\n");
	}

	if (obj->type != J2S_TYPE_STRING) {
		DASSERT_MSG(!(flags & J2S_FLAG_DEP_POINTER), return -1,
			    "dep pointer only for string at %d\n", ctx->line);

		if (flags & J2S_FLAG_ARRAY &&
		    flags & J2S_FLAG_POINTER)
			DASSERT_MSG(flags & J2S_FLAG_ARRAY_POINTER,
				    return -1,
				    "pointer array only for string at %d\n",
				    ctx->line);
	}

	obj->parent = ctx->struct_obj;

	j2s_list_add(&ctx->objs, obj);

	/* Link with prev child */
	if (ctx->struct_obj->child_id < 0) {
		ctx->struct_obj->child_id = obj->id;
	} else {
		j2s_obj *tmp_obj =
		((j2s_list *)ctx->struct_obj->child.prev)->data;
		tmp_obj->next = obj;
	}

	j2s_list_add(&ctx->struct_obj->child, obj);

	if (obj->desc) {
		DBG("adding member %s with desc: %s\n",
		    obj->name, obj->desc);
	} else {
		DBG("adding member %s\n", obj->name);
	}

out:
	if (pending[0])
		return j2s_parse(ctx, pending);

	return 0;
}

/* Dump parsed header */
static inline
void j2s_dump(j2s_ctx *ctx) {
	j2s_list *entry, *next;
	j2s_enum_value *enum_value;
	j2s_struct *struct_obj;
	j2s_enum *enum_obj;
	j2s_obj *obj;
	int magic;

	srand(time(NULL));
	magic = rand();

	printf("#include <stdint.h>\n");
	printf("#include \"j2s.h\"\n\n");

	printf("#define J2S_MAGIC %d\n", magic);
	printf("#define J2S_NUM_OBJ %d\n", ctx->num_obj);
	printf("#define J2S_NUM_STRUCT %d\n", ctx->num_struct);
	printf("#define J2S_NUM_ENUM %d\n", ctx->num_enum);
	printf("#define J2S_NUM_ENUM_VALUE %d\n\n", ctx->num_enum_value);

	printf("static j2s_obj objs[J2S_NUM_OBJ];\n");
	printf("static j2s_struct structs[J2S_NUM_STRUCT];\n");
	printf("static j2s_enum enums[J2S_NUM_ENUM];\n");
	printf("static j2s_enum_value enum_values[J2S_NUM_ENUM_VALUE];\n\n");

	printf("static void _j2s_init(j2s_ctx *ctx) {\n");
	printf("\tj2s_obj *obj;\n\n");

	printf("\tctx->magic = J2S_MAGIC;\n");
	printf("\tctx->priv = NULL;\n");

	printf("\tctx->objs = (j2s_obj *)&objs;\n");
	printf("\tctx->structs = (j2s_struct *)&structs;\n");
	printf("\tctx->enums = (j2s_enum *)&enums;\n");
	printf("\tctx->enum_values = (j2s_enum_value *)&enum_values;\n\n");

	printf("\tctx->num_obj = J2S_NUM_OBJ;\n");
	printf("\tctx->num_struct = J2S_NUM_STRUCT;\n");
	printf("\tctx->num_enum = J2S_NUM_ENUM;\n");
	printf("\tctx->num_enum_value = J2S_NUM_ENUM_VALUE;\n\n");

	printf("\n#ifndef J2S_ENABLE_DESC\n");
	printf("\tctx->num_desc = 0;\n");
	printf("#else\n");
	printf("\tstatic const char *descs[J2S_NUM_OBJ];\n");
	printf("\tctx->descs = (const char **)descs;\n");
	printf("\tctx->num_desc = J2S_NUM_OBJ;\n");
	printf("#endif\n\n");

	struct_obj = ctx->root_struct;
	INFO("root struct: %s\n", struct_obj->name);

	printf("\tctx->root_index = %d;\n\n", struct_obj->id);

	/* Dump obj list */
	j2s_list_walk_safe(&ctx->objs, entry, next, obj) {
		printf("\tobj = &ctx->objs[%d];\n", obj->id);

		printf("\tstrcpy(obj->name, \"%s\");\n", obj->name);
		printf("\tobj->type = %s;\n", j2s_types[obj->type]);

#define DUMP_FLAG(flags, FLAG) \
		if (flags & FLAG) printf("| "#FLAG)

		printf("\tobj->flags = 0 ");
		DUMP_FLAG(obj->flags, J2S_FLAG_ARRAY);
		DUMP_FLAG(obj->flags, J2S_FLAG_POINTER);
		DUMP_FLAG(obj->flags, J2S_FLAG_DEP_ARRAY);
		DUMP_FLAG(obj->flags, J2S_FLAG_DEP_POINTER);
		DUMP_FLAG(obj->flags, J2S_FLAG_ARRAY_POINTER);
		printf(";\n");

#define MEMBER_DEP_0	"((%s*)0)->%s"
#define MEMBER_DEP_1	MEMBER_DEP_0 "[0]"
#define MEMBER_DEP_2	MEMBER_DEP_1 "[0]"

		printf("\tobj->offset = (uintptr_t)&" MEMBER_DEP_0 ";\n",
		       obj->parent->name, obj->name);

		if (obj->flags & (J2S_FLAG_ARRAY | J2S_FLAG_POINTER)) {
			printf("\tobj->elem_size = sizeof(" MEMBER_DEP_1 ");\n",
			       obj->parent->name, obj->name);

		} else {
			printf("\tobj->elem_size = sizeof(" MEMBER_DEP_0 ");\n",
			       obj->parent->name, obj->name);
		}

		if ((obj->flags & J2S_FLAG_ARRAY &&
		     obj->flags & J2S_FLAG_POINTER) ||
		    obj->flags & J2S_FLAG_DEP_ARRAY ||
		    obj->flags & J2S_FLAG_DEP_POINTER) {
			printf("\tobj->base_elem_size = "
			       "sizeof(" MEMBER_DEP_2 ");\n",
			       obj->parent->name, obj->name);
		} else {
			printf("\tobj->base_elem_size = obj->elem_size;\n");
		}

		if (J2S_IS_ARRAY(obj)) {
			printf("\tobj->num_elem = (sizeof(" MEMBER_DEP_0 ") / "
			       "obj->elem_size);\n",
			       obj->parent->name, obj->name);
		} else {
			printf("\tobj->num_elem = 1;\n");
		}

		/* Dynamic array needs a "<name>_len" sibling */
		if ((obj->type != J2S_TYPE_STRING &&
		     obj->flags & J2S_FLAG_POINTER) ||
		    (obj->type == J2S_TYPE_STRING &&
		     (obj->flags & J2S_FLAG_DEP_POINTER ||
		      obj->flags & J2S_FLAG_ARRAY_POINTER))) {
			j2s_obj *len_obj;
			char len_name[MAX_NAME + 5];
			snprintf(len_name, MAX_NAME + 5, "%s_len", obj->name);

			j2s_list_find(&obj->parent->child, entry,
				      len_obj, len_name);

			DASSERT_MSG(len_obj, exit(-1),
				    "missing %s\n", len_name);
			DASSERT_MSG(len_obj->type <= J2S_TYPE_UINT_64 &&
				    !len_obj->flags,
				    exit(-1), "%s should be int\n", len_name);

			obj->len_obj = len_obj;
		}

		printf("\tobj->len_index = %d;\n",
		       obj->len_obj ? ((j2s_obj *)obj->len_obj)->id : -1);
		printf("\tobj->next_index = %d;\n",
		       obj->next ? ((j2s_obj *)obj->next)->id : -1);
		printf("\tobj->struct_index = %d;\n",
		       obj->struct_obj ? obj->struct_obj->id : -1);
		printf("\tobj->enum_index = %d;\n",
		       obj->enum_obj ? obj->enum_obj->id : -1);

		if (obj->desc) {
			char *ptr;

			while (ptr = strchr(obj->desc, '"'))
				*ptr = '\'';

			/* Check default value for enum in desc */
			if (obj->enum_obj) {
				ptr = strstr(obj->desc, "default=");
				if (ptr) {
					char buf[MAX_LINE];
					char *name = buf;

					strcpy(name, ptr + strlen("default="));
					ptr = strchr_first(name, ' ', ',');
					if (ptr)
						*ptr = '\0';

					strip_spaces(&name, true);

					j2s_list_find(&obj->enum_obj->values,
						      entry, enum_value, name);

					DASSERT_MSG(enum_value, exit(-1),
						    "invalid default enum "
						    "%s for %s\n",
						    name, obj->name);
				}
			}

			printf("#ifdef J2S_ENABLE_DESC\n");
			printf("\tctx->descs[%d] = \"%s\";\n",
			       obj->id, obj->desc);
			printf("#endif\n");
		}

		printf("\n");
	}

	/* Dump struct list */
	j2s_list_walk_safe(&ctx->structs, entry, next, struct_obj) {
		printf("\tstrcpy(ctx->structs[%d].name, \"%s\");\n",
		       struct_obj->id, struct_obj->name);
		printf("\tctx->structs[%d].child_index = %d;\n",
		       struct_obj->id, struct_obj->child_id);
	}

	/* Dump enum list */
	j2s_list_walk_safe(&ctx->enums, entry, next, enum_obj) {
		printf("\n\tstrcpy(ctx->enums[%d].name, \"%s\");\n",
		       enum_obj->id, enum_obj->name);
		printf("\tctx->enums[%d].value_index = %d;\n",
		       enum_obj->id, enum_obj->value_id);
		printf("\tctx->enums[%d].num_value = %d;\n\n",
		       enum_obj->id, enum_obj->num_value);

		j2s_list_walk(&enum_obj->values, entry, enum_value) {
			printf("\tstrcpy(ctx->enum_values[%d].name, \"%s\");\n",
			       enum_value->id, enum_value->name);
			printf("\tctx->enum_values[%d].value = %s;\n",
			       enum_value->id, enum_value->name);
		}
	}

	printf("}\n");
}

int main(int argc, char** argv) {
	j2s_list *entry, *next;
	j2s_struct *struct_obj;
	j2s_enum *enum_obj;
	j2s_obj *obj;
	j2s_ctx ctx = {0};
	char buf[MAX_LINE];

	j2s_list_init(&ctx.objs);
	j2s_list_init(&ctx.structs);
	j2s_list_init(&ctx.enums);

	DASSERT_MSG(argc > 1, return -1,
		    "usage: %s <input header> [root struct]\n", argv[0]);

	INFO("start parsing %s\n", argv[1]);
	ctx.fp = fopen(argv[1],"r");
	DASSERT_MSG(ctx.fp, return -1, "failed to open %s\n", argv[1]);

	while (fgets(buf, MAX_LINE, ctx.fp)) {
		ctx.line++;
		j2s_parse(&ctx, buf);
	}

	INFO("finished parsing %s\n", argv[1]);
	fclose(ctx.fp);

	/* Set objs' struct/enum obj and check unsupported type */
	j2s_list_walk_safe(&ctx.objs, entry, next, obj) {
		if (obj->type != J2S_TYPE_STRUCT)
			continue;

		j2s_list_find(&ctx.structs, entry, struct_obj, obj->type_name);

		if (struct_obj) {
			obj->struct_obj = struct_obj;
		} else {
			j2s_list_find(&ctx.enums, entry,
				      enum_obj, obj->type_name);

			DASSERT_MSG(enum_obj, exit(-1),
				    "unknown type '%s' at %d\n",
				    obj->type_name, obj->line);

			obj->type = J2S_TYPE_INT_32;
			obj->enum_obj = enum_obj;
		}
	}

	/* Handle specified root struct */
	if (argc > 2) {
		INFO("specified root struct: %s\n", argv[2]);

		j2s_list_find(&ctx.structs, entry, struct_obj, argv[2]);
		ctx.root_struct = struct_obj;
	}

	DASSERT_MSG(ctx.root_struct, exit(-1), "no root struct!\n");

	/* Dump parsed header */
	j2s_dump(&ctx);

	/* Free lists */
	j2s_list_walk_safe(&ctx.structs, entry, next, struct_obj)
		j2s_list_free(&struct_obj->child, entry, false);
	j2s_list_free(&ctx.structs, entry, true);

	j2s_list_walk(&ctx.objs, entry, obj)
		free(obj->desc);
	j2s_list_free(&ctx.objs, entry, true);

	j2s_list_walk_safe(&ctx.enums, entry, next, enum_obj)
		j2s_list_free(&enum_obj->values, entry, true);
	j2s_list_free(&ctx.enums, entry, true);

	return 0;
}
