all: nollvm

builder:
	docker build -f Dockerfile.builder -t rosdiscover:builder .

llvm: builder
	docker build -f Dockerfile.llvm -t christimperley/llvm11 .

withllvm: builder llvm
	docker build -f Dockerfile.rosdiscover -t rosdiscover ..

nollvm: builder
	docker build -f Dockerfile.rosdiscover -t rosdiscover ..

.PHONY: builder llvm nollvm rosdiscover withllvm
