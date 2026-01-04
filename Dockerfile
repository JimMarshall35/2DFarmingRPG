FROM alpine:3.14
RUN mkdir -p /app
COPY ./hole-punch-master/hole_punch_server /app
RUN ls /app
WORKDIR /app
EXPOSE 666/udp
CMD [ "/app/hole_punch_server", "666" ]