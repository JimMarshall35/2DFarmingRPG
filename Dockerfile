FROM alpine:latest AS builder
RUN apk add build-base
RUN apk add cmake
RUN mkdir -p /app
COPY ./Stardew /app
RUN mkdir /app/build
WORKDIR /app/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release


FROM alpine:latest
RUN mkdir -p /app
COPY --from=builder /app/build/matchmaking_server /app/
RUN ls /app
EXPOSE 666/udp
ENTRYPOINT [ "/app/MatchmakingServer" ]
CMD [ "666" ]