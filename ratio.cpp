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
	auto ratios = 9;
	ofstream fout("rmc2.csv");
	bool fout_flag = true;

	string path = "/home/nctu/dlrm-file/dlrm/table_rm2/";
	auto sum = vector<vector<double>> (shift, vector<double> (ratios, 0));

	for (auto i=0; i<shift; ++i) {
		for (auto it : fs::directory_iterator(path)) {
			for (auto r=1; r<10; ++r) {
				string emb = fs::absolute(it);
				sls_config *config = new sls_config(emb, 500000, 64, 1<<i, 120, r);

				auto test_ratio = bind(sls_ratio, config);
				auto pre_ratio = bind(pre_hook, config, "ratio");
				auto post_ratio = bind(post_hook, config, "ratio");

				auto bench_ratio = bm::real_time(test_ratio, pre_ratio, post_ratio);

				auto result = bm::bench(rnd, bm::excl_avg<bm::nanos, 1>, bench_ratio);

				cout << "[Time]\n";
				printf("[%u] (%s, %d): %lu\n", r, emb.c_str(), (1<<i), result.count()/1000);
				sum[i][r-1] += (result.count()/1000);
				cout << endl;

				delete config;
			}
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
			for (auto j=0; j<ratios; ++j)
				fout << sum[i][j] << ',';
			fout << endl;
		}
	}

	fout.close();

	return 0;
}
