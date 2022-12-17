FROM ubuntu:20.04

RUN sed -i -e 's/http:\/\/archive.ubuntu.com\/ubuntu\//mirror:\/\/mirrors.ubuntu.com\/mirrors.txt/' /etc/apt/sources.list && apt-get update

# Install Linux toolchain (GCC).
RUN apt-get install -y --no-install-recommends build-essential ninja-build sudo bash coreutils ca-certificates curl\
  && apt-get clean \
  && curl -sSL https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.tar.gz \
     | tar -xz -C /usr/local/ --strip-components=1

# Install Windows cross-compiling toolchain (MinGW).
# RUN apt-get install -y --no-install-recommends mingw-w64 g++-mingw-w64 && apt-get clean


# Install dependency libraries under Linux.
RUN apt-get install -y --no-install-recommends freeglut3-dev libudev-dev libopenal-dev && apt-get clean

RUN echo "" | adduser --uid 1000 --disabled-password  --gecos "" sparker && adduser sparker sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

VOLUME /src
USER sparker
ADD build.sh /usr/bin/build.sh

WORKDIR /src
ENTRYPOINT ["build.sh"]
