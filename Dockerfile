FROM debian:bullseye-slim
RUN apt-get update && apt-get install -y gcc make
RUN mkdir -p /app
COPY ./hole-punch-master /app
WORKDIR /app
RUN make
RUN ls 
EXPOSE 666/udp 
ENTRYPOINT [ "sudo", "/app/hole_punch_server", "666" ]