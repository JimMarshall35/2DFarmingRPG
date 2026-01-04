FROM alpine:3.14
RUN mkdir -p /app
COPY ./hole-punch-master/hole_punch_server /app
RUN ls /app
EXPOSE 666/udp
ENTRYPOINT [ "/app/hole_punch_server" ]
CMD [ "666" ]