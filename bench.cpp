#include <iostream> 
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "src/sls.hpp"
#include "src/bench.hpp"

using namespace std;
namespace fs = std::filesystem;

int main() {
	auto rnd = 1;
	auto shift = 7;
	ofstream fout("rmc2.csv");
	bool fout_flag = true;

	string path = "/home/nctu/dlrm-file/dlrm/table_rm2/";
	auto sum = vector<vector<double>> (shift, vector<double> (5, 0));

	for (auto i=0; i<shift; ++i) {
		for (auto it : fs::directory_iterator(path)) {
			string emb = fs::absolute(it);
			sls_config *config = new sls_config(emb, 500000, 64, 1<<i, 120, 2);

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

			auto test_ratio = bind(sls_ratio, config);
			auto pre_ratio = bind(pre_hook, config, "ratio");
			auto post_ratio = bind(post_hook, config, "ratio");

			auto bench_io_buf = bm::real_time(test_io_buf, pre_io_buf, post_io_buf);
			auto bench_io_unbuf = bm::real_time(test_io_unbuf, pre_io_unbuf, post_io_unbuf);
			auto bench_mmap = bm::real_time(test_mmap, pre_mmap, post_mmap);
			auto bench_ram = bm::real_time(test_ram, pre_ram, post_ram);
			auto bench_ratio = bm::real_time(test_ratio, pre_ratio, post_ratio);

			auto result = bm::bench(rnd, bm::excl_avg<bm::nanos, 1>,
									bench_io_buf, 
									bench_io_unbuf,
									bench_mmap,
									bench_ram,
									bench_ratio);

			cout << "[Time]\n";
			auto type = 0ul;
			for (auto e : result) {
				printf("[%lu] (%s, %d): %lu\n", type, emb.c_str(), (1<<i), e.count()/1000);
				sum[i][type] += (e.count()/1000);
				type++;
			}
			cout << endl;

			delete config;
		}
	}

	cout << "[Break down]\n";
	for (auto r : sum) {
		for (auto e : r)
			cout << e << ' ';
		cout << endl;
	}

	if (fout_flag) {
		for (auto i=0; i<shift; ++i) {
			fout << (1<<i) << ',';
			for (auto j=0; j<5; ++j)
				fout << sum[i][j] << ',';
			fout << endl;
		}
	}

	fout.close();

	return 0;
}
