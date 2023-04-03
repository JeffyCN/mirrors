#include <stdio.h>
#include "j2s.h"
#include "j2s_generated_template_json.h"

int main(int argc, char** argv) {
	j2s_ctx ctx;
	char *buf;
	int ret = -1;
    FILE* json_file = NULL;

	j2s_init(&ctx);
	ctx.format_json = true;
	ctx.dump_desc = true;

	printf("Dump template JSON\n");
	buf = j2s_dump_template_root_struct(&ctx);
	DASSERT(buf, goto err);
	printf("Result:\n%s\n", buf);
	free(buf);

    json_file = fopen("rkaiq_ui_template.json","w");
    fprintf(json_file, "%s\n", _j2s_template_json);
    printf("Dump template JSON:\n%s\n", _j2s_template_json);

err:
    fclose(json_file);
	j2s_deinit(&ctx);
	return ret;
}
