#!/usr/bin/env bash
set -u

BUILD_DIR="build"

if [[ ! -d "$BUILD_DIR" ]]; then
  echo "[run_all] build directory '$BUILD_DIR' not found. Run ./build.sh first." >&2
  exit 1
fi


cd "$BUILD_DIR"

# Kill all previous servers to free ports
pkill -f ipv4_dg_server 2>/dev/null || true
pkill -f ipv6_dg_server 2>/dev/null || true
pkill -f ipv4_stream_echo_server 2>/dev/null || true
pkill -f ipv4_stream_chat_server 2>/dev/null || true
sleep 1

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

# 1) IPv4 Datagram

run_exec ipv4_dg_server &
sleep 1
echo
echo "========== Running: ipv4_dg_client 127.0.0.1 hello world =========="
if [[ -x "ipv4_dg_client" ]]; then
  timeout 10s ./ipv4_dg_client 127.0.0.1 hello world
  rc=$?
  echo "[run_all] exit code: $rc"
else
  echo "[run_all] executable not found: ipv4_dg_client (run build.sh first)" >&2
fi
kill %1 2>/dev/null || true
sleep 1

# 2) IPv6 Datagram

run_exec ipv6_dg_server &
sleep 1
echo
echo "========== Running: ipv6_dg_client ::1 hello world =========="
if [[ -x "ipv6_dg_client" ]]; then
  timeout 10s ./ipv6_dg_client ::1 hello world
  rc=$?
  echo "[run_all] exit code: $rc"
else
  echo "[run_all] executable not found: ipv6_dg_client (run build.sh first)" >&2
fi
kill %1 2>/dev/null || true
sleep 1

# 3) IPv4 Stream Echo Service
run_exec ipv4_stream_echo_server &
echo $! > .echo_server_pid
sleep 1
echo "echo test" | ./ipv4_stream_echo_client 127.0.0.1 | grep 'echo test' && echo "[run_all] Echo service OK"
kill $(cat .echo_server_pid) 2>/dev/null || true
rm -f .echo_server_pid
sleep 1

# 4) IPv4 Stream Chatroom
run_exec ipv4_stream_chat_server &
echo $! > .chat_server_pid
sleep 1
( echo "msg from client1" | ./ipv4_stream_chat_client 127.0.0.1 ) &
client1_pid=$!
( echo "msg from client2" | ./ipv4_stream_chat_client 127.0.0.1 ) &
client2_pid=$!
wait $client1_pid $client2_pid
kill $(cat .chat_server_pid) 2>/dev/null || true
rm -f .chat_server_pid
sleep 1

echo
 echo "[run_all] All tasks attempted."
