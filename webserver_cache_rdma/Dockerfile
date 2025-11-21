# Multistage build with RDMA support
FROM ubuntu:24.04 AS build
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates libboost-all-dev \
    rdma-core librdmacm-dev libibverbs-dev ibverbs-providers \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
# Toggle RDMA with -DENABLE_RDMA=OFF if needed
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_RDMA=ON \
 && cmake --build build -j

FROM ubuntu:24.04 AS runtime
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 libgcc-s1 rdma-core \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /app/build/webserver /app/webserver
RUN mkdir -p /site && printf '<h1>It works</h1>\n' > /site/index.html

EXPOSE 8080
EXPOSE 7471
CMD ["/app/webserver", "--port", "8080", "--doc-root", "/site", "--threads", "0", "--cache.mem-mb", "128"]