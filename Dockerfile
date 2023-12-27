FROM ubuntu:20.04

RUN apt-get update -y &&\
    apt-get install -y libssl-dev sqlite3 libsqlite3-dev gcc make

WORKDIR /app

COPY . /app

RUN make

EXPOSE 4000

CMD bin/initdb
CMD bin/server
