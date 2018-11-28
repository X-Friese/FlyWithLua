FROM ubuntu:16.04

RUN sed -i -e 's/http:\/\/archive.ubuntu.com\/ubuntu\//mirror:\/\/mirrors.ubuntu.com\/mirrors.txt/' /etc/apt/sources.list && apt-get update

# Install Linux toolchain (GCC).
RUN apt-get install -y --no-install-recommends build-essential ninja-build sudo bash coreutils ca-certificates curl \
  && apt-get clean \
  && curl -sSL https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.tar.gz \
     | tar -xz -C /usr/local/ --strip-components=1

# Install Windows cross-compiling toolchain (MinGW).
RUN apt-get install -y --no-install-recommends mingw-w64 g++-mingw-w64 && apt-get clean

# Install OS X cross-compiling toolchain (clang).

RUN apt-get install -y --no-install-recommends clang curl && apt-get clean
#Build arguments
ARG osxcross_repo="tpoechtrager/osxcross"
ARG osxcross_revision="f4ba4facae996b3b14d89eb62c0384564f7368b5"
ARG darwin_sdk_version="10.11"
ARG darwin_sdk_url="https://github.com/apriorit/osxcross-sdks/raw/master/MacOSX10.11.sdk.tar.xz"

# ENV available in docker image
ENV OSXCROSS_REPO="${osxcross_repo}" \
    OSXCROSS_REVISION="${osxcross_revision}" \
    DARWIN_SDK_VERSION="${darwin_sdk_version}" \
    DARWIN_SDK_URL="${darwin_sdk_url}" \
    CROSSBUILD=1

RUN mkdir -p "/tmp/osxcross" \
 && cd "/tmp/osxcross" \
 && curl -sSLo osxcross.tar.gz "https://codeload.github.com/${OSXCROSS_REPO}/tar.gz/${OSXCROSS_REVISION}" \
 && tar --strip=1 -xzf osxcross.tar.gz \
 && rm -f osxcross.tar.gz \
 && curl -sLo tarballs/MacOSX${DARWIN_SDK_VERSION}.sdk.tar.xz \
             "${DARWIN_SDK_URL}" \
 && UNATTENDED=1 JOBS=4 SDK_VERSION=10.11 ./build.sh \
 && mv target /usr/osxcross \
 && mv tools /usr/osxcross/ \
 && rm -rf "/usr/osxcross/SDK/MacOSX${DARWIN_SDK_VERSION}.sdk/usr/share/man" \
 && true

# Install dependency libraries under Linux.
RUN apt-get install -y --no-install-recommends freeglut3-dev libudev-dev libopenal-dev && apt-get clean

RUN echo "" | adduser --uid 1000 --disabled-password  --gecos "" aleks && adduser aleks sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

VOLUME /src
USER aleks
ADD build.sh /usr/bin/build.sh

WORKDIR /src
ENTRYPOINT ["build.sh"]
