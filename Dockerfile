FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -y; \
    apt-get install -y build-essential \
    gcc \
    g++ \
    cmake \
    make \
    libpthread-stubs0-dev
COPY . /crt_dir
WORKDIR /crt_dir/build
RUN cmake .. && /usr/bin/make -j4
CMD ["./crt"]