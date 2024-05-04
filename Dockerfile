FROM alpine:latest

# Installa le dipendenze
RUN apk add --update gcc g++ clang gdb cmake make ninja autoconf automake dos2unix tar rsync hiredis hiredis-dev libpq-dev nlohmann-json valgrind git redis\
    && rm -rf /tmp/* /var/cache/apk/*

# Clona la libreria redis-plus-plus
RUN git clone --recursive https://github.com/sewenew/redis-plus-plus.git /redis-plus-plus

# Compila la libreria redis-plus-plus
WORKDIR /redis-plus-plus

RUN mkdir build && \
        cd build && \
        # Configura il progetto con cmake
        cmake .. && \
        # Compila il codice
        make && \
        # Installa redis-plus-plus
        make install

WORKDIR /
#FROM debian:bookworm

## Aggiorna il repository e installa le dipendenze
#RUN apt-get update && \
#    apt-get install -y \
#        redis-server \
#        postgresql \
#        postgresql-contrib \
#        cmake \
#        build-essential \
#        libhiredis-dev \
#        libpq-dev \
#        git \
#        && \
#    apt-get clean && \
#    rm -rf /var/lib/apt/lists/*