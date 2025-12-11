#!/usr/bin/env bash
set -eo pipefail

BUILD_DIR="build"
SRC_DIR="$(dirname "$0")"

if [[ ! -f "$SRC_DIR/CMakeLists.txt" ]]; then
	echo "[build.sh] CMakeLists.txt not found in $SRC_DIR" >&2
	exit 1
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "[build.sh] Running cmake ..."
cmake ..
echo "[build.sh] Running make ..."
make

echo "[build.sh] Build finished. Executables are in $BUILD_DIR/"
