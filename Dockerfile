FROM debian:trixie-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN make BUILD=release


FROM debian:trixie-slim

WORKDIR /app
COPY --from=builder /app/build/main .

CMD ["./main"]
