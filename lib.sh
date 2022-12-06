# common bash functions used by multiple utilities

make_quiet() {
  # make a target only if make thinks it's necessary (outputs nothing if not)
  if [[ $# -gt 1 ]]; then
    >&2 echo "Internal error: make_quiet should only be passed one target"
    return 1
  fi
  make -q "$1" 2>/dev/null || make "$1"
}
