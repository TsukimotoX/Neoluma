#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# Neoluma Build Script (Linux)
#
# Usage:
#   ./build.sh [release|debug] [target] [--no-install]
#
# Modes:
#   release (default)
#   debug
#
# Targets:
#   neoluma    - build compiler only
#   payload    - build CMake payload target
#   installer  - build payload + tauri installer
#   all        - same as installer
#
# Flags:
#   --no-install  - do NOT auto-install missing packages
#
# Examples:
#   ./build.sh
#   ./build.sh release neoluma
#   ./build.sh debug payload
#   ./build.sh release installer --no-install
#
# ============================================================

# ---- Colors ----
ESC=$'\033'
INFO="${ESC}[38;2;232;75;133m[INFO]${ESC}[0m"
OK="${ESC}[38;2;117;255;135m[SUCCESS]${ESC}[0m"
ERR="${ESC}[38;2;255;80;80m[ERROR]${ESC}[0m"
DONE="${ESC}[38;2;117;255;135m[DONE]${ESC}[0m"

MODE="${1:-release}"
TARGET="${2:-neoluma}"

AUTO_INSTALL=1
for arg in "$@"; do
  if [[ "$arg" == "--no-install" ]]; then
    AUTO_INSTALL=0
  fi
done

CFG_PRESET="release-ninja"
BUILD_PRESET="release"
CONFIG="Release"

if [[ "${MODE,,}" == "debug" ]]; then
  CFG_PRESET="debug-ninja"
  BUILD_PRESET="debug"
  CONFIG="Debug"
fi

# ---- Force Clang + libc++ (needed for <print> / std::println) ----
export CC="clang-22"
export CXX="clang++-22"
export CXXFLAGS="-stdlib=libc++"
export LDFLAGS="-stdlib=libc++"

have_cmd() { command -v "$1" >/dev/null 2>&1; }

is_debian_like() {
  [[ -f /etc/debian_version ]] && have_cmd apt-get
}

pkg_installed() {
  dpkg-query -W -f='${Status}\n' "$1" 2>/dev/null | grep -q "install ok installed"
}

install_packages() {
  local pkgs=("$@")

  if (( AUTO_INSTALL == 0 )); then
    echo "${ERR} Missing packages:"
    printf "   %s\n" "${pkgs[@]}"
    echo "${ERR} Install them manually or remove --no-install."
    exit 1
  fi

  if ! is_debian_like; then
    echo "${ERR} Auto-install supported only on Debian/Ubuntu."
    echo "Install manually:"
    printf "   %s\n" "${pkgs[@]}"
    exit 1
  fi

  echo "${INFO} Installing missing packages..."
  sudo apt-get update
  sudo apt-get install -y "${pkgs[@]}"
}

ensure_deps() {

  REQUIRED_PKGS=(
    ninja-build
    build-essential
    pkg-config
    clang-22
    lld-22
    llvm-22-dev
    libc++-22-dev
    libc++abi-22-dev
    zlib1g-dev
    libzstd-dev
    libcurl4-openssl-dev
    libedit-dev
    libxml2-dev
  )

  MISSING=()

  for pkg in "${REQUIRED_PKGS[@]}"; do
    if ! pkg_installed "$pkg"; then
      MISSING+=("$pkg")
    fi
  done

  if (( ${#MISSING[@]} > 0 )); then
    install_packages "${MISSING[@]}"
  fi

  for tool in clang-22 clang++-22 cmake ninja; do
    if ! have_cmd "$tool"; then
      echo "${ERR} Required tool missing: $tool"
      exit 1
    fi
  done
}

build_neoluma() {
  echo "${INFO} Building neoluma..."
  cmake --build --preset "${BUILD_PRESET}" --target neoluma

  OUT=".build/.executables/${CONFIG}/neoluma"
  if [[ -f "$OUT" ]]; then
    echo "${OK} Built: $OUT"
  else
    echo "${ERR} neoluma binary not found."
    exit 1
  fi
}

build_payload() {
  echo "${INFO} Building payload..."
  cmake --build --preset "${BUILD_PRESET}" --target payload
}

build_installer() {
  echo "${INFO} Building Tauri installer..."
  pushd src/Installer >/dev/null
  npm run tauri build
  popd >/dev/null
}

# ---- Start ----
echo "${INFO} Mode: ${MODE}"
echo "${INFO} Target: ${TARGET}"

ensure_deps

echo "${INFO} Configuring..."
cmake --preset "${CFG_PRESET}"

case "${TARGET,,}" in
  neoluma)
    build_neoluma
    ;;
  payload)
    build_payload
    ;;
  installer|all)
    build_payload
    build_installer
    ;;
  *)
    echo "${ERR} Unknown target: ${TARGET}"
    echo "Valid: neoluma, payload, installer, all"
    exit 1
    ;;
esac

echo "${DONE}"