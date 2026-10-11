#!/bin/sh
# ==============================================================================
# run_tests.sh - Test suite for ls_custom on NetBSD 10.1 and POSIX systems
# ==============================================================================

# Locate binary (ls_custom or ls_custom.exe)
if [ -x "./ls_custom" ]; then
    LS_BIN="./ls_custom"
elif [ -x "./ls_custom.exe" ]; then
    LS_BIN="./ls_custom.exe"
elif [ -x "../ls_custom" ]; then
    LS_BIN="../ls_custom"
elif [ -x "../ls_custom.exe" ]; then
    LS_BIN="../ls_custom.exe"
else
    echo "Executable ls_custom not found. Attempting to build with make..."
    make
    if [ -x "./ls_custom" ]; then
        LS_BIN="./ls_custom"
    elif [ -x "./ls_custom.exe" ]; then
        LS_BIN="./ls_custom.exe"
    else
        echo "Error: Failed to build ls_custom."
        exit 1
    fi
fi

echo "=========================================================="
echo " Starting ls_custom Test Suite"
echo " Binary under test: $LS_BIN"
echo " Host OS: $(uname -s 2>/dev/null || echo Windows)"
echo "=========================================================="

PASSED=0
FAILED=0
TOTAL=0

# Helper test runner function
run_test() {
    TEST_NAME="$1"
    CMD="$2"
    EXPECTED_EXIT="$3"

    TOTAL=$((TOTAL + 1))
    printf "[TEST %02d] %-50s " "$TOTAL" "$TEST_NAME"

    OUTPUT=$(eval "$CMD" 2>&1)
    ACTUAL_EXIT=$?

    if [ "$ACTUAL_EXIT" -eq "$EXPECTED_EXIT" ]; then
        echo "PASS"
        PASSED=$((PASSED + 1))
    else
        echo "FAIL (Expected exit $EXPECTED_EXIT, got $ACTUAL_EXIT)"
        FAILED=$((FAILED + 1))
        echo "---- Output ----"
        echo "$OUTPUT"
        echo "----------------"
    fi
}

# Setup temporary sandbox for testing
SANDBOX="tests/sandbox"
rm -rf "$SANDBOX" 2>/dev/null
mkdir -p "$SANDBOX"
mkdir -p "$SANDBOX/empty_dir"
mkdir -p "$SANDBOX/sub_dir/nested_sub"
mkdir -p "$SANDBOX/.hidden_dir"

# Populate sandbox files with distinct content and sizes
echo "Small content" > "$SANDBOX/file_small.txt"
echo "Medium content with extra text for size testing" > "$SANDBOX/file_medium.txt"
echo "A very large content file with lots of repeated bytes to ensure size is distinct 1234567890" > "$SANDBOX/file_large.txt"
echo "Hidden file content" > "$SANDBOX/.hidden_file"
echo "Secret in hidden directory" > "$SANDBOX/.hidden_dir/.secret"
echo "Nested file inside sub_dir" > "$SANDBOX/sub_dir/nested_file.txt"
echo "Deep file inside nested_sub" > "$SANDBOX/sub_dir/nested_sub/deep.txt"

# Try to create symlinks if supported by the OS
if ln -s "$SANDBOX/file_small.txt" "$SANDBOX/symlink_file" 2>/dev/null; then
    HAVE_SYMLINKS=1
else
    HAVE_SYMLINKS=0
fi

if [ "$HAVE_SYMLINKS" -eq 1 ]; then
    ln -s "$SANDBOX/sub_dir" "$SANDBOX/symlink_dir" 2>/dev/null
fi

echo "Sandbox initialized at $SANDBOX"
echo ""

# ------------------------------------------------------------------------------
# 1. Basic Listing & Operand Handling
# ------------------------------------------------------------------------------
run_test "Default listing without arguments" "$LS_BIN >/dev/null" 0
run_test "Single regular file operand" "$LS_BIN $SANDBOX/file_small.txt >/dev/null" 0
run_test "Single directory operand" "$LS_BIN $SANDBOX >/dev/null" 0
run_test "Multiple directory operands" "$LS_BIN $SANDBOX/sub_dir $SANDBOX/empty_dir >/dev/null" 0
run_test "Non-existent path returns exit code > 0" "$LS_BIN $SANDBOX/non_existent_file >/dev/null" 1
run_test "Mixed valid and invalid operands" "$LS_BIN $SANDBOX/file_small.txt $SANDBOX/no_such_file >/dev/null" 1

# ------------------------------------------------------------------------------
# 2. Hidden Files: -a and -A
# ------------------------------------------------------------------------------
run_test "Option -a (include . and ..)" "$LS_BIN -a $SANDBOX | grep -q '^\.$'" 0
run_test "Option -a (include hidden files)" "$LS_BIN -a $SANDBOX | grep -q '^\.hidden_file$'" 0
run_test "Option -A (include hidden file)" "$LS_BIN -A $SANDBOX | grep -q '^\.hidden_file$'" 0
run_test "Option -A (excludes . and ..)" "$LS_BIN -A $SANDBOX | grep -qv '^\.\{1,2\}$'" 0

# ------------------------------------------------------------------------------
# 3. Directory Treatment: -d
# ------------------------------------------------------------------------------
run_test "Option -d on directory (lists as plain file)" "$LS_BIN -d $SANDBOX/sub_dir | grep -q 'sub_dir'" 0
run_test "Option -d on multiple directories" "$LS_BIN -d $SANDBOX/sub_dir $SANDBOX/empty_dir >/dev/null" 0

# ------------------------------------------------------------------------------
# 4. Sorting Options: -r, -S, -t, -c, -u, -f
# ------------------------------------------------------------------------------
run_test "Default lexicographical sort" "$LS_BIN $SANDBOX >/dev/null" 0
run_test "Option -r (reverse sort)" "$LS_BIN -r $SANDBOX >/dev/null" 0
run_test "Option -S (sort by size)" "$LS_BIN -S $SANDBOX >/dev/null" 0
run_test "Option -Sr (sort by size reversed)" "$LS_BIN -S -r $SANDBOX >/dev/null" 0
run_test "Option -t (sort by modification time)" "$LS_BIN -t $SANDBOX >/dev/null" 0
run_test "Option -tr (sort by time reversed)" "$LS_BIN -t -r $SANDBOX >/dev/null" 0
run_test "Option -tc (sort by status change time ctime)" "$LS_BIN -t -c $SANDBOX >/dev/null" 0
run_test "Option -tu (sort by access time atime)" "$LS_BIN -t -u $SANDBOX >/dev/null" 0
run_test "Option -f (unsorted output)" "$LS_BIN -f $SANDBOX >/dev/null" 0
run_test "Override -t vs -S (-t -S size wins)" "$LS_BIN -t -S $SANDBOX >/dev/null" 0
run_test "Override -S vs -t (-S -t time wins)" "$LS_BIN -S -t $SANDBOX >/dev/null" 0

# ------------------------------------------------------------------------------
# 5. Long Format: -l and -n
# ------------------------------------------------------------------------------
run_test "Option -l (long format output)" "$LS_BIN -l $SANDBOX | grep -q 'total'" 0
run_test "Option -n (long format with numeric UID/GID)" "$LS_BIN -n $SANDBOX | grep -q 'total'" 0
run_test "Option -l on empty directory" "$LS_BIN -l $SANDBOX/empty_dir | grep -q 'total 0'" 0

# ------------------------------------------------------------------------------
# 6. Metadata Indicators & Formats: -i, -s, -h, -k, -F
# ------------------------------------------------------------------------------
run_test "Option -i (print inode numbers)" "$LS_BIN -i $SANDBOX >/dev/null" 0
run_test "Option -s (print block counts)" "$LS_BIN -s $SANDBOX | grep -q 'total'" 0
run_test "Option -sk (block counts in KB)" "$LS_BIN -sk $SANDBOX | grep -q 'total'" 0
run_test "Option -sh (human-readable block counts)" "$LS_BIN -sh $SANDBOX | grep -q 'total'" 0
run_test "Option -lh (human-readable file sizes)" "$LS_BIN -lh $SANDBOX | grep -q 'total'" 0
run_test "Override -skh (-h wins, human readable)" "$LS_BIN -skh $SANDBOX | grep -q 'total'" 0
run_test "Override -shk (-k wins, kilobytes)" "$LS_BIN -shk $SANDBOX | grep -q 'total'" 0
run_test "Option -F (file classification indicators)" "$LS_BIN -F $SANDBOX | grep -q 'sub_dir/'" 0

# ------------------------------------------------------------------------------
# 7. Non-printable Characters: -q and -w
# ------------------------------------------------------------------------------
run_test "Option -q (non-printable as ?)" "$LS_BIN -q $SANDBOX >/dev/null" 0
run_test "Option -w (raw non-printable)" "$LS_BIN -w $SANDBOX >/dev/null" 0
run_test "Override -qw (-w wins)" "$LS_BIN -q -w $SANDBOX >/dev/null" 0
run_test "Override -wq (-q wins)" "$LS_BIN -w -q $SANDBOX >/dev/null" 0

# ------------------------------------------------------------------------------
# 8. Recursive Traversal: -R
# ------------------------------------------------------------------------------
run_test "Option -R (recursive listing)" "$LS_BIN -R $SANDBOX | grep -q 'sub_dir/nested_sub:'" 0
run_test "Option -R on empty directory" "$LS_BIN -R $SANDBOX/empty_dir >/dev/null" 0
run_test "Option -RA (recursive with hidden files)" "$LS_BIN -RA $SANDBOX | grep -q '\.hidden_dir:'" 0
run_test "Option -Ra (recursive including . and ..)" "$LS_BIN -Ra $SANDBOX | grep -q 'nested_sub:'" 0
run_test "Option -Rl (recursive with long format)" "$LS_BIN -Rl $SANDBOX | grep -q 'total'" 0
run_test "Option -RF (recursive with classification)" "$LS_BIN -RF $SANDBOX | grep -q 'nested_sub/'" 0
run_test "Option -Rr (recursive with reverse sort)" "$LS_BIN -Rr $SANDBOX >/dev/null" 0
run_test "Override -Rd (-d wins, no recursion)" "$LS_BIN -Rd $SANDBOX | grep -qv 'nested_sub:'" 0
run_test "Override -dR (-R wins, recursive)" "$LS_BIN -dR $SANDBOX | grep -q 'nested_sub:'" 0

# ------------------------------------------------------------------------------
# 9. Comparison with System /bin/ls (NetBSD specific)
# ------------------------------------------------------------------------------
if [ -x "/bin/ls" ]; then
    echo ""
    echo "--- System /bin/ls detected. Comparing outputs ---"
    TEST_OUT_DIR="tests/test_output"
    rm -rf "$TEST_OUT_DIR" 2>/dev/null
    mkdir -p "$TEST_OUT_DIR"
    SYS_OUT="$TEST_OUT_DIR/sys_out.txt"
    CUSTOM_OUT="$TEST_OUT_DIR/custom_out.txt"

    /bin/ls -1 "$SANDBOX" | tr -d '\r' > "$SYS_OUT" 2>&1
    $LS_BIN "$SANDBOX" | tr -d '\r' > "$CUSTOM_OUT" 2>&1
    if command -v diff >/dev/null 2>&1; then
        run_test "Comparison with /bin/ls (default short listing)" "diff -u $SYS_OUT $CUSTOM_OUT" 0
    elif command -v cmp >/dev/null 2>&1; then
        run_test "Comparison with /bin/ls (default short listing)" "cmp -s $SYS_OUT $CUSTOM_OUT" 0
    fi
fi

# Cleanup sandbox and temporary outputs
rm -rf "$SANDBOX" "$TEST_OUT_DIR" 2>/dev/null

echo ""
echo "=========================================================="
echo " Test Suite Summary: $PASSED / $TOTAL Passed ($FAILED Failed)"
echo "=========================================================="

if [ "$FAILED" -eq 0 ]; then
    exit 0
else
    exit 1
fi
