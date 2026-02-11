#!/usr/bin/env bash
set -euo pipefail

repo_url="${1:-https://github.com/limine-bootloader/limine.git}"
branch="${2:-v9.x-binary}"
dest_dir="${3:-tools/limine}"

if [ -f "$dest_dir/limine" ] && [ -f "$dest_dir/limine-bios.sys" ]; then
    exit 0
fi

rm -rf "$dest_dir"
git clone --depth 1 --branch "$branch" "$repo_url" "$dest_dir"
