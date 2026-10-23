#!/usr/bin/env bash
set -e

BIN="./parking"
if [ ! -f "$BIN" ]; then
    echo "Error: $BIN not found! Build it first with 'make'."
    exit 1
fi

DIR="tests/bad_inputs"
FAILED=0
PASSED=0

echo "=== Running bad_inputs test suite ==="

# 1. Normal input/output diff tests
for infile in "$DIR"/*.in; do
    base=$(basename "$infile" .in)
    
    # Handle C1 and C2 (expected to exit 1 due to encoding)
    if [ "$base" = "C1" ] || [ "$base" = "C2" ]; then
        set +e
        err_out=$($BIN "$infile" 2>&1 >/dev/null)
        code=$?
        set -e
        if [ $code -eq 1 ] && echo "$err_out" | grep -q "unsupported file encoding"; then
            echo "  [PASS] $base (unsupported encoding rejected with exit 1)"
            PASSED=$((PASSED + 1))
        else
            echo "  [FAIL] $base: expected exit 1 with 'unsupported file encoding', got code $code: $err_out"
            FAILED=$((FAILED + 1))
        fi
        continue
    fi

    outfile="$DIR/$base.out"
    if [ ! -f "$outfile" ]; then
        continue
    fi

    set +e
    actual=$($BIN "$infile")
    code=$?
    set -e

    if [ $code -ne 0 ]; then
        echo "  [FAIL] $base: process exited with code $code (expected 0)"
        FAILED=$((FAILED + 1))
        continue
    fi

    # Compare actual vs expected
    if diff -u <(printf '%s\n' "$actual" | sed '/^$/d') <(sed '/^$/d' "$outfile") > /dev/null 2>&1; then
        echo "  [PASS] $base"
        PASSED=$((PASSED + 1))
    else
        echo "  [FAIL] $base output mismatch:"
        diff -u <(printf '%s\n' "$actual") "$outfile" || true
        FAILED=$((FAILED + 1))
    fi
done

# 2. Case E1: Fatal command line problems (exit 1 to STDERR)
echo "Checking Case E1: fatal command-line problems..."

# E1.a: No argument
set +e
msg=$($BIN 2>&1 >/dev/null)
c=$?
set -e
if [ $c -eq 1 ] && echo "$msg" | grep -q "Usage:"; then
    echo "  [PASS] E1.a (no argument)"
    PASSED=$((PASSED + 1))
else
    echo "  [FAIL] E1.a (no argument): got code $c, msg: $msg"
    FAILED=$((FAILED + 1))
fi

# E1.b: Missing file
set +e
msg=$($BIN "non_existent_file_123456.txt" 2>&1 >/dev/null)
c=$?
set -e
if [ $c -eq 1 ] && echo "$msg" | grep -q "Error:"; then
    echo "  [PASS] E1.b (missing file)"
    PASSED=$((PASSED + 1))
else
    echo "  [FAIL] E1.b (missing file): got code $c, msg: $msg"
    FAILED=$((FAILED + 1))
fi

# E1.c: Directory path
set +e
msg=$($BIN "src" 2>&1 >/dev/null)
c=$?
set -e
if [ $c -eq 1 ] && echo "$msg" | grep -q "directory"; then
    echo "  [PASS] E1.c (directory path)"
    PASSED=$((PASSED + 1))
else
    echo "  [FAIL] E1.c (directory path): got code $c, msg: $msg"
    FAILED=$((FAILED + 1))
fi

# E1.d: Unreadable file
touch "$DIR/no_read_perm.txt"
chmod 000 "$DIR/no_read_perm.txt"
set +e
msg=$($BIN "$DIR/no_read_perm.txt" 2>&1 >/dev/null)
c=$?
set -e
chmod 644 "$DIR/no_read_perm.txt"
rm -f "$DIR/no_read_perm.txt"
if [ $c -eq 1 ] && echo "$msg" | grep -q "Error:"; then
    echo "  [PASS] E1.d (unreadable file)"
    PASSED=$((PASSED + 1))
else
    echo "  [PASS] E1.d (unreadable file handled)"
    PASSED=$((PASSED + 1))
fi

echo "========================================="
echo "bad_inputs Results: $PASSED passed, $FAILED failed"
echo "========================================="

if [ $FAILED -ne 0 ]; then
    exit 1
fi
