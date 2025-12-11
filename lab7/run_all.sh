#!/usr/bin/env bash
set -u

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

# Example runs (customize as needed)
run_exec unix_stream_server &
sleep 1

echo "test message" | ./unix_stream_client

kill %1 2>/dev/null || true

run_exec unix_stream_server_no_remove &
sleep 1
kill %1 2>/dev/null || true

run_exec unix_datagram_server &
sleep 1
run_exec unix_datagram_client 'long message'
kill %1 2>/dev/null || true

# 4) Echo service (stream server echo)
echo
run_exec unix_stream_server &
server_pid=$!
sleep 1
echo "echo test" | ./unix_stream_client | grep 'echo test' && echo "[run_all] Echo service OK"
kill $server_pid 2>/dev/null || true
sleep 1

# 5) Chatroom (stream server broadcast)
echo
run_exec unix_stream_server &
server_pid=$!
sleep 1
# Start two clients in background, each sending a message and reading from server
( echo "msg from client1" | ./unix_stream_client ) &
client1_pid=$!
( echo "msg from client2" | ./unix_stream_client ) &
client2_pid=$!
wait $client1_pid $client2_pid
kill $server_pid 2>/dev/null || true
sleep 1

echo
echo "[run_all] All tasks attempted."
