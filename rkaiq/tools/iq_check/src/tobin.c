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
#include ".j2s_generated.h"

int main(int argc, char** argv) {

	FILE* fp = NULL;
	size_t total = 0;
	size_t wr_size = 0;

	DASSERT_MSG(argc > 1, return -1,
		    "usage: %s <input header> [root struct]\n", argv[0]);

	INFO("j2s2bin to file: %s\n", argv[1]);

	j2s_ctx ctx;
	_j2s_init(&ctx);

	fp = fopen(argv[1],"wb+");
	DASSERT_MSG(fp, return -1, "failed to open %s\n", argv[1]);

	wr_size = fwrite(objs,sizeof(objs), 1, fp);
	total = wr_size * sizeof(objs);
	printf("write objs size: %zu, expected: %zu\n", wr_size * sizeof(objs), sizeof(objs));

	wr_size = fwrite(structs,sizeof(structs), 1, fp);
	total += wr_size * sizeof(structs);
	printf("write structs size: %zu, expected: %zu\n", wr_size * sizeof(structs), sizeof(structs));

	wr_size = fwrite(enums,sizeof(enums), 1, fp);
	total += wr_size * sizeof(enums);
	printf("write enums size: %zu, expected: %zu\n", wr_size * sizeof(enums), sizeof(enums));

	wr_size = fwrite(enum_values,sizeof(enum_values), 1, fp);
	total += wr_size * sizeof(enum_values);
	printf("write enum_values size: %zu, expected: %zu\n", wr_size * sizeof(enum_values), sizeof(enum_values));

	printf("write total size: %zu, expected: %zu\n", total,
			sizeof(objs) + sizeof(structs) + sizeof(enums) + sizeof(enum_values));

	fclose(fp);

	return 0;
}
