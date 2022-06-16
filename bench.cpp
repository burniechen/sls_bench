#include <iostream>

#include "src/sls.hpp"
#include "src/bench.hpp"

using namespace std;

int main() {
	sls_config *config = new sls_config("table/rmc1", 4000000, 32, 80, 1);

	sls_io_buf(config, true);
	sls_io_unbuf(config, true);
	sls_mmap(config, true);
	sls_ram(config, true);

	return 0;
}
