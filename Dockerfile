ARG ALPINE_IMAGE=alpine:3.17

FROM ${ALPINE_IMAGE} AS build

WORKDIR /root/rtorrent

# Install build dependencies
RUN apk --no-cache add \
    bash \
    build-base \
    gcompat \
    git \
    linux-headers \
    pythonispython3 \
    python3 \
    rpm

RUN rpm --initdb


RUN if [[ `uname -m` == "aarch64" ]]; then \
        wget https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-arm64 -O /usr/local/bin/bazel ;\
    elif [[ `uname -m` == "x86_64" ]]; then \
        wget https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64 -O /usr/local/bin/bazel ; \
    fi


RUN chmod +x /usr/local/bin/bazel

ENV USE_BAZEL_VERSION=7

RUN bazel --help

# Checkout rTorrent sources from current directory
COPY . ./

# # Checkout rTorrent sources from Github repository
# RUN git clone https://github.com/jesec/rtorrent .

# Set architecture for packages
RUN set -ex ; if [[ `uname -m` == "aarch64" ]]; then \
        sed -i 's/architecture = \"all\"/architecture = \"arm64\"/' BUILD.bazel; \
    elif [[ `uname -m` == "x86_64" ]]; then \
        sed -i 's/architecture = \"all\"/architecture = \"amd64\"/' BUILD.bazel; \
    fi



# Build rTorrent packages
RUN bazel build rtorrent-deb --features=fully_static_link --verbose_failures

# Copy outputs
RUN mkdir dist
RUN cp -L bazel-bin/rtorrent dist/
RUN cp -L bazel-bin/rtorrent-deb.deb dist/

# Now get the clean image
FROM ${ALPINE_IMAGE} AS build-sysroot

WORKDIR /root

# Fetch runtime dependencies
RUN apk --no-cache add \
    binutils \
    ca-certificates \
    ncurses-terminfo-base

# Install rTorrent and dependencies to new system root
RUN mkdir -p /root/sysroot/etc/ssl/certs
COPY --from=build /root/rtorrent/dist/rtorrent-deb.deb .
RUN ar -xv rtorrent-deb.deb
RUN tar xvf data.tar.* -C /root/sysroot/
RUN cp -L /etc/ssl/certs/ca-certificates.crt /root/sysroot/etc/ssl/certs/ca-certificates.crt
RUN cp -r /etc/terminfo /root/sysroot/etc/terminfo

# Create 1001:1001 user
RUN mkdir -p /root/sysroot/home/download
RUN chown 1001:1001 /root/sysroot/home/download

FROM scratch AS rtorrent

COPY --from=build-sysroot /root/sysroot /

# Run as 1001:1001 user
ENV HOME=/home/download
USER 1001:1001

# rTorrent
ENTRYPOINT ["rtorrent"]
