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

#include "j2s.h"

static inline
void dump_intent(int depth) {
	for (int i = 0; i < depth; i++)
		printf("\t");
}

static
void dump_struct(j2s_ctx *ctx, int struct_index, bool expend, int depth);

static
void dump_obj(j2s_ctx *ctx, int obj_index, bool expend, int depth) {
	j2s_obj *obj;
	const char *type;

	if (obj_index < 0)
		return;

	obj = &ctx->objs[obj_index];
	if (obj->type == J2S_TYPE_STRUCT && expend) {
		j2s_struct *struct_obj = &ctx->structs[obj->struct_index];

		dump_intent(depth);
		printf("// %s %s", struct_obj->name, obj->name);

		printf("; // id: %d|off: %d|", obj_index, obj->offset);

		if (J2S_IS_POINTER(obj)) {
			printf("size: %d", (int)sizeof(void *));
		} else if (J2S_IS_ARRAY(obj)) {
			printf("size: %d*%d", obj->elem_size, obj->num_elem);
		} else {
			printf("size: %d", obj->elem_size);
		}

		if (obj->len_index >= 0)
			printf("|len: @%s", ctx->objs[obj->len_index].name);

		printf("\n");

		dump_struct(ctx, obj->struct_index, expend, depth);

		dump_obj(ctx, obj->next_index, expend, depth);
		return;
	}

	if (obj->type == J2S_TYPE_STRUCT) {
		j2s_struct *struct_obj = &ctx->structs[obj->struct_index];
		type = struct_obj->name;
	} else if (obj->enum_index >= 0) {
		j2s_enum *enum_obj = &ctx->enums[obj->enum_index];
		type = enum_obj->name;
	} else {
		type = j2s_type_name((j2s_type)obj->type);
	}

	dump_intent(depth);

	if (obj->flags & J2S_FLAG_ARRAY_POINTER)
		printf("array_ptr_%d_", obj->elem_size / obj->base_elem_size);

	printf("%s ", type);

	if (!(obj->flags & J2S_FLAG_ARRAY_POINTER) &&
	    obj->flags & J2S_FLAG_POINTER) {
		printf("*");

		if (obj->flags & J2S_FLAG_DEP_POINTER)
			printf("*");
	}

	printf("%s", obj->name);

	if (J2S_IS_ARRAY(obj)) {
		printf("[%d]", obj->num_elem);

		if (obj->flags & J2S_FLAG_DEP_ARRAY)
			printf("[%d]", obj->elem_size / obj->base_elem_size);
	}

	printf("; // id: %d|off: %d|", obj_index, obj->offset);

	if (J2S_IS_POINTER(obj)) {
		printf("size: %d", (int)sizeof(void *));
	} else if (J2S_IS_ARRAY(obj)) {
		printf("size: %d*%d", obj->elem_size, obj->num_elem);
	} else {
		printf("size: %d", obj->elem_size);
	}

	if (obj->len_index >= 0)
		printf("|len: @%s", ctx->objs[obj->len_index].name);

	printf("\n");

	if (obj->next_index < 0)
		return;

	dump_obj(ctx, obj->next_index, expend, depth);
}

static
void dump_struct(j2s_ctx *ctx, int struct_index, bool expend, int depth) {
	j2s_struct *struct_obj;

	if (struct_index < 0)
		return;

	struct_obj = &ctx->structs[struct_index];

	dump_intent(depth);

	if (!expend)
		printf("typedef struct {\n");
	else
		printf("{ // %s\n", struct_obj->name);

	dump_obj(ctx, struct_obj->child_index, expend, depth + 1);

	dump_intent(depth);

	if (!expend)
		printf("} __attribute__((packed)) %s\n", struct_obj->name);
	else
		printf("}\n");
}

static
void dump_enum(j2s_ctx *ctx, int enum_index) {
	j2s_enum *enum_obj;
	j2s_enum_value *enum_value;

	if (enum_index < 0)
		return;

	enum_obj = &ctx->enums[enum_index];

	printf("typedef enum {\n");
	for (int i = 0; i < enum_obj->num_value; i++) {
		enum_value =
			&ctx->enum_values[enum_obj->value_index + i];

		printf("\t%s = %d;\n", enum_value->name, enum_value->value);
	}
	printf("} %s;\n", enum_obj->name);
}

int main(int argc, char** argv) {
	j2s_struct *root_struct;
	j2s_ctx ctx = {0};
	char *buf;
	bool template_ = false;
	bool dump_desc = true;
	bool format = true;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--template")) {
			template_ = true;
		} else if (!strcmp(argv[i], "--nodesc")) {
			dump_desc = false;
		} else if (!strcmp(argv[i], "--unformat")) {
			format = false;
		} else {
			ERR("unknown arg: %s\n", argv[i]);
			return -1;
		}
	}

	j2s_init(&ctx);

	ctx.format_json = format;
	ctx.dump_desc = dump_desc;

	if (template_) {
		INFO("Dump template JSON\n");

		ctx.dump_enums = true;
		buf = j2s_dump_template_root_struct(&ctx);
		DASSERT(buf, return -1);

		printf("%s\n", buf);

		free(buf);

		j2s_deinit(&ctx);
		return 0;
	}

	INFO("\nDumping structs:\n");
	for (int i = 0; i < ctx.num_struct; i++) {
		dump_struct(&ctx, i, false, 0);
		printf("\n");
	}

	INFO("\nDumping enums:\n");
	for (int i = 0; i < ctx.num_enum; i++) {
		dump_enum(&ctx, i);
		printf("\n");
	}

	root_struct = &ctx.structs[ctx.root_index];
	INFO("\nDumping root_struct: %s\n", root_struct->name);

	dump_struct(&ctx, ctx.root_index, true, 0);

	j2s_deinit(&ctx);
	return 0;
}
