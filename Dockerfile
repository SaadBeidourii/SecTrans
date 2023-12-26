FROM ubuntu:20.04

RUN apt-get update -y &&\
    apt-get install -y --allow-unauthenticated sqlite3 libsqlite3-dev gcc make

WORKDIR /app

COPY . /app

RUN make

EXPOSE 4000

