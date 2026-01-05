FROM alpine:latest AS builder
RUN apk add build-base
RUN apk add cmake
RUN apk add glfw
RUN apk add libxml2
RUN apk add box2d
RUN apk add freetype
RUN apk add gtest
RUN apk add lua5.4

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