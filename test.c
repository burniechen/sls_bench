#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "src/sls.h"

int main() {
	srand(time(NULL));

	sls_config *config = malloc(5 * sizeof(u32));
	config->emb_row = 4000000;
	config->emb_col = 32;
	config->lengths = 80;
	config->lengths_size = 1;

	u32 total = config->lengths * config->lengths_size;

	u32 *ids = (u32*) malloc(total * sizeof(u32));
	for (size_t i=0; i<total; ++i)
		ids[i] = rand() % config->emb_row;

	config->ids = ids;

	const char *table = "table/rmc1";
	sls_io(table, config, 1);
	sls_dram(table, config, 1);

	free(ids);

	return 0;
}
