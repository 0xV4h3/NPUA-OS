#!/usr/bin/env bash
set -u
set -o pipefail

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
	rc=127
	# try local top-level, then bin/
	if [[ -x "./$exe" ]]; then
		./$exe "$@"
		rc=$?
	elif [[ -x "./bin/$exe" ]]; then
		./bin/$exe "$@"
		rc=$?
	else
		echo "[run_all] executable not found: $exe (run build.sh first)" >&2
	fi
	echo "[run_all] exit code: $rc"
	return $rc
}

echo "=== lab5/run_all.sh: enhanced smoke tests (run-only) ==="

echo "-- prepare test files --"
printf '0123456789abcdef\n0123456789abcdef\n' > /tmp/lab5_testfile
printf 'hello cp\n' > /tmp/lab5_src_cp
# 1KB zero-filled file for task 2
dd if=/dev/zero of=/tmp/lab5_1kb bs=1024 count=1 &>/dev/null || true

echo "-- cp_simple (task 9) --"
run_exec cp_simple /tmp/lab5_src_cp /tmp/lab5_dest_cp || true
if cmp -s /tmp/lab5_src_cp /tmp/lab5_dest_cp; then echo "cp_simple: OK"; else echo "cp_simple: MISMATCH"; fi

echo "-- mcat (task 1) --"
run_exec mcat /tmp/lab5_testfile || true

echo "-- mmap basic on testfile (task 2) --"
run_exec mmap /tmp/lab5_testfile || true

echo "-- mmap on 1KB zero file with an arbitrary value (task 2 strict) --"
# This exercises mapping a 1KB file and writing a value into the mapping.
# MEM_SIZE in mmap.c is small (15), but this still demonstrates mapping a different file.
run_exec mmap /tmp/lab5_1kb SOMEVALUE || true

echo "-- mmap private copy test (task 5) --"
run_exec mmap /tmp/lab5_testfile HELLO --private || true
echo "mmap private: file hexdump (first lines):"
hexdump -C /tmp/lab5_testfile | head

echo "-- mmap SIGBUS demonstration (task 4) --"
run_exec mmap /tmp/lab5_testfile --sigbus || echo "mmap --sigbus ended"

echo "-- mmap sleep + /proc/<pid>/maps demo (task 3) --"
# Start mmap with --sleep so it pauses inside the program; we'll stop it, dump /proc/<pid>/maps, then continue.
MMAP_PID=0
if [[ -x "./mmap" ]]; then
	./mmap /tmp/lab5_testfile --sleep &
	MMAP_PID=$!
elif [[ -x "./bin/mmap" ]]; then
	./bin/mmap /tmp/lab5_testfile --sleep &
	MMAP_PID=$!
fi
sleep 0.5
if ps -p $MMAP_PID > /dev/null 2>&1; then
	echo "Pausing process $MMAP_PID (SIGSTOP) and dumping /proc/$MMAP_PID/maps"
	kill -STOP $MMAP_PID || true
	echo "--- /proc/$MMAP_PID/maps (first 40 lines) ---"
	if [[ -r "/proc/$MMAP_PID/maps" ]]; then
		sed -n '1,40p' "/proc/$MMAP_PID/maps"
	else
		echo "Cannot read /proc/$MMAP_PID/maps"
	fi
	echo "--- end maps ---"
	echo "Resuming process $MMAP_PID (SIGCONT)"
	kill -CONT $MMAP_PID || true
	wait $MMAP_PID || true
else
	echo "mmap with --sleep did not start"
fi

echo "(Tip: you can reproduce interactively by running mmap with --sleep and pressing Ctrl+Z, then 'cat /proc/<pid>/maps')"

echo "-- anon_mmap (task 7) --"
# anon_mmap uses MAP_PRIVATE by default; to test MAP_ANONYMOUS build with USE_MAP_ANON.
run_exec anon_mmap || true

echo "-- mmap munmap test (task 6, may cause SIGSEGV) --"
# The mmap program supports --munmap which unmaps the region then attempts access.
# This may crash the program; we run it to demonstrate behavior.
run_exec mmap /tmp/lab5_testfile --munmap || true

echo "-- writer/reader (task 8 replacement using mmap) --"
# writer/reader in repo already use mmap-backed file /tmp/mmap_shmfile
if [[ -x "./writer" ]] || [[ -x "./bin/writer" ]]; then
	if [[ -x "./writer" ]]; then
		./writer < /tmp/lab5_testfile > /tmp/lab5_writer.out 2>&1 &
		WID=$!
	elif [[ -x "./bin/writer" ]]; then
		./bin/writer < /tmp/lab5_testfile > /tmp/lab5_writer.out 2>&1 &
		WID=$!
	fi
	sleep 0.2
	run_exec reader > /tmp/lab5_reader.out 2>&1 || true
	wait ${WID:-0} || true

	echo "reader hexdump (first lines):"
	hexdump -C /tmp/lab5_reader.out | head
else
	echo "writer/reader not found; build the project first"
fi

echo
echo "[run_all] All tasks attempted."
echo "[run_all] Notes:"
echo "- To exercise anon_mmap with MAP_ANONYMOUS, rebuild lab5 with USE_MAP_ANON: mkdir -p build && cd build && cmake .. -DBUILD_ALL=ON -DUSE_MAP_ANON=ON && cmake --build ."
echo "- If mmap.c's MEM_SIZE is smaller than some inputs, use the provided testfile(s) used here."
exit 0