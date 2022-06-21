#ifndef SLS_HPP
#define SLS_HPP

#include <vector>
#include <string>
#include <random>
#include <cstdint>

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

	sls_config(std::string filename, u32 R, u32 C, u32 K, u32 L) 
	: table(filename), emb_row(R), emb_col(C), lengths(K), lengths_size(L) {};

	void gen_ids() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<u32> dis(0, emb_row-1);

		auto total = lengths * lengths_size;
		ids = std::vector<u32> (total);
		for (auto &ID : ids)
			ID = dis(gen);
	}
};

void pre_hook(sls_config *config, std::string pre_state);
void post_hook(sls_config *config, std::string post_state);

void sls_io_buf(sls_config *config);
void sls_io_unbuf(sls_config *config);
void sls_mmap(sls_config *config);
void sls_ram(sls_config *config);

#endif
