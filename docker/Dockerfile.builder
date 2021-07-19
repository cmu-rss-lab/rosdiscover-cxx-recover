FROM ubuntu:xenial-20210416
ENV DEBIAN_FRONTEND noninteractive
ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        apt-transport-https \
        build-essential \
        ca-certificates \
        curl \
        g++ \
        gcc \
        git \
        libboost-all-dev \
        musl-dev \
        ninja-build \
        python3 \
        python3-pip \
        software-properties-common \
        vim \
        wget \
        zlib1g-dev \
 && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
  | gpg --dearmor - \
  | tee /etc/apt/trusted.gpg.d/kitware.gpg > /dev/null \
 && apt-add-repository 'deb https://apt.kitware.com/ubuntu/ xenial main' \
 && apt-get update \
 && apt-get install -y cmake \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*
