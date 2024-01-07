FROM ubuntu:20.04

RUN useradd -d /home/tester -m -s /bin/bash tester && echo "tester:tester" | chpasswd && adduser tester sudo

RUN apt-get update -y &&\
    apt-get install -y libssl-dev sqlite3 libsqlite3-dev gcc make

WORKDIR /app

COPY . /app

RUN chown -R tester:tester /app

USER tester

RUN make 

EXPOSE 3000

CMD ["sh", "-c", "./bin/initdb && ./bin/server"]
