FROM alpine:3.14
RUN apk add build-base
RUN set -ex && apk --no-cache add sudo
RUN mkdir -p /app
COPY ./hole-punch-master /app
WORKDIR /app
RUN make
RUN ls 
EXPOSE 666
ENTRYPOINT [ "sudo", "/app/hole_punch_server", "666" ]