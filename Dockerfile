FROM alpine:3.14
RUN apk add --no-cache make
RUN mkdir -p /app
COPY ./hole-punch-master /app
WORKDIR /app
RUN make
EXPOSE 666
CMD [ "/app/hole_punch_server", "666" ]