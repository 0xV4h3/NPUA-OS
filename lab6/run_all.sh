#!/usr/bin/env bash
set -u

# This script assumes the project is already built into the "build/" directory.
BUILD_DIR="build"

if [[ ! -d "$BUILD_DIR" ]]; then
  echo "[run_all] build directory '$BUILD_DIR' not found. Run ./build.sh first." >&2
  exit 1
fi

cd "$BUILD_DIR"

run_exec() {
  local exe="$1"; shift
  echo
  echo "========== Running: $exe $* =========="
  if [[ -x "$exe" ]]; then
    ./$exe "$@"
    rc=$?
    echo "[run_all] exit code: $rc"
  else
    echo "[run_all] executable not found: $exe (run build.sh first)" >&2
  fi
}

# 1) task1: thread returns length of string
run_exec task1_simple_return "Hello from run_all"

# 2) check_thread
run_exec check_thread

# 3) detached thread (task3_detach)
run_exec task3_detach

# 4) thread_increment (race) and ordered version
run_exec thread_increment 1000000
run_exec thread_increment_ordered 1000000
run_exec thread_increment_mutex 1000000

# 5) thread_cond_variables
run_exec thread_cond_variables

# 6) elements
run_exec elements

# 7) thread_cancel
run_exec thread_cancel

# 8) chain: double then triple
run_exec task8_chain 3

# 9) fill even/odd concurrently
run_exec task9_fill

# 10) sum first 5 and last 5
run_exec task10_sum

echo
echo "[run_all] All tasks attempted."

