_zen_common_cpp_srcs = [
  'src/logger.cc'
]

_zen_common_cpp_inc = include_directories('include')

_zen_common_cpp_deps = [
  zen_common_dep,
]

_zen_common_cpp_lib = static_library(
  'zen-common-cpp',
  _zen_common_cpp_srcs,
  install: false,
  dependencies: _zen_common_cpp_deps,
  include_directories: _zen_common_cpp_inc,
)

zen_common_cpp_dep = declare_dependency(
  link_with: _zen_common_cpp_lib,
  include_directories: _zen_common_cpp_inc,
)

