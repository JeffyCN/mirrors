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
#include ".j2s_generated.h"

int main(int argc, char** argv) {

	FILE* fp = NULL;
	// int magic = 0;
	j2s_struct *struct_obj = NULL;

	DASSERT_MSG(argc > 1, return -1,
		    "usage: %s <j2s_generated.h> \n", argv[0]);

	INFO("optimize j2s_generated.h to file: %s\n", argv[1]);

	j2s_ctx ctx_rel;
	j2s_ctx *ctx = &ctx_rel;
	_j2s_init(ctx);

	fp = fopen(argv[1],"w+");
	DASSERT_MSG(fp, return -1, "failed to open %s\n", argv[1]);

	fprintf(fp, "#include <stdint.h>\n");
	fprintf(fp, "#include \"j2s.h\"\n\n");
	fprintf(fp, "#include \"j2s_code2bin.h\"\n\n");

	fprintf(fp, "#define J2S_MAGIC %d\n", ctx->magic);
	fprintf(fp, "#define J2S_NUM_OBJ %d\n", ctx->num_obj);
	fprintf(fp, "#define J2S_NUM_STRUCT %d\n", ctx->num_struct);
	fprintf(fp, "#define J2S_NUM_ENUM %d\n", ctx->num_enum);
	fprintf(fp, "#define J2S_NUM_ENUM_VALUE %d\n\n", ctx->num_enum_value);
	fprintf(fp, "static void _j2s_init(j2s_ctx *ctx) {\n");
	fprintf(fp, "\tsize_t objs_len = 0;\n");
	fprintf(fp, "\tsize_t structs_len = 0;\n");
	fprintf(fp, "\tsize_t enums_len = 0;\n\n");

	fprintf(fp, "\tctx->magic = J2S_MAGIC;\n");
	fprintf(fp, "\tctx->priv = NULL;\n");

	fprintf(fp, "\tctx->objs = (j2s_obj *)(&j2s_code2bin_bin[0]);\n");
	fprintf(fp, "\tobjs_len = sizeof(j2s_obj) * J2S_NUM_OBJ;\n");
	fprintf(fp, "\tctx->structs = (j2s_struct *)(&j2s_code2bin_bin[0] + objs_len);\n");
	fprintf(fp, "\tstructs_len = sizeof(j2s_struct) * J2S_NUM_STRUCT;\n");
	fprintf(fp, "\tctx->enums = (j2s_enum *)(&j2s_code2bin_bin[0] + objs_len + structs_len);\n");
	fprintf(fp, "\tenums_len = sizeof(j2s_enum) * J2S_NUM_ENUM;\n");
	fprintf(fp, "\tctx->enum_values = (j2s_enum_value *)(&j2s_code2bin_bin[0] + objs_len + structs_len + enums_len);\n\n");

	fprintf(fp, "\tctx->num_obj = J2S_NUM_OBJ;\n");
	fprintf(fp, "\tctx->num_struct = J2S_NUM_STRUCT;\n");
	fprintf(fp, "\tctx->num_enum = J2S_NUM_ENUM;\n");
	fprintf(fp, "\tctx->num_enum_value = J2S_NUM_ENUM_VALUE;\n\n");

	fprintf(fp, "\tctx->num_desc = 0;\n");

	fprintf(fp, "\tctx->root_index = %d;\n", ctx->root_index);
	fprintf(fp, "}\n\n");

	fclose(fp);

	return 0;
}
