FROM alpine:latest

# Installa le dipendenze
RUN apk add --update gcc g++ clang gdb cmake make ninja autoconf automake dos2unix tar rsync hiredis hiredis-dev libpq-dev nlohmann-json valgrind git redis curl-dev boost-dev\
    && rm -rf /tmp/* /var/cache/apk/*

# RUN apt-get update && apt-get install -y curl libcurl4-openssl-dev
# Clona la libreria redis-plus-plus

# DB
ENV DB_HOST=localhost
ENV DB_PORT=5432
ENV DB_NAME=postgres
ENV DB_USER=postgres
ENV DB_PASS=postgres

# Redis
ENV REDIS_HOST=localhost
ENV REDIS_PORT=6379

WORKDIR /
