FROM alpine:3.14
RUN apk add build-base
RUN apk add --no-cache su-exec
RUN mkdir -p /app
COPY ./hole-punch-master /app
WORKDIR /app
RUN make
RUN ls 
EXPOSE 666
ENTRYPOINT su-exec "/app/hole_punch_server"