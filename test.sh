#!/usr/bin/env bash

set -e

FILE="${1:-10gb_file.img}"
SIZE_MB=10240

echo "Creating $FILE (10GB)..."

dd if=/dev/zero of="$FILE" bs=1M count=$SIZE_MB status=progress conv=fsync

echo "Done."
