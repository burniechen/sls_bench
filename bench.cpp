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
	auto rnd = 4;
	auto shift = 7;
	auto total_type = 6;
	ofstream fout("rmc3.csv");
	bool fout_flag = true;

	string path = "/home/nctu/dlrm-file/dlrm/table_rm3/";
	auto sum = vector<vector<double>> (shift, vector<double> (total_type, 0));

	for (auto i=0; i<shift; ++i) {
		for (auto it : fs::directory_iterator(path)) {
			string emb = fs::absolute(it);
			sls_config *config = new sls_config(emb, 2000000, 32, 1<<i, 20, 1);

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

			auto test_opt = bind(sls_opt, config);
			auto pre_opt = bind(pre_hook, config, "opt");
			auto post_opt = bind(post_hook, config, "opt");

			auto bench_io_buf = bm::real_time(test_io_buf, pre_io_buf, post_io_buf);
			auto bench_io_unbuf = bm::real_time(test_io_unbuf, pre_io_unbuf, post_io_unbuf);
			auto bench_mmap = bm::real_time(test_mmap, pre_mmap, post_mmap);
			auto bench_ram = bm::real_time(test_ram, pre_ram, post_ram);
			auto bench_ratio = bm::real_time(test_ratio, pre_ratio, post_ratio);
			auto bench_opt = bm::real_time(test_opt, pre_opt, post_opt);

			auto result = bm::bench(rnd, bm::excl_avg<bm::nanos, 1>,
									bench_io_buf, 
									bench_io_unbuf,
									bench_mmap,
									bench_ram,
									bench_ratio,
									bench_opt);

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
			for (auto j=0; j<total_type; ++j)
				fout << sum[i][j] << ',';
			fout << endl;
		}
	}

	fout.close();

	return 0;
}
