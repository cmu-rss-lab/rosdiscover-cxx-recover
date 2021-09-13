IMAGE_NAME="rosdiscover/cxx-extract"

all: install

builder:
	docker build -f Dockerfile.builder -t ${IMAGE_NAME}:builder .

llvm: builder
	docker build -f Dockerfile.llvm -t christimperley/llvm11 .

withllvm: builder llvm
	docker build -f Dockerfile.rosdiscover -t ${IMAGE_NAME} ..

nollvm: builder
	docker build -f Dockerfile.rosdiscover -t ${IMAGE_NAME} ..

install: nollvm
	./install

.PHONY: builder install llvm nollvm rosdiscover withllvm
