#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# Neoluma Build Script (Linux / WSL)
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
#   --no-install  - do NOT auto-install missing packages (just check + error)
#
# What this script does automatically:
#   - Forces clang-22 + libc++ (needed for <print>/std::println)
#   - Installs missing system deps for:
#       * C++ build: ninja, clang, lld, llvm-dev, libc++/abi, zlib, zstd, curl, edit, xml2
#       * Tauri build (installer/all): glib, gtk3, webkit2gtk, appindicator, rsvg, patchelf
#   - Uses your CMake presets: release-ninja / debug-ninja + build preset release/debug
#
# Commands you can run manually (rough equivalents):
#   - Configure:
#       CC=clang-22 CXX=clang++-22 cmake --preset release-ninja
#   - Build:
#       cmake --build --preset release --target neoluma
#       cmake --build --preset release --target payload
#   - Tauri:
#       (cd src/Installer && npm run tauri build)
# ============================================================

# ---- Pretty colors ----
ESC=$'\033'
INFO="${ESC}[38;2;232;75;133m[INFO]${ESC}[0m"
OK="${ESC}[38;2;117;255;135m[SUCCESS]${ESC}[0m"
ERR="${ESC}[38;2;255;80;80m[ERROR]${ESC}[0m"
DONE="${ESC}[38;2;117;255;135m[DONE]${ESC}[0m"

MODE="${1:-release}"
TARGET="${2:-neoluma}"

AUTO_INSTALL=1
for arg in "$@"; do
  [[ "$arg" == "--no-install" ]] && AUTO_INSTALL=0
done

CFG_PRESET="release-ninja"
BUILD_PRESET="release"
CONFIG="Release"

if [[ "${MODE,,}" == "debug" ]]; then
  CFG_PRESET="debug-ninja"
  BUILD_PRESET="debug"
  CONFIG="Debug"
fi

TARGET_LC="${TARGET,,}"

# ---- Toolchain policy: clang-22 + libc++ (no manual flags) ----
export CC="clang-22"
export CXX="clang++-22"
export CXXFLAGS="${CXXFLAGS:-} -stdlib=libc++"
export LDFLAGS="${LDFLAGS:-} -stdlib=libc++"

have_cmd() { command -v "$1" >/dev/null 2>&1; }

is_wsl() {
  grep -qi microsoft /proc/version 2>/dev/null
}

# ---- Detect package manager ----
PM=""
detect_pm() {
  if have_cmd apt-get; then PM="apt"
  elif have_cmd dnf; then PM="dnf"
  elif have_cmd pacman; then PM="pacman"
  elif have_cmd zypper; then PM="zypper"
  else PM=""
  fi
}

# ---- Package install/check per PM ----
pkg_installed() {
  local pkg="$1"
  case "$PM" in
    apt)    dpkg-query -W -f='${Status}\n' "$pkg" 2>/dev/null | grep -q "install ok installed" ;;
    dnf)    rpm -q "$pkg" >/dev/null 2>&1 ;;
    pacman) pacman -Q "$pkg" >/dev/null 2>&1 ;;
    zypper) rpm -q "$pkg" >/dev/null 2>&1 ;;
    *)      return 1 ;;
  esac
}

pm_install() {
  local pkgs=("$@")

  if (( AUTO_INSTALL == 0 )); then
    echo "${ERR} Missing packages (auto-install disabled via --no-install):"
    printf "   %s\n" "${pkgs[@]}"
    exit 1
  fi

  if [[ -z "$PM" ]]; then
    echo "${ERR} No supported package manager found (apt/dnf/pacman/zypper)."
    echo "${ERR} Please install manually:"
    printf "   %s\n" "${pkgs[@]}"
    exit 1
  fi

  echo "${INFO} Installing missing packages via ${PM}:"
  printf "   %s\n" "${pkgs[@]}"

  case "$PM" in
    apt)
      sudo apt-get update
      sudo apt-get install -y "${pkgs[@]}"
      ;;
    dnf)
      sudo dnf install -y "${pkgs[@]}"
      ;;
    pacman)
      sudo pacman -Sy --noconfirm "${pkgs[@]}"
      ;;
    zypper)
      sudo zypper --non-interactive install "${pkgs[@]}"
      ;;
  esac
}

ensure_pkgs() {
  local desired=("$@")
  local missing=()
  for p in "${desired[@]}"; do
    if ! pkg_installed "$p"; then
      missing+=("$p")
    fi
  done
  (( ${#missing[@]} > 0 )) && pm_install "${missing[@]}"
}

# ---- Distro-specific package name mapping ----
# Notes:
# - We keep names as correct as possible, but distros vary.
# - If your distro differs, install equivalents once; script will stop complaining after.
pkgs_cpp_for_pm() {
  case "$PM" in
    apt)
      cat <<'EOF'
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
EOF
      ;;
    dnf)
      cat <<'EOF'
ninja-build
gcc-c++
pkgconf-pkg-config
clang
lld
llvm-devel
libcxx-devel
libcxxabi-devel
zlib-devel
libzstd-devel
libcurl-devel
libedit-devel
libxml2-devel
EOF
      ;;
    pacman)
      cat <<'EOF'
ninja
base-devel
pkgconf
clang
lld
llvm
libc++
zlib
zstd
curl
libedit
libxml2
EOF
      ;;
    zypper)
      cat <<'EOF'
ninja
patterns-devel-base-devel_basis
pkg-config
clang
lld
llvm-devel
libc++-devel
libc++abi-devel
zlib-devel
libzstd-devel
libcurl-devel
libedit-devel
libxml2-devel
EOF
      ;;
  esac
}

pkgs_tauri_for_pm() {
  case "$PM" in
    apt)
      cat <<'EOF'
nodejs
npm
libglib2.0-dev
libgtk-3-dev
libwebkit2gtk-4.1-dev
libappindicator3-dev
librsvg2-dev
patchelf
EOF
      ;;
    dnf)
      cat <<'EOF'
nodejs
npm
glib2-devel
gtk3-devel
webkit2gtk4.1-devel
libappindicator-gtk3-devel
librsvg2-devel
patchelf
EOF
      ;;
    pacman)
      cat <<'EOF'
nodejs
npm
glib2
gtk3
webkit2gtk-4.1
libappindicator-gtk3
librsvg
patchelf
EOF
      ;;
    zypper)
      cat <<'EOF'
nodejs
npm
glib2-devel
gtk3-devel
webkit2gtk3-devel
libappindicator3-devel
librsvg-devel
patchelf
EOF
      ;;
  esac
}

# ---- npm sanity (WSL trap) ----
ensure_npm_is_linux() {
  if ! have_cmd npm; then
    echo "${ERR} npm not found."
    echo "${ERR} Install Node.js/npm (the script can do it unless --no-install)."
    exit 1
  fi

  local npm_path
  npm_path="$(command -v npm)"

  # In WSL, Windows npm.exe often leaks into PATH => breaks because it uses UNC paths.
  if is_wsl && [[ "$npm_path" == *".exe" ]]; then
    echo "${ERR} Detected Windows npm inside WSL: $npm_path"
    echo "${ERR} Fix: install Linux npm (recommended):"
    echo "      sudo apt-get install -y nodejs npm"
    exit 1
  fi
}

ensure_toolchain_cmds() {
  for t in cmake ninja "${CC}" "${CXX}"; do
    if ! have_cmd "$t"; then
      echo "${ERR} Required tool not in PATH: $t"
      exit 1
    fi
  done
  # nice-to-have; if missing, clang still compiles but dependency scanning might degrade.
  if ! have_cmd clang-scan-deps-22 && have_cmd clang-scan-deps; then
    echo "${INFO} clang-scan-deps-22 not found, but clang-scan-deps exists. OK."
  fi
}

ensure_deps() {
  detect_pm
  if [[ -z "$PM" ]]; then
    echo "${ERR} No supported package manager found (apt/dnf/pacman/zypper)."
    exit 1
  fi

  # ---- C++ deps always (for any target) ----
  mapfile -t CPP_PKGS < <(pkgs_cpp_for_pm)
  ensure_pkgs "${CPP_PKGS[@]}"

  # ---- Tauri deps only for installer/all ----
  if [[ "$TARGET_LC" == "installer" || "$TARGET_LC" == "all" ]]; then
    mapfile -t TAURI_PKGS < <(pkgs_tauri_for_pm)
    ensure_pkgs "${TAURI_PKGS[@]}"
    ensure_npm_is_linux
  fi

  ensure_toolchain_cmds
}

build_payload() {
  echo "${INFO} Building payload..."
  cmake --build --preset "${BUILD_PRESET}" --target payload
}

build_neoluma() {
  echo "${INFO} Building neoluma..."
  cmake --build --preset "${BUILD_PRESET}" --target neoluma

  local out=".build/.executables/${CONFIG}/neoluma"
  if [[ -f "$out" ]]; then
    echo "${OK} Built: ${out}"
  else
    echo "${ERR} Built binary not found at: ${out}"
    exit 1
  fi
}

build_tauri() {
  echo "${INFO} Building Tauri installer..."
  pushd src/Installer >/dev/null
  npm run tauri build
  popd >/dev/null
}

# ---- Run ----
echo "${INFO} Mode: ${MODE}"
echo "${INFO} Target: ${TARGET}"
echo "${INFO} Using toolchain: CC=${CC}, CXX=${CXX} (libc++)"

ensure_deps

echo "${INFO} Configuring..."
cmake --preset "${CFG_PRESET}"

case "$TARGET_LC" in
  neoluma)
    build_neoluma
    ;;
  payload)
    build_payload
    ;;
  installer|all)
    build_payload
    build_tauri
    ;;
  *)
    echo "${ERR} Unknown target: ${TARGET}"
    echo "Valid: neoluma, payload, installer, all"
    exit 1
    ;;
esac

echo "${DONE}"