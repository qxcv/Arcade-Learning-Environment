FROM ubuntu:20.04

# installing curl/zip/unzip/wget/tar because vcpkg requires them (or at least
# requires curl/zip/unzip/tar; wget is there just in case; pkg-config is for SDL)
RUN export DEBIAN_FRONTEND=noninteractive \
  && apt-get -y update \
  && apt-get install -y build-essential cmake ffmpeg git curl zip unzip wget tar pkg-config

RUN useradd -m wabbit
USER wabbit

# the mkdir ~/.vcpkg is to stop vcpkg integrate install from dying with "no such
# file or directory" when it tries to write into that dir
RUN cd ~ \
  && git clone https://github.com/microsoft/vcpkg \
  && ./vcpkg/bootstrap-vcpkg.sh -disableMetrics \
  && mkdir ~/.vcpkg \
  && ./vcpkg/vcpkg integrate install
ENV PATH="/home/wabbit/vcpkg:$PATH"
RUN vcpkg install zlib sdl2
