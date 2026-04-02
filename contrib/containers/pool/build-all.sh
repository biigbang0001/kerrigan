#!/usr/bin/env bash
# Build kerrigand binaries for multiple Ubuntu versions using Docker
# Output: ./out/<version>/kerrigand, ./out/<version>/kerrigan-cli
#
# Usage: ./contrib/containers/pool/build-all.sh [18|20|24]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

mkdir -p "$REPO_ROOT/out"

build_target() {
    local ver="$1"
    local tag="kerrigan-build:ubuntu${ver}"
    local dockerfile="$SCRIPT_DIR/Dockerfile.ubuntu${ver}"

    if [ ! -f "$dockerfile" ]; then
        echo "ERROR: $dockerfile not found"
        return 1
    fi

    echo "=== Building for Ubuntu ${ver} ==="
    mkdir -p "$REPO_ROOT/out/ubuntu${ver}"

    docker build \
        -f "$dockerfile" \
        -t "$tag" \
        "$REPO_ROOT"

    docker run --rm \
        -v "$REPO_ROOT/out/ubuntu${ver}:/out" \
        "$tag" \
        bash -c "cp /kerrigan/src/kerrigand /kerrigan/src/kerrigan-cli /kerrigan/src/kerrigan-tx /out/ 2>/dev/null; echo 'Copied to /out'"

    echo "=== Ubuntu ${ver}: $(ls -lh "$REPO_ROOT/out/ubuntu${ver}/kerrigand" 2>/dev/null | awk '{print $5}') ==="
}

# Build requested targets (or all)
targets="${1:-all}"
case "$targets" in
    18) build_target 18 ;;
    20) build_target 20 ;;
    24) build_target 24 ;;
    all)
        build_target 18
        build_target 20
        build_target 24
        echo ""
        echo "=== All builds complete ==="
        ls -lh "$REPO_ROOT"/out/*/kerrigand 2>/dev/null
        ;;
    *)
        echo "Usage: $0 [18|20|24|all]"
        exit 1
        ;;
esac
