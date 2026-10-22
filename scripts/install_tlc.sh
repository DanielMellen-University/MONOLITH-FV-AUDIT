#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
TOOLS_DIR="$ROOT/tools"
VERSION="v1.8.0"
EXPECTED_SHA1="ed814d7792505bbd141096ae66a0c4c5d878d951"
JAR="$TOOLS_DIR/tla2tools.jar"
URL="https://github.com/tlaplus/tlaplus/releases/download/$VERSION/tla2tools.jar"

mkdir -p "$TOOLS_DIR"
curl --fail --location --silent --show-error "$URL" -o "$JAR"
actual_sha1="$(sha1sum "$JAR" | cut -d ' ' -f 1)"
if [[ "$actual_sha1" != "$EXPECTED_SHA1" ]]; then
    rm -f "$JAR"
    printf 'TLA+ tools checksum mismatch: expected %s, got %s\n' \
        "$EXPECTED_SHA1" "$actual_sha1" >&2
    exit 1
fi

printf 'Installed TLA+ tools %s at %s\n' "$VERSION" "$JAR"
printf 'SHA-1: %s\n' "$actual_sha1"
sha256sum "$JAR"
