# common bash functions used by multiple utilities

make_quiet() {
  # make a target only if make thinks it's necessary (outputs nothing if not)
  targets=()
  args=()
  while [[ $# -gt 0 ]]; do
    args+=("$1")
    case $1 in
      -*)
        # allow any number of options
        ;;
      *)
        targets+=("$1")
        ;;
    esac
    shift
  done
  set -- "${args[@]}"
  if [[ ${#targets[@]} -gt 1 ]]; then
    >&2 echo "Internal error: make_quiet should only be passed one target"
    return 1
  fi
  make -q "${targets[@]}" 2>/dev/null || make "$@"
}
