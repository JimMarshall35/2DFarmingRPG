FROM alpine:latest AS builder
RUN apk add build-base
RUN mkdir -p /app
COPY ./hole-punch-master /app
WORKDIR /app
RUN make

FROM alpine:latest
RUN mkdir -p /app
COPY --from=builder /app/hole_punch_server /app/
RUN ls /app
EXPOSE 666/udp
ENTRYPOINT [ "/app/hole_punch_server" ]
CMD [ "666" ]