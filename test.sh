#!/bin/bash
make clean
make
ASAN_OPTIONS=detect_leaks=1 ./bench
