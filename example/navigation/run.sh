#!/bin/bash
set -eu

SOURCE_FILES="/ros_ws/build src/turtlebot3_simulations/turtlebot3_gazebo/src/turtlebot3_drive.cpp"
SOURCE_FILES="/ros_ws/build src/turtlebot3_simulations/turtlebot3_fake/src/turtlebot3_fake.cpp"
HERE_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
DOCKER_DIR="${HERE_DIR}/../../docker"

make -C "${DOCKER_DIR}"

pushd "${HERE_DIR}"
docker build -t example .
#docker run --rm -w /ros_ws -it example
#docker run --rm -w /ros_ws -it example rosdiscover -p /ros_ws/build src/turtlebot3_simulations/turtlebot3_gazebo/src/turtlebot3_drive.cpp
docker run --rm -w /ros_ws -it example rosdiscover -p /ros_ws/build ${SOURCE_FILES}
