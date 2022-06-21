#include <iostream> 
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <regex>

#include "src/sls.hpp"
#include "src/bench.hpp"

using namespace std;
namespace fs = std::filesystem;

void parse_arg(map<string, string> &m, string target, string pattern) {
	auto begin = 0;
	auto end = target.find(pattern);

	auto key = string(), val = string();
	while (end != string::npos) {
		key = target.substr(begin, end - begin);
		begin = end + pattern.size();
		end = target.find(pattern, begin);
	}

	val = target.substr(begin, end - begin);

	cout << key << ',' << val << endl;
	m.insert(make_pair(key, val));
}

int main(int argc, char *argv[]) {
	auto arg = map<string, string> ();
	for (int i=1; i<argc; ++i)
		parse_arg(arg, argv[i], "=");

	auto rnd = 1;
	auto power = 1;

	auto dir = arg["--dir"];
	dir = regex_replace(dir, regex("~/"), "/home/nctu/");
	auto R = stoi(arg["--embedding-size"]);
	auto C = stoi(arg["--feature-size"]);
	// auto K = stoi(arg["--lengths-count"]); // batch-size
	auto L = stoi(arg["--num-indices-per-lookup"]);

	auto sum = 0;
	for (auto i=0; i<power; ++i) {
		for (auto it : fs::directory_iterator(dir)) {
			string emb = fs::absolute(it);
			sls_config *config = new sls_config(emb, R, C, 1<<i, L);

			auto test_ram = bind(sls_ram, config);
			auto pre_ram = bind(pre_hook, config, "ram");
			auto post_ram = bind(post_hook, config, "ram");

			auto bench_ram = bm::real_time(test_ram, pre_ram, post_ram);

			auto result = bm::bench(rnd, bm::excl_avg<bm::nanos, 1>, bench_ram);
			sum += (result.count()/1000);

			cout << "[Time]\n";
			printf("(%s, %d): %lu\n", emb.c_str(), (1<<i), result.count()/1000);
			cout << endl;

			delete config;
		}
	}

	printf("[Break down] %d\n", sum);

	return 0;
}
