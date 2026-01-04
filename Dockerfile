FROM alpine:latest AS builder
RUN apk add build-base
RUN mkdir -p /app
COPY ./hole-punch-master /app
WORKDIR /app
RUN make

FROM alpine:latest
RUN apk add --no-cache gcc
RUN mkdir -p /app
COPY --from=builder /app/hole_punch_server .
RUN ls /app
EXPOSE 666/udp
ENTRYPOINT [ "./hole_punch_server" ]
CMD [ "666" ]