#!/usr/bin/env bash
# Kerrigan Build Script -- handles toolchain setup on Ubuntu 18.04-24.04, Debian 11-12, Fedora 38+
# Usage: ./build.sh [--with-gui] [--with-wallet] [--jobs N]
#
# This script:
# 1. Detects your OS and installs the correct compiler + dependencies
# 2. Installs Rust + cxxbridge if missing
# 3. Configures and builds kerrigand
#
# Requires: sudo access for package installation

set -euo pipefail

# --- Color output (defined early so argument parsing can use error()) ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }

# --- Parse arguments ---
WITH_GUI=0
WITH_WALLET=0
JOBS=$(nproc 2>/dev/null || echo 2)

while [ $# -gt 0 ]; do
    case "$1" in
        --with-gui) WITH_GUI=1 ;;
        --with-wallet) WITH_WALLET=1 ;;
        --jobs)
            shift
            if [ $# -eq 0 ]; then
                error "--jobs requires a numeric argument"
            fi
            JOBS="$1"
            ;;
        --jobs=*) JOBS="${1#*=}" ;;
        --help|-h)
            echo "Usage: $0 [--with-gui] [--with-wallet] [--jobs N]"
            echo "  Default: headless daemon, no wallet, $(nproc 2>/dev/null || echo 2) parallel jobs"
            exit 0
            ;;
        *) error "Unknown option: $1. Use --help for usage." ;;
    esac
    shift
done

# --- Detect OS ---
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS_ID="${ID}"
        OS_VERSION="${VERSION_ID}"
        OS_NAME="${PRETTY_NAME}"
    elif [ -f /etc/redhat-release ]; then
        OS_ID="rhel"
        OS_VERSION=$(grep -oE '[0-9]+\.[0-9]+' /etc/redhat-release | head -1)
        OS_NAME=$(cat /etc/redhat-release)
    else
        error "Cannot detect OS. Supported: Ubuntu 18.04-24.04, Debian 11-12, Fedora 38+, Rocky/Alma 9+"
    fi
    info "Detected: $OS_NAME"
}

# --- Check GCC version ---
check_gcc() {
    local gcc_cmd="${1:-g++}"
    if ! command -v "$gcc_cmd" &>/dev/null; then
        echo "0"
        return
    fi
    "$gcc_cmd" -dumpversion | cut -d. -f1
}

# --- Install system dependencies ---
install_deps() {
    case "$OS_ID" in
        ubuntu|linuxmint|pop)
            install_deps_ubuntu
            ;;
        debian)
            install_deps_debian
            ;;
        fedora)
            install_deps_fedora
            ;;
        rocky|almalinux|rhel|centos)
            install_deps_rhel
            ;;
        arch|manjaro)
            install_deps_arch
            ;;
        *)
            error "Unsupported OS: $OS_ID $OS_VERSION. See doc/build-unix.md for manual instructions."
            ;;
    esac
}

install_deps_ubuntu() {
    local gcc_ver
    local gcc_major

    # Determine which GCC to use
    gcc_major=$(check_gcc g++)
    info "System GCC version: ${gcc_major:-not installed}"

    local need_ppa=0
    local gcc_pkg="g++"

    case "${OS_VERSION%%.*}" in
        18)
            # Ubuntu 18.04: GCC 7 default, need PPA for GCC 11
            need_ppa=1
            gcc_pkg="g++-11"
            gcc_ver=11
            ;;
        20)
            # Ubuntu 20.04: GCC 9 default, need PPA for GCC 11
            need_ppa=1
            gcc_pkg="g++-11"
            gcc_ver=11
            ;;
        22)
            # Ubuntu 22.04: GCC 11 default -- works
            gcc_ver=11
            if [ "$gcc_major" -lt 11 ] 2>/dev/null; then
                gcc_pkg="g++-11"
            fi
            ;;
        24)
            # Ubuntu 24.04: GCC 13 default -- works
            gcc_ver=13
            ;;
        *)
            gcc_ver="$gcc_major"
            if [ "$gcc_major" -lt 11 ] 2>/dev/null; then
                warn "Unknown Ubuntu version $OS_VERSION with GCC $gcc_major. Attempting PPA..."
                need_ppa=1
                gcc_pkg="g++-12"
                gcc_ver=12
            fi
            ;;
    esac

    info "Installing system dependencies..."
    sudo apt-get update -qq

    if [ "$need_ppa" -eq 1 ]; then
        info "Adding toolchain PPA for GCC ${gcc_ver}..."
        if ! command -v add-apt-repository &>/dev/null; then
            sudo apt-get install -y software-properties-common
        fi
        sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
        sudo apt-get update -qq
    fi

    local base_pkgs="build-essential libtool autotools-dev automake pkg-config bsdmainutils bison python3 curl"
    local lib_pkgs="libevent-dev libboost-dev libsqlite3-dev libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev"
    local gui_pkgs=""
    local wallet_pkgs=""

    if [ "$WITH_GUI" -eq 1 ]; then
        gui_pkgs="qtbase5-dev qttools5-dev qttools5-dev-tools qtwayland5 libqrencode-dev"
    fi
    if [ "$WITH_WALLET" -eq 1 ]; then
        wallet_pkgs="libdb-dev libdb++-dev"
    fi

    # libbacktrace: only available in Ubuntu 25.04+ / Debian 13+
    local backtrace_pkg=""
    if apt-cache show libbacktrace-dev &>/dev/null 2>&1; then
        backtrace_pkg="libbacktrace-dev"
    else
        info "libbacktrace-dev not available -- will build without stack traces"
    fi

    sudo apt-get install -y $base_pkgs $gcc_pkg $lib_pkgs $gui_pkgs $wallet_pkgs $backtrace_pkg

    # Set up alternatives if we installed a specific GCC version
    if [ "$gcc_pkg" != "g++" ]; then
        local gcc_num="${gcc_pkg#g++-}"
        info "Setting GCC $gcc_num as default compiler..."
        sudo update-alternatives --install /usr/bin/gcc gcc "/usr/bin/gcc-${gcc_num}" 100 2>/dev/null || true
        sudo update-alternatives --install /usr/bin/g++ g++ "/usr/bin/g++-${gcc_num}" 100 2>/dev/null || true
        export CC="gcc-${gcc_num}"
        export CXX="g++-${gcc_num}"
    fi

    # Verify GCC version
    local final_gcc
    final_gcc=$(check_gcc "${CXX:-g++}")
    if [ "$final_gcc" -lt 11 ] 2>/dev/null; then
        error "GCC $final_gcc is too old. Need 11+. Try: sudo apt install g++-11"
    fi
    info "Using GCC $final_gcc"
}

install_deps_debian() {
    local gcc_major
    gcc_major=$(check_gcc g++)

    local gcc_pkg="g++"
    if [ "${OS_VERSION%%.*}" -le 11 ] 2>/dev/null && [ "$gcc_major" -lt 11 ] 2>/dev/null; then
        info "Debian $OS_VERSION: enabling backports for GCC 12..."
        echo "deb http://deb.debian.org/debian ${VERSION_CODENAME}-backports main" | sudo tee /etc/apt/sources.list.d/backports.list
        sudo apt-get update -qq
        gcc_pkg="g++-12"
        sudo apt-get install -y -t "${VERSION_CODENAME}-backports" "$gcc_pkg" "gcc-12"
        sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100
        sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100
        export CC=gcc-12
        export CXX=g++-12
    fi

    sudo apt-get update -qq
    sudo apt-get install -y build-essential libtool autotools-dev automake pkg-config \
        bsdmainutils bison python3 curl libevent-dev libboost-dev libsqlite3-dev \
        libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev "$gcc_pkg"
}

install_deps_fedora() {
    sudo dnf install -y gcc-c++ libtool make autoconf automake python3 curl \
        libevent-devel boost-devel sqlite-devel gmp-devel libsodium-devel \
        zeromq-devel miniupnpc-devel libnatpmp-devel
}

install_deps_rhel() {
    info "Installing GCC toolset 12..."
    sudo dnf install -y gcc-toolset-12-gcc-c++ libtool make autoconf automake python3 curl \
        libevent-devel boost-devel sqlite-devel gmp-devel libsodium-devel \
        zeromq-devel miniupnpc-devel
    export CC=/opt/rh/gcc-toolset-12/root/usr/bin/gcc
    export CXX=/opt/rh/gcc-toolset-12/root/usr/bin/g++
    export PATH="/opt/rh/gcc-toolset-12/root/usr/bin:$PATH"
}

install_deps_arch() {
    sudo pacman -Sy --needed --noconfirm base-devel libtool autoconf automake pkg-config python \
        boost libevent sqlite gmp libsodium zeromq miniupnpc libnatpmp curl
}

# --- Install Rust ---
install_rust() {
    if command -v rustc &>/dev/null; then
        local rust_ver
        rust_ver=$(rustc --version | grep -oE '[0-9]+\.[0-9]+' | head -1)
        local rust_major="${rust_ver%%.*}"
        local rust_minor="${rust_ver#*.}"
        if [ "$rust_major" -ge 1 ] && [ "$rust_minor" -ge 81 ] 2>/dev/null; then
            info "Rust $rust_ver already installed (>= 1.81)"
        else
            warn "Rust $rust_ver too old (need 1.81+). Updating via rustup..."
            rustup update stable
        fi
    else
        info "Installing Rust via rustup..."
        curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
        # shellcheck source=/dev/null
        source "$HOME/.cargo/env"
    fi

    # Ensure cargo is in PATH for this session
    if ! command -v cargo &>/dev/null; then
        if [ -f "$HOME/.cargo/env" ]; then
            # shellcheck source=/dev/null
            source "$HOME/.cargo/env"
        else
            error "cargo not found after Rust install. Try: source ~/.cargo/env"
        fi
    fi

    info "Rust $(rustc --version | cut -d' ' -f2)"
}

# --- Install cxxbridge ---
install_cxxbridge() {
    local required_ver="1.0.186"
    if command -v cxxbridge &>/dev/null; then
        local cur_ver
        cur_ver=$(cxxbridge --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' || echo "0")
        if [ "$cur_ver" = "$required_ver" ]; then
            info "cxxbridge $required_ver already installed"
            return
        else
            warn "cxxbridge $cur_ver installed, need exactly $required_ver. Reinstalling..."
        fi
    else
        info "Installing cxxbridge $required_ver..."
    fi

    cargo install cxxbridge-cmd --version "$required_ver" --force
    info "cxxbridge $(cxxbridge --version 2>/dev/null || echo 'installed')"
}

# --- Configure ---
configure_kerrigan() {
    info "Running autogen.sh..."
    ./autogen.sh

    local configure_flags=()

    if [ "$WITH_GUI" -eq 0 ]; then
        configure_flags+=(--without-gui)
    fi
    if [ "$WITH_WALLET" -eq 0 ]; then
        configure_flags+=(--disable-wallet)
    else
        configure_flags+=(--with-incompatible-bdb)
    fi
    configure_flags+=(--disable-tests --disable-bench)

    # Check if libbacktrace is available
    if ! pkg-config --exists libbacktrace 2>/dev/null && \
       ! [ -f /usr/include/backtrace.h ] && \
       ! [ -f /usr/local/include/backtrace.h ]; then
        configure_flags+=(--disable-stacktraces)
    fi

    # Pass through CC/CXX if set
    local env_flags=()
    if [ -n "${CC:-}" ]; then
        env_flags+=("CC=$CC")
    fi
    if [ -n "${CXX:-}" ]; then
        env_flags+=("CXX=$CXX")
    fi

    info "Configuring with: ${configure_flags[*]}"
    ./configure "${env_flags[@]}" "${configure_flags[@]}"
}

# --- Build ---
build_kerrigan() {
    info "Building with $JOBS parallel jobs..."

    # Check available RAM -- Rust/bellman needs ~2GB
    local mem_kb
    mem_kb=$(grep MemTotal /proc/meminfo 2>/dev/null | awk '{print $2}' || echo 0)
    local mem_gb=$((mem_kb / 1024 / 1024))
    if [ "$mem_gb" -lt 2 ]; then
        warn "Low memory detected (${mem_gb}GB). Rust compilation may fail."
        warn "Consider adding swap: sudo fallocate -l 4G /swapfile && sudo mkswap /swapfile && sudo swapon /swapfile"
        if [ "$JOBS" -gt 2 ]; then
            JOBS=2
            warn "Reducing parallel jobs to $JOBS to save memory"
        fi
    fi

    if ! make -j"$JOBS"; then
        warn "Build failed. Retrying with single job (may fix Rust bridge race)..."
        make -j1
    fi

    info "Build complete!"
    ls -lh src/kerrigand src/kerrigan-cli 2>/dev/null || true
}

# --- Main ---
main() {
    info "Kerrigan Build Script"
    info "Options: GUI=$WITH_GUI, Wallet=$WITH_WALLET, Jobs=$JOBS"
    echo ""

    detect_os
    install_deps
    install_rust
    install_cxxbridge
    configure_kerrigan
    build_kerrigan

    echo ""
    info "Binary: $(pwd)/src/kerrigand"
    info "Next: see doc/pool-integration.md for pool operator setup"
}

main
