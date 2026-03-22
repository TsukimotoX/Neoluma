#!/usr/bin/env bash
set -euo pipefail

MARKER_START="# >>> Neoluma installer >>>"
MARKER_END="# <<< Neoluma installer <<<"

ASSUME_YES=0

# ---- Args ----
while [[ $# -gt 0 ]]; do
  case "$1" in
    --yes) ASSUME_YES=1; shift ;;
    -h|--help)
      echo "Neoluma Uninstaller"
      echo "Usage: ./uninstaller.sh [--yes]"
      exit 0
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done

# ---- Detect install prefix (script directory) ----
PREFIX="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if [[ -z "$PREFIX" || ! -d "$PREFIX" ]]; then
  echo "❌ Failed to resolve install directory."
  exit 1
fi

# ---- Safety guard ----
case "$PREFIX" in
  "/"|"/usr"|"/usr/"*|"/bin"|"/sbin"|"/etc"|"/opt"|"/var"|"/tmp"|"/home")
    echo "❌ Refusing to remove dangerous directory: $PREFIX"
    exit 1
    ;;
esac

echo "Neoluma will be removed from:"
echo "  $PREFIX"
echo

if [[ "$ASSUME_YES" -eq 0 ]]; then
  read -r -p "Continue? [y/N] " ans
  case "${ans,,}" in
    y|yes) ;;
    *) echo "Aborted."; exit 0 ;;
  esac
fi

# ---- Remove PATH block from shell rc files ----
remove_block() {
  local file="$1"
  [[ -f "$file" ]] || return 0

  if grep -Fq "$MARKER_START" "$file" && grep -Fq "$MARKER_END" "$file"; then
    echo "🧹 Cleaning PATH in $file"
    if sed --version >/dev/null 2>&1; then
      sed -i.bak "/$MARKER_START/,/$MARKER_END/d" "$file"
    else
      sed -i .bak "/$MARKER_START/,/$MARKER_END/d" "$file"
    fi
  fi
}

HOME_DIR="${HOME:-}"
if [[ -n "$HOME_DIR" ]]; then
  for rc in ".profile" ".bashrc" ".zshrc" ".zprofile"; do
    remove_block "$HOME_DIR/$rc"
  done
fi

# ---- Remove installation directory ----
echo "🗑 Removing files..."
rm -rf -- "$PREFIX"

echo "✅ Neoluma successfully removed."
exit 0