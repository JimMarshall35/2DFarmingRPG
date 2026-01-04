FROM alpine:3.14
COPY ./hole-punch-master/hole_punch_server .
EXPOSE 666
CMD [ "ls", ";", "./hole_punch_server", "666" ]