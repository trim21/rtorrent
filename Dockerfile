FROM debian:12-slim AS build

WORKDIR /root/rtorrent

# Install build dependencies
RUN apt-get update -y &&\
    apt-get install build-essentional \
    libcurl4-openssl-dev \
    nlohmann-json3-dev\
    libxmlrpc-c++8-dev \
    libssl-dev \
    zlib1g-dev \
    cmake \
    wget

RUN if [[ `uname -m` == "aarch64" ]]; then \
        wget https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-arm64 -O /usr/local/bin/bazel ;\
    elif [[ `uname -m` == "x86_64" ]]; then \
        wget https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64 -O /usr/local/bin/bazel ; \
    fi


RUN chmod +x /usr/local/bin/bazel

ENV USE_BAZEL_VERSION="7.*"

RUN bazel --help

# Checkout rTorrent sources from current directory

COPY . ./

# Build rTorrent packages
RUN bazel build rtorrent-deb --features=fully_static_link --verbose_failures

# Copy outputs
RUN mkdir dist
RUN cp -L bazel-bin/rtorrent dist/
RUN cp -L bazel-bin/rtorrent-deb.deb dist/


FROM build AS rtorrent

# Run as 1001:1001 user
ENV HOME=/home/download
USER 1001:1001

# rTorrent
ENTRYPOINT ["rtorrent"]
