#!/bin/bash
make clean
make
ASAN_OPTIONS=detect_leaks=1 ./bench

# ASAN_OPTIONS=detect_leaks=1 ./each --dir="~/dlrm-file/dlrm/table_rm2" --embedding-size="500000" --feature-size="64" --num-indices-per-lookup="120" --type="io_buf"

# ./each --dir="~/dlrm-file/custom/" \
#	--embedding-size="500000" \
#	--feature-size="64" \
#	--num-indices-per-lookup="120" \
#	--type="io_buf"
