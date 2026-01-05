FROM debian:bookworm AS builder
RUN sudo apt-get update
RUN sudo apt-get install -y cmake build-essential
RUN sudo apt-get install -y libxml2 liblua5.4-dev libxml2-dev libbox2d-dev libglfw3-dev libfreetype-dev libgtest-dev

RUN mkdir -p /app
COPY ./Stardew /app
RUN mkdir /app/build
WORKDIR /app/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release


FROM debian:bookworm-slim
RUN mkdir -p /app
COPY --from=builder /app/build/matchmaking_server/MatchmakingServer /app/
RUN ls /app
EXPOSE 666/udp
ENTRYPOINT [ "/app/MatchmakingServer" ]
CMD [ "666" ]