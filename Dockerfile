FROM alpine:3.14
COPY ./hole-punch-master/server /usr/bin/
CMD [ "sudo", "server", "666" ]