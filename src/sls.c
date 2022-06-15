#include "sls.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

void print_ans(double *ans, int K, int C) {
	for (int i=0; i<K; ++i) {
		for (int j=0; j<C; ++j)
			printf("%lf,", ans[i*C + j]);
		printf("\n");
	}
}

void get_emb_vec(FILE *fp, double *v, int C, int ID) {
	rewind(fp);
	fseek(fp, ID*C*sizeof(double), SEEK_SET);
	fread(v, sizeof(double), C, fp);
}

void sls_io(const char *filename, int flag) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		fputs("File error", stderr);
		exit(1);
	}

	struct stat sb;
	if (stat(filename, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	int C = 32;
	int R = sb.st_size / (C * sizeof(double)); 
	int Lengths[1] = {80};
	int K = sizeof(Lengths) / sizeof(Lengths[0]);
	int sumID = 0;
	for (int i=0; i<K; ++i)
		sumID += Lengths[i];
	int *IDs = (int*) malloc (sizeof(int) * sumID);
	for (int i=0; i<sumID; ++i)
		IDs[i] = rand() % R;

	int curID = 0, outID = 0;

	double *v = (double*) malloc(C * sizeof(double));
	double *ans = (double *) malloc(sizeof(double) * K * C);
	memset(ans, 0, sizeof(double) * K * C);

	// sls
	for (int i=0; i<K; ++i) {
		int L = Lengths[i];
		for (int j=curID; j<curID+L; ++j) {
			int ID = IDs[j];
			get_emb_vec(fp, v, C, ID);

			for (int idx=0; idx<C; ++idx)
				ans[outID * C + idx] += v[idx];
		}
		outID += 1;
		curID += L;
	}

	fclose(fp);
	free(v);
	free(ans);

	printf("[sls_io]\n");
	if (flag) print_ans(ans, K, C);
}

void sls_dram(const char *filename, int flag) {
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		fputs("File error", stderr);
		exit(1);
	}

	struct stat sb;
	if (stat(filename, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	int C = 32;
	int R = sb.st_size / (C * sizeof(double)); 
	int Lengths[1] = {80};
	int K = sizeof(Lengths) / sizeof(Lengths[0]);
	int sumID = 0;
	for (int i=0; i<K; ++i)
		sumID += Lengths[i];
	int* IDs = (int*) malloc (sizeof(int) * sumID);
	for (int i=0; i<sumID; ++i)
		IDs[i] = rand() % R;

	int curID = 0, outID = 0;

	double *ans = (double *) malloc(sizeof(double) * K * C);
	memset(ans, 0, sizeof(double) * K * C);

	double *map = (double *) mmap(NULL, R * C * sizeof(double), PROT_READ, MAP_SHARED, fd, 0);

	// sls
	for (int i = 0; i < K; ++i) {
		int L = Lengths[i];
		for (int j = curID; j < curID + L; ++j) {
			int ID = IDs[j];
			for (int idx = 0; idx < C; ++idx)
				ans[outID * C + idx] += map[ID * C + idx];
		}
		outID += 1;
		curID += L;
	}

	munmap(map, R * C * sizeof(double));
	free(ans);

	printf("[sls_dram]\n");
	if (flag) print_ans(ans, K, C);
}
