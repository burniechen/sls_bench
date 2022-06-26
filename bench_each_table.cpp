#include <iostream> 
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <map>
#include <functional>

#include "src/sls.hpp"
#include "src/bench.hpp"

using namespace std;
namespace fs = std::filesystem;

int main() {
	auto rnd = 1ul;
	auto shift = 9ul;

	auto types = vector<string> {
		"io_buf", 
		"io_unbuf", 
		"mmap", 
		"ram", 
		"ratio", 
		"opt"
	};

	void (*funs[])(sls_config &) = {
		sls_io_buf,
		sls_io_unbuf,
		sls_mmap,
		sls_ram,
		sls_ratio,
		sls_opt
	};

	ofstream fout("rmc1.csv");
	bool fout_flag = true;

	string path = "/home/nctu/dlrm-file/dlrm/table_rm1/";
	auto sum = vector<vector<u64>> (shift, vector<u64> (types.size(), 0));

	for (auto i=0ul; i<shift; ++i) {
		for (auto it : fs::directory_iterator(path)) {
			auto emb = fs::absolute(it);
			auto config = sls_config(emb, 4000000, 32, 1<<i, 80, 1);

			auto test_funs = map< string, function<void()> > ();
			auto  pre_funs = map< string, function<void()> > ();
			auto post_funs = map< string, function<void()> > ();

			for (auto idx=0ul; idx<types.size(); ++idx) {
				auto str = types[idx];

				auto test = bind(funs[idx], config);
				auto pre  = bind(pre_hook, config, str);
				auto post = bind(post_hook, config, str);

				test_funs.insert(make_pair(str, test));
				pre_funs.insert(make_pair(str, pre));
				post_funs.insert(make_pair(str, post));
			}

			auto bench_io_buf = bm::real_time(test_funs["io_buf"], pre_funs["io_buf"], post_funs["io_buf"]);
			auto bench_io_unbuf = bm::real_time(test_funs["io_unbuf"], pre_funs["io_unbuf"], post_funs["io_unbuf"]);
			auto bench_mmap = bm::real_time(test_funs["mmap"], pre_funs["mmap"], post_funs["mmap"]);
			auto bench_ram = bm::real_time(test_funs["ram"], pre_funs["ram"], post_funs["ram"]);
			auto bench_ratio = bm::real_time(test_funs["ratio"], pre_funs["ratio"], post_funs["ratio"]);
			auto bench_opt = bm::real_time(test_funs["opt"], pre_funs["opt"], post_funs["opt"]);

			auto result = bm::bench(rnd, bm::excl_avg<bm::nanos, 1>,
									bench_io_buf, 
									bench_io_unbuf,
									bench_mmap,
									bench_ram,
									bench_ratio,
									bench_opt);

			cout << "[Time]\n";
			auto type_idx = 0ul;
			for (auto e : result) {
				printf("[%8s] (%s, %d): %8lu µs\n", 
						types[type_idx].c_str(), emb.c_str(), (1<<i), e.count()/1000);
				sum[i][type_idx] += (e.count()/1000);
				type_idx++;
			}
			cout << endl;
		}
	}

	cout << "[Break down]\n";
	for (auto r : sum) {
		for (auto e : r)
			printf("%10lu", e);
		cout << endl;
	}

	if (fout_flag) {
		for (auto i=0ul; i<shift; ++i) {
			fout << (1<<i) << ',';
			for (auto j=0ul; j<types.size(); ++j)
				fout << sum[i][j] << ',';
			fout << endl;
		}
	}

	fout.close();

	return 0;
}
