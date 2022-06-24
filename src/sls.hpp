#ifndef SLS_HPP
#define SLS_HPP

#include <vector>
#include <string>
#include <random>
#include <cstdint>
#include <algorithm>

#ifndef _U_TYPE
#define _U_TYPE
typedef int8_t			s8; 
typedef int16_t			s16;
typedef int32_t			s32;
typedef int64_t			s64;
typedef uint8_t			u8; 
typedef uint16_t		u16;
typedef uint32_t		u32;
typedef uint64_t		u64;
#endif 

struct sls_config {
	std::string table;		// filename
	u32 emb_row;			// R, embedding-size
	u32 emb_col;			// C, feature-size
	u32 lengths;			// K, lengths-count, batch-size
	u32 lengths_size;		// L, num-indices-per-lookup
	std::vector<u32> ids;	// ids-list
	u32 ram_ratio;

	sls_config(std::string filename, u32 R, u32 C, u32 K, u32 L, u32 val) 
	: table(filename), emb_row(R), emb_col(C), lengths(K), lengths_size(L), ram_ratio(val) {};

	void gen_ids(bool uniform) {
		std::random_device rd;
		std::mt19937 gen(rd());
		auto total = lengths * lengths_size;

		ids = std::vector<u32> (total);
		if (uniform)
			for (auto &ID : ids)  {
				std::uniform_int_distribution<u32> d(0, emb_row-1);
				ID = d(gen);
			}

		else {
			std::uniform_int_distribution<u32> d_lookup(0, total);
			// 0 < v0 < v1 < v2 < v3 < v4 < v5=total
			auto v = std::vector<u32> (6, total);
			v[0] = d_lookup(gen);
			for (auto i=1; i<5; ++i) {
				v[i] = d_lookup(gen);
				while (v[i] < v[i-1]) 
					v[i] = d_lookup(gen);
			}

			// cnt0 = v0
			// cnt1 = v1-v0
			// cnt2 = v2-v1
			auto cnt = std::vector<u32> (5, 0);
			cnt[0] = v[0];
			for (auto i=1; i<5; ++i)
				cnt[i] = v[i] - v[i-1];

			auto c = 0;
			auto k = emb_row/5;
			for (auto i=0ul; i<5; ++i) {
				std::uniform_int_distribution<u32> d(i*k, (i+1)*k-1);
				for (auto j=0ul; j<cnt[i]; ++j)
					ids[c] = d(gen), c++;
			}
			std::random_shuffle(ids.begin(), ids.end());
		}
	}
};

void pre_hook(sls_config *config, std::string pre_state);
void post_hook(sls_config *config, std::string post_state);

void sls_io_buf(sls_config *config);
void sls_io_unbuf(sls_config *config);
void sls_mmap(sls_config *config);
void sls_ram(sls_config *config);
void sls_ratio(sls_config *config);
void sls_opt(sls_config *config);

#endif
