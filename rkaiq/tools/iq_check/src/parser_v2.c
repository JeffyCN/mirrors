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

#include ".j2s_generated.h"
#include "j2s.h"

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

static int j2s_find_enum_index(j2s_ctx *ctx, const char *name) {
  if (!name)
    return -1;

  for (int i = 0; i < ctx->num_enum; i++) {
    j2s_enum *enum_obj = &ctx->enums[i];
    if (!strcmp(enum_obj->name, name))
      return i;
  }

  return -1;
}

static int j2s_find_struct_index(j2s_ctx *ctx, const char *name) {
  if (!name)
    return -1;

  for (int i = 0; i < ctx->num_struct; i++) {
    j2s_struct *struct_obj = &ctx->structs[i];
    if (!strcmp(struct_obj->name, name))
      return i;
  }

  return -1;
}

int _j2s_dump_to_new_ctx_enum_value(j2s_ctx *octx, j2s_ctx *nctx, int enum_index, int num_val)
{
    j2s_enum_value *enum_value;
    j2s_enum_value *new_enum_value;
    int new_enum_value_idx;

    new_enum_value_idx = nctx->num_enum_value;
    nctx->num_enum_value += num_val;
    if (!nctx->enums) {
        nctx->enum_values = (j2s_enum_value*)calloc(1, sizeof(j2s_enum_value));
    } else {
        nctx->enum_values = (j2s_enum_value*)realloc(nctx->enum_values, sizeof(j2s_enum_value) * nctx->num_enum_value);
    }

    for (int i = 0; i < num_val; i++) {
        enum_value = &octx->enum_values[enum_index + i];
        new_enum_value = &nctx->enum_values[new_enum_value_idx + i];
        strcpy(new_enum_value->name, enum_value->name);
        new_enum_value->value = enum_value->value;
    }

    return new_enum_value_idx;
}

int _j2s_dump_to_new_ctx_enum(j2s_ctx *octx, j2s_ctx *nctx, int enum_index)
{
    j2s_enum *enum_obj;
    j2s_enum *new_enum_obj;
    int new_enum_idx;

    enum_obj = &octx->enums[enum_index];
    new_enum_idx = nctx->num_enum;
    nctx->num_enum++;
    if (!nctx->enums) {
        nctx->enums = (j2s_enum*)calloc(1, sizeof(j2s_enum));
    } else {
        nctx->enums = (j2s_enum*)realloc(nctx->enums, sizeof(j2s_enum) * nctx->num_enum);
    }
    new_enum_obj = &nctx->enums[new_enum_idx];

    strcpy(new_enum_obj->name, enum_obj->name);
    DBG("enum %d %s, old %d %s\n", new_enum_idx, new_enum_obj->name, enum_index, enum_obj->name);
    new_enum_obj->num_value = enum_obj->num_value;
    new_enum_obj->value_index =
        _j2s_dump_to_new_ctx_enum_value(octx, nctx, enum_obj->value_index, enum_obj->num_value);

    return new_enum_idx;
}

int _j2s_dump_to_new_ctx_desc(j2s_ctx *octx, j2s_ctx *nctx, int desc_index)
{
    const char *desc = octx->descs[desc_index];
    int new_desc_idx;

    new_desc_idx = nctx->num_desc;
    nctx->num_desc++;

    if (!nctx->descs) {
        nctx->descs = (const char **)calloc(1, sizeof(const char *));
    } else {
        nctx->descs = (const char**)realloc(nctx->descs, sizeof(const char *) * nctx->num_desc);
    }

    nctx->descs[new_desc_idx] = desc;

    return 0;
}

int _j2s_dump_to_new_ctx_struct(j2s_ctx *octx, j2s_ctx *nctx, int struct_index);

int _j2s_dump_to_new_ctx_obj(j2s_ctx *octx, j2s_ctx *nctx, int obj_index)
{
    j2s_obj *obj;
    j2s_obj *n_obj;
    int new_child_idx;

    obj = &octx->objs[obj_index];

    new_child_idx = nctx->num_obj;
    nctx->num_obj++;
    if (!nctx->objs) {
        nctx->objs = (j2s_obj*)calloc(1, sizeof(j2s_obj));
    } else {
        nctx->objs = (j2s_obj*)realloc(nctx->objs, sizeof(j2s_obj) * nctx->num_obj);
    }
    n_obj = &nctx->objs[new_child_idx];

    strcpy(n_obj->name, obj->name);
    n_obj->type = obj->type;
    n_obj->flags = obj->flags;
    n_obj->offset = obj->offset;
    n_obj->elem_size = obj->elem_size;
    n_obj->base_elem_size = obj->base_elem_size;
    n_obj->elem_size = obj->elem_size;

    if (octx->descs)
        _j2s_dump_to_new_ctx_desc(octx, nctx, obj_index);

    DBG("obj name %s begin %p\n", n_obj->name, n_obj);

    if (obj->enum_index >= 0) {
        int enum_index = j2s_find_enum_index(nctx, octx->enums[obj->enum_index].name);
        if (enum_index < 0) {
            n_obj->enum_index = _j2s_dump_to_new_ctx_enum(octx, nctx, obj->enum_index);
        } else {
            n_obj->enum_index = enum_index;
        }
    } else {
        n_obj->enum_index = -1;
    }

    if (obj->type == J2S_TYPE_STRUCT) {
        int struct_idx = j2s_find_struct_index(nctx, octx->structs[obj->struct_index].name);
        if (struct_idx < 0) {
            struct_idx = _j2s_dump_to_new_ctx_struct(octx, nctx, obj->struct_index);
        }
        n_obj = &nctx->objs[new_child_idx];
        n_obj->struct_index = struct_idx;
        DBG("obj name %s struct idx %d\n", n_obj->name, n_obj->struct_index);
    } else {
        n_obj->struct_index = -1;
    }

    if (obj->len_index >= 0) {
        n_obj->len_index = nctx->num_obj;
    } else {
        n_obj->len_index = -1;
    }

    DBG("obj name %s end %p\n", n_obj->name, n_obj);

    return new_child_idx;
}

int _j2s_dump_to_new_ctx_struct(j2s_ctx *octx, j2s_ctx *nctx, int struct_index)
{

    j2s_obj* child;
    j2s_obj* n_child;
    j2s_struct* struct_obj;
    int child_index;
    int new_child_index;
    int new_struct_idx;

    struct_obj = &octx->structs[struct_index];
    child_index = struct_obj->child_index;
    new_struct_idx = nctx->num_struct;
    nctx->num_struct++;
    if (!nctx->structs) {
        nctx->structs = (j2s_struct*)calloc(1, sizeof(j2s_struct));
    } else {
        nctx->structs = (j2s_struct*)realloc(nctx->structs, sizeof(j2s_struct) * nctx->num_struct);
    }
    strcpy(nctx->structs[new_struct_idx].name, struct_obj->name);
    if (child_index >= 0) {
        nctx->structs[new_struct_idx].child_index = nctx->num_obj;
    } else {
        nctx->structs[new_struct_idx].child_index = -1;
    }
    DBG("idx %d, name %s, child_idx %d\n", new_struct_idx, nctx->structs[new_struct_idx].name, nctx->structs[new_struct_idx].child_index);
    DBG("old idx %d, name %s, child_idx %d\n", struct_index, struct_obj->name, struct_obj->child_index);

    for(; child_index >= 0; child_index = child->next_index) {
        child = &octx->objs[child_index];
        new_child_index = _j2s_dump_to_new_ctx_obj(octx, nctx, child_index);
        if (child->next_index >= 0)
            nctx->objs[new_child_index].next_index = nctx->num_obj;
        else
            nctx->objs[new_child_index].next_index = -1;
    }

    return new_struct_idx;

}

int j2s_dump_to_new_ctx(j2s_ctx *octx, j2s_ctx *nctx, const char* name)
{
    int struct_idx = name ? j2s_find_struct_index(octx, name) : octx->root_index;
    return _j2s_dump_to_new_ctx_struct(octx, nctx, struct_idx);

}

void j2s_dump_obj_to_file(j2s_ctx *ctx, int struct_idx, int *idx_pr)
{
    j2s_obj *obj;
    j2s_struct *struct_obj;
    int child_index;
    char tmp_buf[4096];
    char desc_buf[4096];
    char *tmp_ptr;

    struct_obj = &ctx->structs[struct_idx];
    child_index = struct_obj->child_index;
    DBG("name %s %d %d\n", struct_obj->name, struct_idx, child_index);

    for (; child_index >= 0; child_index = obj->next_index) {
        obj = &ctx->objs[child_index];
        if (child_index < *idx_pr)
            continue;

        printf("\tobj = &ctx->objs[%d];\n", child_index);

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
		       struct_obj->name, obj->name);

		if (obj->flags & (J2S_FLAG_ARRAY | J2S_FLAG_POINTER)) {
			printf("\tobj->elem_size = sizeof(" MEMBER_DEP_1 ");\n",
			       struct_obj->name, obj->name);

		} else {
			printf("\tobj->elem_size = sizeof(" MEMBER_DEP_0 ");\n",
			       struct_obj->name, obj->name);
		}

        if ((obj->flags & J2S_FLAG_ARRAY &&
		     obj->flags & J2S_FLAG_POINTER) ||
		    obj->flags & J2S_FLAG_DEP_ARRAY ||
		    obj->flags & J2S_FLAG_DEP_POINTER) {
			printf("\tobj->base_elem_size = "
			       "sizeof(" MEMBER_DEP_2 ");\n",
			       struct_obj->name, obj->name);
		} else {
			printf("\tobj->base_elem_size = obj->elem_size;\n");
		}

        if (J2S_IS_ARRAY(obj)) {
			printf("\tobj->num_elem = (sizeof(" MEMBER_DEP_0 ") / "
			       "obj->elem_size);\n",
			       struct_obj->name, obj->name);
		} else {
			printf("\tobj->num_elem = 1;\n");
		}

        printf("\tobj->len_index = %d;\n", obj->len_index);
		printf("\tobj->next_index = %d;\n", obj->next_index);
		printf("\tobj->struct_index = %d;\n", obj->struct_index);
		printf("\tobj->enum_index = %d;\n", obj->enum_index);

        if (ctx->descs && ctx->descs[child_index]) {
            printf("#ifdef J2S_ENABLE_DESC\n");
            printf("\tctx->descs[%d] = \"", child_index);
            if (strlen(ctx->descs[child_index]) > 4095) {
                ERR(".j2s_generated.h descs[%d] strlen %zu more large then 4096, maybe cause some error\n%s\n",
                    child_index, strlen(ctx->descs[child_index]), ctx->descs[child_index]);
            }
            snprintf(desc_buf, 4096, "%s", ctx->descs[child_index]);
            tmp_ptr = strtok(desc_buf, "\n");
            while (tmp_ptr != NULL) {
                printf("%s", tmp_ptr);
                tmp_ptr = strtok(NULL, "\n");
                if (tmp_ptr != NULL)
                    printf("\\n");
            }
            printf("\";\n");
            printf("#endif\n");
        }

        printf("\n");

        *idx_pr = child_index;

        if (obj->type == J2S_TYPE_STRUCT)
            j2s_dump_obj_to_file(ctx, obj->struct_index, idx_pr);
    }
}

void j2s_dump_to_file(j2s_ctx *ctx)
{
    j2s_obj *obj;
    j2s_struct *struct_obj;
    int already_print_idx = -1;
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

    printf("\tctx->root_index = %d;\n\n", ctx->root_index);

    j2s_dump_obj_to_file(ctx, ctx->root_index, &already_print_idx);
    int tuning_idx = j2s_find_struct_index(ctx, "CamCalibDbV2Tuning_t");
    INFO("tunning idx %d", tuning_idx);
    j2s_dump_obj_to_file(ctx, tuning_idx, &already_print_idx);

    for (int i = 0; i < ctx->num_struct; i++) {
        printf("\tstrcpy(ctx->structs[%d].name, \"%s\");\n",
		       i, ctx->structs[i].name);
		printf("\tctx->structs[%d].child_index = %d;\n",
		       i, ctx->structs[i].child_index);
    }
    printf("\n");

    for (int i = 0; i < ctx->num_enum; i++) {
        printf("\n\tstrcpy(ctx->enums[%d].name, \"%s\");\n",
		       i, ctx->enums[i].name);
		printf("\tctx->enums[%d].value_index = %d;\n",
		       i, ctx->enums[i].value_index);
		printf("\tctx->enums[%d].num_value = %d;\n\n",
		       i, ctx->enums[i].num_value);

        for (int j = ctx->enums[i].value_index; j < ctx->enums[i].value_index + ctx->enums[i].num_value; j++) {
            printf("\tstrcpy(ctx->enum_values[%d].name, \"%s\");\n",
			       j, ctx->enum_values[j].name);
			printf("\tctx->enum_values[%d].value = %s;\n",
			       j, ctx->enum_values[j].name);
        }
    }

    printf("}\n");

}

int main()
{
    j2s_ctx ctx_old;
    j2s_ctx ctx_new;
    memset(&ctx_old, 0, sizeof(j2s_ctx));
    memset(&ctx_new, 0, sizeof(j2s_ctx));
    const char *name = "CalibDb_Sensor_ParaV2_t";

    _j2s_init(&ctx_old);

    j2s_dump_to_new_ctx(&ctx_old, &ctx_new, NULL);
    j2s_dump_to_new_ctx(&ctx_old, &ctx_new, "CamCalibDbV2Tuning_t");
    ctx_new.root_index = j2s_find_struct_index(&ctx_new, ctx_old.structs[ctx_old.root_index].name);

    INFO("ctx_old root index %d\n", ctx_old.root_index);
    INFO("ctx_new root index %d\n", ctx_new.root_index);

    j2s_dump_to_file(&ctx_new);

    if (ctx_new.structs) {
        free(ctx_new.structs);
        ctx_new.structs = NULL;
    }

    if (ctx_new.objs) {
        free(ctx_new.objs);
        ctx_new.objs = NULL;
    }

    if (ctx_new.enums) {
        free(ctx_new.enums);
        ctx_new.enums = NULL;
    }

    return 0;
}

