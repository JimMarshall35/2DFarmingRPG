FROM debian:bullseye-slim
RUN mkdir -p /app
COPY ./hole-punch-master/hole_punch_server /app
EXPOSE 666/udp 
ENTRYPOINT [ "/app/hole_punch_server", "666" ]
