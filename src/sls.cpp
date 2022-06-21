#include "sls.hpp"

#include <iostream>
#include <vector>

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

using namespace std;

void pre_hook(sls_config *config, string s) {
	printf("-------- [BEG] %s --------\n", s.c_str());

	system("echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null");
	config->gen_ids();

	printf("emb-table: %s\n", config->table.c_str());
	printf("emb-size: %u\n", config->emb_row);
	printf("feature-size: %u\n", config->emb_col);
	printf("batch-size: %u\n", config->lengths);
	printf("num-indices-per-lookup: %u\n", config->lengths_size);
	printf("total-lookup: %lu\n", config->ids.size());
}

void post_hook(sls_config *config, string s) {
	printf("output-size: %ux%u\n", config->lengths, config->emb_col);
	printf("-------- [END] %s --------\n\n", s.c_str());
}

void emb_vec_io_buf(FILE *fp, vector<double> &v, u32 ID) {
	rewind(fp);
	fseek(fp, v.size() * ID * sizeof(double), SEEK_SET);
	fread(&v[0], sizeof(double), v.size(), fp);
}

void emb_vec_io_unbuf(int fd, vector<double> &v, u32 ID) {
	lseek(fd, v.size() * ID * sizeof(double), SEEK_SET);
	read(fd, &v[0], v.size() * sizeof(double));
}

void sls_io_buf(sls_config *config) {
	FILE *fp = fopen(config->table.c_str(), "rb");
	if (fp == NULL) {
		fputs("File error", stderr);
		exit(1);
	}

	struct stat sb;
	if (stat(config->table.c_str(), &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	auto C = config->emb_col;
	auto K = config->lengths;
	auto Lengths = vector<u32> (K, config->lengths_size);
	auto v = vector<double> (C, 0);
	auto ans = vector<double> (K * C, 0);

	u32 curID = 0, outID = 0;
	// sls
	for (auto L : Lengths) {
		for (size_t i=curID; i<curID+L; ++i) {
			auto ID = config->ids[i];
			emb_vec_io_buf(fp, v, ID);

			for (size_t idx=0; idx<C; ++idx)
				ans[outID * C + idx] += v[idx];
		}
		outID += 1;
		curID += L;
	}

	fclose(fp);
}

void sls_io_unbuf(sls_config *config) {
	int fd = open(config->table.c_str(), O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	struct stat sb;
	if (stat(config->table.c_str(), &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	auto C = config->emb_col;
	auto K = config->lengths;
	auto Lengths = vector<u32> (K, config->lengths_size);
	auto v = vector<double> (C, 0);
	auto ans = vector<double> (K * C, 0);

	u32 curID = 0, outID = 0;
	// sls
	for (auto L : Lengths) {
		for (size_t j=curID; j<curID+L; ++j) {
			u32 ID = config->ids[j];
			emb_vec_io_unbuf(fd, v, ID);

			for (size_t idx=0; idx<C; ++idx)
				ans[outID * C + idx] += v[idx];
		}
		outID += 1;
		curID += L;
	}

	close(fd);
}

void sls_mmap(sls_config *config) {
	int fd = open(config->table.c_str(), O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	struct stat sb;
	if (stat(config->table.c_str(), &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	auto R = config->emb_row;
	auto C = config->emb_col;
	auto K = config->lengths;
	auto Lengths = vector<u32> (K, config->lengths_size);
	auto ans = vector<double> (K * C, 0);

	double *map = (double *) mmap(NULL, R * C * sizeof(double), PROT_READ, MAP_SHARED, fd, 0);

	u32 curID = 0, outID = 0;
	// sls
	for (auto L : Lengths) {
		for (size_t j = curID; j < curID + L; ++j) {
			u32 ID = config->ids[j];
			for (size_t idx = 0; idx < C; ++idx)
				ans[outID * C + idx] += map[ID * C + idx];
		}
		outID += 1;
		curID += L;
	}
	cout << endl;

	munmap(map, R * C * sizeof(double));
	close(fd);
}

void sls_ram(sls_config *config) {
	int fd = open(config->table.c_str(), O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	struct stat sb;
	if (stat(config->table.c_str(), &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}
	
	auto R = config->emb_row;
	auto C = config->emb_col;
	auto K = config->lengths;
	auto Lengths = vector<u32> (K, config->lengths_size);
	auto ans = vector<double> (K * C, 0);

	auto emb = vector<double> (R * C);
	read(fd, &emb[0], R * C * sizeof(double));

	int curID = 0, outID = 0;
	// sls
	for (auto L : Lengths) {
		for (size_t j = curID; j < curID + L; ++j) {
			u32 ID = config->ids[j];
			for (size_t idx = 0; idx < C; ++idx)
				ans[outID * C + idx] += emb[ID * C + idx];
		}
		outID += 1;
		curID += L;
	}

	close(fd);
}
