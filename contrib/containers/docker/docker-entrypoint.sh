#!/bin/bash
# Copyright (c) 2025-2026 Kerrigan Network
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

set -e

DATADIR="/home/kerrigan/.kerrigan"

# Ensure data directory exists
mkdir -p "${DATADIR}"

# Graceful shutdown: on SIGTERM, ask kerrigand to stop cleanly
_term() {
    echo "Caught SIGTERM, shutting down kerrigand..."
    kerrigan-cli -datadir="${DATADIR}" stop 2>/dev/null || true
    wait "$CHILD" 2>/dev/null
}
trap _term SIGTERM

# If the first argument is kerrigand, run it in the background so we can
# forward signals. Otherwise, exec whatever command was given (useful for
# debugging with /bin/bash, etc.).
if [ "$1" = "kerrigand" ]; then
    shift
    kerrigand -datadir="${DATADIR}" "$@" &
    CHILD=$!
    wait "$CHILD"
    exit $?
fi

exec "$@"
