FROM alpine:3.14
COPY ./hole-punch-master/hole_punch_server /usr/bin/
RUN apk add doas-sudo-shim
CMD [ "doas-sudo-shim", "hole_punch_server", "666" ]