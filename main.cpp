#include <iostream>

#include "src/sls.h"
using namespace std;

int main() {
	const char *filename = "table/rmc1";
	sls_io(filename, 1);
	// sls_dram(filename, 1);

	return 0;
}
