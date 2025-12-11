#!/usr/bin/env bash
set -eo pipefail

BUILD_DIR="build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "[build.sh] Compiling sources..."
gcc ../unix_stream_server.c -o unix_stream_server
gcc ../unix_stream_client.c -o unix_stream_client
gcc ../unix_stream_server_no_remove.c -o unix_stream_server_no_remove

gcc ../unix_datagram_server.c -o unix_datagram_server
gcc ../unix_datagram_client.c -o unix_datagram_client

echo "[build.sh] Build finished. Executables are in $BUILD_DIR/"
