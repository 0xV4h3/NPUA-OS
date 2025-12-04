#!/usr/bin/env bash
set -eo pipefail

BUILD_DIR="build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "[build.sh] Configuring (BUILD_ALL=ON)"
cmake .. -DBUILD_ALL=ON

echo "[build.sh] Building..."
cmake --build . -j || {
  echo "[build.sh] build failed" >&2
  exit 1
}

echo "[build.sh] Build finished. Executables are in $BUILD_DIR/"
