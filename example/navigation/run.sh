#!/bin/bash
set -eu

HERE_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
DOCKER_DIR="${HERE_DIR}/../../docker"

make -C "${DOCKER_DIR}"

pushd "${HERE_DIR}"
docker build -t example .
# docker run --rm -it example
docker run --rm -it example opt -load librosdiscover.so -find-ros-api-calls turtlebot3_drive.bc
