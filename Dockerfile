FROM alpine:3.14
RUN mkdir -p /app
COPY ./hole-punch-master/hole_punch_server /app
EXPOSE 666
CMD [ "ls", "/app", ";", "/app/hole_punch_server", "666" ]