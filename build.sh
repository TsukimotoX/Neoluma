#!/usr/bin/env bash
set -euo pipefail

# lowkey don't know about bash

# ---- Colors ----
ESC=$'\033'
INFO="${ESC}[38;2;232;75;133m[INFO]${ESC}[0m"
OK="${ESC}[38;2;117;255;135m[SUCCESS]${ESC}[0m"
ERR="${ESC}[38;2;255;80;80m[ERROR]${ESC}[0m"
DONE="${ESC}[38;2;117;255;135m[DONE]${ESC}[0m"

MODE="${1:-release}"
TARGET="${2:-neoluma}"

CFG_PRESET="release-ninja"
BUILD_PRESET="release"
CONFIG="Release"

if [[ "${MODE,,}" == "debug" ]]; then
  CFG_PRESET="debug-ninja"
  BUILD_PRESET="debug"
  CONFIG="Debug"
fi

echo "${INFO} Configure preset: ${CFG_PRESET}"
echo "${INFO} Build preset: ${BUILD_PRESET}"
echo "${INFO} Target: ${TARGET}"

echo "${INFO} Configuring..."
cmake --preset "${CFG_PRESET}"

copy_installer_artifact() {
  local config="$1"
  local installer_dir="src/Installer"

  local dest_dir=".build/.executables/${config}"
  mkdir -p "${dest_dir}"

  # Typical Tauri output locations (v1 & v2-ish)
  # We'll pick the newest matching artifact.
  local candidates=(
    "${installer_dir}/src-tauri/target/release/bundle"
    "${installer_dir}/src-tauri/target/debug/bundle"
    "${installer_dir}/src-tauri/target/release"
    "${installer_dir}/src-tauri/target/debug"
  )

  # Find likely artifacts:
  # - Linux: .AppImage, .deb, .rpm
  # - macOS: .dmg, .app
  # - fallback: any file containing "installer" or "neoluma" (not dirs)
  local found=""
  local newest_mtime=0

  for base in "${candidates[@]}"; do
    [[ -e "$base" ]] || continue

    while IFS= read -r -d '' f; do
      # Skip debug symbols / irrelevant stuff
      [[ -f "$f" ]] || continue

      local bn
      bn="$(basename "$f")"

      # prefer installer-like names and common bundle extensions
      if [[ "$bn" =~ neoluma|installer ]]; then
        :
      fi

      local ext="${bn##*.}"
      local score=0
      case "$bn" in
        *.AppImage) score=90 ;;
        *.deb|*.rpm) score=80 ;;
        *.dmg) score=80 ;;
        *.app) score=70 ;; # .app is usually dir, but keep just in case
        *) score=10 ;;
      esac

      # mtime (GNU stat vs BSD stat)
      local mt=0
      if stat --version >/dev/null 2>&1; then
        mt="$(stat -c %Y "$f" 2>/dev/null || echo 0)"
      else
        mt="$(stat -f %m "$f" 2>/dev/null || echo 0)"
      fi

      # choose newest with decent score
      if (( mt > newest_mtime )) && (( score >= 10 )); then
        newest_mtime=$mt
        found="$f"
      fi
    done < <(find "$base" -maxdepth 6 -type f \( \
        -name "*.AppImage" -o -name "*.deb" -o -name "*.rpm" -o -name "*.dmg" -o \
        -iname "*neoluma*" -o -iname "*installer*" \
      \) -print0 2>/dev/null || true)
  done

  if [[ -z "$found" ]]; then
    echo "${ERR} Couldn't find installer artifact in Tauri output."
    echo "${ERR} Looked in:"
    for c in "${candidates[@]}"; do echo "  - $c"; done
    return 1
  fi

  local out_name
  out_name="$(basename "$found")"

  echo "${INFO} Found installer artifact: ${found}"
  cp -f "$found" "${dest_dir}/${out_name}"
  echo "${OK} Installer copied to ${dest_dir}/${out_name}"
}

do_neoluma() {
  echo "${INFO} Building neoluma..."
  cmake --build --preset "${BUILD_PRESET}" --target neoluma

  local exe=".build/.executables/${CONFIG}/neoluma"
  # If your CMake outputs different name/location on unix, adjust here.
  if [[ -f "$exe" ]]; then
    echo "${OK} Built: ${exe}"
  else
    # fallback search
    local f
    f="$(find ".build/.executables/${CONFIG}" -maxdepth 2 -type f -iname "neoluma*" -print -quit 2>/dev/null || true)"
    if [[ -n "$f" ]]; then
      echo "${OK} Built: ${f}"
    else
      echo "${ERR} Can't find built neoluma in .build/.executables/${CONFIG}"
      return 1
    fi
  fi
}

do_payload() {
  echo "${INFO} Building payload..."
  cmake --build --preset "${BUILD_PRESET}" --target payload
}

do_tauri() {
  echo "${INFO} Building Tauri installer..."
  pushd "src/Installer" >/dev/null

  # npm must exist; using npm run tauri build like your .bat
  npm run tauri build

  popd >/dev/null

  copy_installer_artifact "${CONFIG}"
}

case "${TARGET,,}" in
  neoluma)
    do_neoluma
    ;;
  payload)
    do_payload
    ;;
  installer)
    do_payload
    do_tauri
    ;;
  all)
    do_payload
    do_tauri
    ;;
  *)
    echo "${ERR} Unknown target: ${TARGET}"
    echo "   valid targets: neoluma, payload, installer, all"
    exit 1
    ;;
esac

echo "${DONE}"