#!/usr/bin/env bash
set -u

# Run-only smoke tests for lab5. Expects the project to be built in `build/`.
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
	if [[ -x "./$exe" ]]; then
		./$exe "$@"
		rc=$?
		echo "[run_all] exit code: $rc"
	elif [[ -x "./bin/$exe" ]]; then
		./bin/$exe "$@"
		rc=$?
		echo "[run_all] exit code: $rc"
	else
		echo "[run_all] executable not found: $exe (run build.sh first)" >&2
	fi
}

echo "=== lab5/run_all.sh: smoke tests (run-only) ==="

echo "-- prepare test files --"
printf '0123456789abcdef\n0123456789abcdef\n' > /tmp/lab5_testfile
printf 'hello cp\n' > /tmp/lab5_src_cp
dd if=/dev/zero of=/tmp/lab5_1kb bs=1024 count=1 &>/dev/null || true

run_exec cp_simple /tmp/lab5_src_cp /tmp/lab5_dest_cp || true
if cmp -s /tmp/lab5_src_cp /tmp/lab5_dest_cp; then echo "cp_simple: OK"; else echo "cp_simple: MISMATCH"; fi

run_exec mcat /tmp/lab5_testfile || true

run_exec mmap /tmp/lab5_testfile || true

run_exec mmap /tmp/lab5_testfile HELLO --private || true
echo "mmap private: file hexdump (first lines):"
hexdump -C /tmp/lab5_testfile | head

run_exec mmap /tmp/lab5_testfile --sigbus || echo "mmap --sigbus ended"

echo "-- mmap sleep + /proc/<pid>/maps demo --"
# Start mmap with --sleep so it pauses inside the program; we'll stop it, dump /proc/<pid>/maps, then continue.
if [[ -x "./mmap" ]]; then
	./mmap /tmp/lab5_testfile --sleep &
	MMAP_PID=$!
elif [[ -x "./bin/mmap" ]]; then
	./bin/mmap /tmp/lab5_testfile --sleep &
	MMAP_PID=$!
else
	MMAP_PID=0
fi
sleep 0.5
if ps -p $MMAP_PID > /dev/null 2>&1; then
	echo "Pausing process $MMAP_PID (SIGSTOP) and dumping /proc/$MMAP_PID/maps"
	kill -STOP $MMAP_PID || true
	echo "--- /proc/$MMAP_PID/maps ---"
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

run_exec anon_mmap || true

echo "-- mmap munmap test (may cause SIGSEGV) --"
# The mmap program supports --munmap which unmaps the region then attempts access.
# This may crash the program; we run it to demonstrate behavior.
run_exec mmap /tmp/lab5_testfile --munmap || true

echo "-- writer/reader --"
./writer < /tmp/lab5_testfile > /tmp/lab5_writer.out 2>&1 &
WID=$!
sleep 0.2
run_exec reader > /tmp/lab5_reader.out 2>&1 || true
wait $WID || true

echo "reader hexdump (first lines):"
hexdump -C /tmp/lab5_reader.out | head

echo "=== run_all finished ==="
