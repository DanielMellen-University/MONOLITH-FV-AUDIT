#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
TOOLS_DIR="$ROOT/tools"
VERSION="6.9.0"
PACKAGE="ubuntu-22.04-cbmc-${VERSION}-Linux.deb"
EXPECTED_SHA256="5194e76aeff7d4bccc2351abe4b9229b3fca97297ffe685c973211987386db19"
PACKAGE_PATH="$TOOLS_DIR/$PACKAGE"
EXTRACT_ROOT="$TOOLS_DIR/cbmc-${VERSION}-root"
URL="https://github.com/diffblue/cbmc/releases/download/cbmc-${VERSION}/$PACKAGE"

mkdir -p "$TOOLS_DIR" "$EXTRACT_ROOT"
curl --fail --location --silent --show-error "$URL" -o "$PACKAGE_PATH"
actual_sha256="$(sha256sum "$PACKAGE_PATH" | cut -d ' ' -f 1)"
if [[ "$actual_sha256" != "$EXPECTED_SHA256" ]]; then
    rm -f "$PACKAGE_PATH"
    printf 'CBMC package checksum mismatch: expected %s, got %s\n' \
        "$EXPECTED_SHA256" "$actual_sha256" >&2
    exit 1
fi

dpkg-deb --extract "$PACKAGE_PATH" "$EXTRACT_ROOT"
printf 'Installed CBMC %s under %s\n' "$VERSION" "$EXTRACT_ROOT"
printf 'SHA-256: %s\n' "$actual_sha256"
"$EXTRACT_ROOT/usr/bin/cbmc" --version
