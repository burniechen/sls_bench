#include "sls.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void print_ans(double *ans, int K, int C) {
	for (int i=0; i<K; ++i) {
		for (int j=0; j<C; ++j)
			printf("%lf,", ans[i*C + j]);
		printf("\n");
	}
}

void emb_vec_io_buf(FILE *fp, double *v, int C, int ID) {
	rewind(fp);
	fseek(fp, ID*C*sizeof(double), SEEK_SET);
	fread(v, sizeof(double), C, fp);
}

void emb_vec_io_unbuf(int fd, double *v, int C, int ID) {
	lseek(fd, ID*C*sizeof(double), SEEK_SET);
	read(fd, v, C*sizeof(double));
}

void sls_io_buf(const char *table, sls_config *config, int flag) {
	FILE *fp = fopen(table, "rb");
	if (fp == NULL) {
		fputs("File error", stderr);
		exit(1);
	}

	struct stat sb;
	if (stat(table, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	u32 C = config->emb_col;
	u32 K = config->lengths_size;
	u32 *Lengths = (u32*) malloc(K * sizeof(u32));
	for (size_t i=0; i<K; ++i)
		Lengths[i] = config->lengths;

	u32 curID = 0, outID = 0;

	double *v = (double*) malloc(C * sizeof(double));
	double *ans = (double *) malloc(K * C * sizeof(double));
	memset(ans, 0, K * C * sizeof(double));

	// sls
	for (size_t i=0; i<K; ++i) {
		u32 L = Lengths[i];
		for (size_t j=curID; j<curID+L; ++j) {
			u32 ID = config->ids[j];
			emb_vec_io_buf(fp, v, C, ID);

			for (size_t idx=0; idx<C; ++idx)
				ans[outID * C + idx] += v[idx];
		}
		outID += 1;
		curID += L;
	}

	printf("[sls_io_buf]\n");
	if (flag) print_ans(ans, K, C);

	fclose(fp);
	free(v);
	free(ans);
}

void sls_io_unbuf(const char *table, sls_config *config, int flag) {
	int fd = open(table, O_RDONLY);
	if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

	struct stat sb;
	if (stat(table, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	u32 C = config->emb_col;
	u32 K = config->lengths_size;
	u32 *Lengths = (u32*) malloc(K * sizeof(u32));
	for (size_t i=0; i<K; ++i)
		Lengths[i] = config->lengths;

	u32 curID = 0, outID = 0;

	double *v = (double*) malloc(C * sizeof(double));
	double *ans = (double *) malloc(K * C * sizeof(double));
	memset(ans, 0, sizeof(double) * K * C);

	// sls
	for (size_t i=0; i<K; ++i) {
		u32 L = Lengths[i];
		for (size_t j=curID; j<curID+L; ++j) {
			u32 ID = config->ids[j];
			emb_vec_io_unbuf(fd, v, C, ID);

			for (size_t idx=0; idx<C; ++idx)
				ans[outID * C + idx] += v[idx];
		}
		outID += 1;
		curID += L;
	}

	printf("[sls_io_unbuf]\n");
	if (flag) print_ans(ans, K, C);

	close(fd);
	free(v);
	free(ans);
}

void sls_mmap(const char *filename, sls_config *config, int flag) {
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

	struct stat sb;
	if (stat(filename, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	u32 R = config->emb_row;
	u32 C = config->emb_col;
	u32 K = config->lengths_size;
	u32 *Lengths = (u32*) malloc(K * sizeof(u32));
	for (size_t i=0; i<K; ++i)
		Lengths[i] = config->lengths;
	
	int curID = 0, outID = 0;

	double *ans = (double *) malloc(sizeof(double) * K * C);
	memset(ans, 0, sizeof(double) * K * C);

	double *map = (double *) mmap(NULL, R * C * sizeof(double), PROT_READ, MAP_SHARED, fd, 0);

	// sls
	for (size_t i = 0; i < K; ++i) {
		u32 L = Lengths[i];
		for (size_t j = curID; j < curID + L; ++j) {
			u32 ID = config->ids[j];
			for (size_t idx = 0; idx < C; ++idx)
				ans[outID * C + idx] += map[ID * C + idx];
		}
		outID += 1;
		curID += L;
	}

	printf("[sls_mmap]\n");
	if (flag) print_ans(ans, K, C);

	munmap(map, R * C * sizeof(double));
	close(fd);
	free(ans);
}

void sls_ram(const char *filename, sls_config *config, int flag) {
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

	struct stat sb;
	if (stat(filename, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	u32 R = config->emb_row;
	u32 C = config->emb_col;
	u32 K = config->lengths_size;
	u32 *Lengths = (u32*) malloc(K * sizeof(u32));
	for (size_t i=0; i<K; ++i)
		Lengths[i] = config->lengths;
	
	int curID = 0, outID = 0;

	double *emb = (double*) malloc(R * C * sizeof(double));
	read(fd, emb, R * C * sizeof(double));
	double *ans = (double*) malloc(K * C * sizeof(double));
	memset(ans, 0, K * C * sizeof(double));

	// sls
	for (size_t i = 0; i < K; ++i) {
		u32 L = Lengths[i];
		for (size_t j = curID; j < curID + L; ++j) {
			u32 ID = config->ids[j];
			for (size_t idx = 0; idx < C; ++idx)
				ans[outID * C + idx] += emb[ID * C + idx];
		}
		outID += 1;
		curID += L;
	}

	printf("[sls_ram]\n");
	if (flag) print_ans(ans, K, C);

	close(fd);
	free(emb);
	free(ans);
}
