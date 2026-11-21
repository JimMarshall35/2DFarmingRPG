# appimage builder container
# based on an old ubuntu for maximum appimage compatibility
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# apt packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    pkg-config git curl wget python3 python3-pip build-essential libfuse2 libxml2 liblua5.3-dev libxml2-dev libbox2d-dev libglfw3-dev libfreetype-dev libopenal-dev libgtest-dev zip unzip \
    && rm -rf /var/lib/apt/lists/*
# rust (for nushell)
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# a version of cmake that's newer than the one apt can give us
RUN wget -qO- https://github.com/Kitware/CMake/releases/download/v3.29.0/cmake-3.29.0-linux-x86_64.tar.gz \
    | tar xz -C /opt \
    && ln -s /opt/cmake-3.29.0-linux-x86_64/bin/cmake /usr/local/bin/cmake

# not quite sure what we need this for?
RUN pip install jinja2

# install nushell
RUN PATH="/root/.cargo/bin:${PATH}" && cargo install nu --locked

# add nushell to path
ENV PATH="/root/.cargo/bin:${PATH}"


WORKDIR /work

CMD ["/bin/bash"]