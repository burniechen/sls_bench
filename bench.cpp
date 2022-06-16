#include <iostream> 
#include <fstream>

#include "src/sls.hpp"
#include "src/bench.hpp"

using namespace std;

int main() {
	constexpr auto rnd = 5UL;
	int level = 7;
	ofstream fout("rmc1.csv");

	for (auto i=0; i<level; ++i) {
		sls_config *config = new sls_config("table/rmc1", 4000000, 32, 80, 1<<i);
		auto test_io_buf = bind(sls_io_buf, config);
		auto pre_io_buf = bind(pre_hook, config, "io_buf");
		auto post_io_buf = bind(post_hook, config, "io_buf");

		auto test_io_unbuf = bind(sls_io_unbuf, config);
		auto pre_io_unbuf = bind(pre_hook, config, "io_unbuf");
		auto post_io_unbuf = bind(post_hook, config, "io_unbuf");

		auto test_mmap = bind(sls_mmap, config);
		auto pre_mmap = bind(pre_hook, config, "mmap");
		auto post_mmap = bind(post_hook, config, "mmap");

		auto test_ram = bind(sls_ram, config);
		auto pre_ram = bind(pre_hook, config, "ram");
		auto post_ram = bind(post_hook, config, "ram");

		auto bench_io_buf = bp::real_time(test_io_buf, pre_io_buf, post_io_buf);
		auto bench_io_unbuf = bp::real_time(test_io_unbuf, pre_io_unbuf, post_io_unbuf);
		auto bench_mmap = bp::real_time(test_mmap, pre_mmap, post_mmap);
		auto bench_ram = bp::real_time(test_ram, pre_ram, post_ram);

		auto result = bp::bench(rnd, bp::excl_avg<1>, 
								bench_io_buf, 
								bench_io_unbuf,
								bench_mmap,
								bench_ram);

		cout << "[TIME]\n";
		fout << (1<<i) << ',';
		for (auto e : result) {
			cout << (1<<i) << " : " << e.count()/1000 << endl;
			fout << e.count() << ',';
		}
		cout << endl;
		fout << endl;
	}

	return 0;
}
