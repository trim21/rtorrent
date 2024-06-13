FROM gcr.io/distroless/cc-debian12

ENV HOME=/home/download
USER 1001:1001

COPY rtorrent /usr/bin/rtorrent

ENTRYPOINT ["/usr/bin/rtorrent"]