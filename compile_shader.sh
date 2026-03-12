#!/bin/bash
echo "Compiling shaders..."
echo ""

failed=0
total=0
total_time=0

for shader in shaders/*.vert shaders/*.frag; do
    [ -e "$shader" ] || continue

    ext="${shader##*.}"
    base="${shader%.*}"
    output="${base}_${ext:0:1}.spv"

    size_in=$(wc -c < "$shader")
    start=$(date +%s%3N)

    glslc "$shader" -o "$output"
    status=$?

    end=$(date +%s%3N)
    elapsed=$((end - start))
    total_time=$((total_time + elapsed))
    total=$((total + 1))

    if [ $status -ne 0 ]; then
        echo "  [FAIL] $shader"
        echo "         Time   : ${elapsed}ms"
        echo "         Reason : glslc exited with code $status"
        failed=1
    else
        size_out=$(wc -c < "$output")
        echo "  [OK]   $shader"
        echo "         Output : $output"
        echo "         Time   : ${elapsed}ms"
        echo "         Input  : ${size_in} bytes"
        echo "         Output : ${size_out} bytes (SPIR-V)"
        echo "         Type   : $([ "$ext" = "vert" ] && echo "Vertex shader" || echo "Fragment shader")"
    fi
    echo ""
done

echo "----------------------------------------"
echo "  Total shaders : $total"
echo "  Failed        : $failed"
echo "  Total time    : ${total_time}ms"
echo "----------------------------------------"

if [ $failed -ne 0 ]; then
    echo "ERROR: One or more shaders failed to compile"
    exit 1
fi

echo "All shaders compiled successfully"