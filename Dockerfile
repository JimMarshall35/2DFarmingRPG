FROM alpine:3.14
RUN apk add build-base
RUN mkdir -p /app
COPY ./hole-punch-master /app
WORKDIR /app
RUN make
RUN ls 
EXPOSE 666
CMD [ "/app/hole_punch_server", "666" ]