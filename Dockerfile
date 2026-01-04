FROM alpine:3.14
COPY ./hole-punch-master/hole_punch_server /usr/bin/
CMD [ "sudo", "hole_punch_server", "666" ]