FROM alpine:3.14
COPY ./hole-punch-master/server /usr/bin/
CMD [ "sudo", "hole_punch_server", "666" ]