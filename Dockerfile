# Usa Debian come base image
FROM debian:bookworm

# Aggiorna il repository e installa le dipendenze
RUN apt-get update && \
    apt-get install -y \
        redis-server \
        postgresql \
        postgresql-contrib \
        cmake \
        build-essential \
        libhiredis-dev \
        libpq-dev \
        git \
        && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


# Copia il file di configurazione Redis Streams
# COPY redis.conf /etc/redis/redis.conf

# Esponi le porte di Redis e PostgreSQL
EXPOSE 6379 5432

# Avvia i servizi di Redis e PostgreSQL
CMD service redis-server start && service postgresql start && bash