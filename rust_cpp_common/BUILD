# Copyright 2026 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Shared runtime, sanitizer interface, coverage, and engine ABI libraries used
# by both the C++ and Rust FuzzTest frameworks.

load("@rules_cc//cc:cc_library.bzl", "cc_library")  # @unused buildcleaner: keep
load("@rules_cc//cc:cc_test.bzl", "cc_test")

DEFAULT_VISIBILITY = ["//visibility:public"]

package(default_visibility = DEFAULT_VISIBILITY)

licenses(["notice"])

### Files

# Centipede runner needs access to these files, since it has special build
# requirements and needs to compile the files directly with specific flags.
exports_files(
    srcs = [
        "coverage_interceptors.cc",
        "reverse_pc_table.h",
        "runner_dl_info.cc",
        "runner_dl_info.h",
        "runtime_interceptors.cc",
        "sancov_callbacks.cc",
        "sancov_object_array.cc",
        "sancov_object_array.h",
        "sancov_runtime.h",
        "sancov_state.cc",
        "sancov_state.h",
    ],
    visibility = ["@com_google_fuzztest//centipede:__pkg__"],
)

### Libraries

# TODO: b/436366556 - Remove and replace once this is standardized.
DISABLE_SANCOV_COPTS = [
    "-fsanitize-coverage=0",
]

# Disable sancov and sanitizer instrumentation.
#
# Need to allow MSAN to avoid false positives when interacting with dependencies.
RUNNER_COPTS = [
    "-fsanitize-coverage=0",
    "-fno-sanitize=address,hwaddress,thread,undefined",
]

cc_library(
    name = "callstack",
    hdrs = ["callstack.h"],
    copts = DISABLE_SANCOV_COPTS,
    # Avoid non-trivial dependencies here, as this library will be linked to target binaries.
    deps = [
        ":rolling_hash",
    ],
)

cc_library(
    name = "engine_abi",
    hdrs = ["engine_abi.h"],
)

cc_library(
    name = "engine_controller_with_subprocess",
    srcs = ["engine_controller_with_subprocess.cc"],
    hdrs = ["engine_controller_abi.h"],
    data = select({
        "@com_google_fuzztest//centipede:is_controller_binary_bundled": [
            "@com_google_fuzztest//centipede:centipede_uninstrumented",
        ],
        "//conditions:default": [],
    }),
    deps = [
        ":engine_abi",
        ":escaping",
    ],
)

cc_library(
    name = "engine_worker",
    srcs = [
        "engine_worker.cc",
    ],
    hdrs = ["engine_worker_abi.h"],
    copts = RUNNER_COPTS,
    deps = [
        ":engine_abi",
        ":execution_metadata",
        ":runner_request",
        ":runner_utils",
        ":shared_memory_blob_sequence",
        "@abseil-cpp//absl/base:nullability",
        "@com_google_fuzztest//centipede:feature",
        "@com_google_fuzztest//centipede:runner_fork_server",
        "@com_google_fuzztest//centipede:runner_result",
        "@com_google_fuzztest//common:defs",
    ],
)

cc_library(
    name = "escaping",
    srcs = ["escaping.cc"],
    hdrs = ["escaping.h"],
    deps = ["@abseil-cpp//absl/strings"],
)

cc_library(
    name = "execution_metadata",
    srcs = ["execution_metadata.cc"],
    hdrs = ["execution_metadata.h"],
    copts = DISABLE_SANCOV_COPTS,
    deps = [
        # This target must have a minimal set of dependencies since it is
        # used in centipede_runner.
        ":shared_memory_blob_sequence",
        "@com_google_fuzztest//common:defs",
    ],
)

cc_library(
    name = "foreach_nonzero",
    hdrs = ["foreach_nonzero.h"],
    copts = DISABLE_SANCOV_COPTS,
)

# This lib must have zero non-trivial dependencies (other than libc).
cc_library(
    name = "int_utils",
    hdrs = ["int_utils.h"],
    copts = DISABLE_SANCOV_COPTS,
)

cc_library(
    name = "interceptor_utils",
    hdrs = ["interceptor_utils.h"],
    copts = DISABLE_SANCOV_COPTS,
    linkopts = ["-ldl"],  # for dlsym
)

cc_library(
    name = "reverse_pc_table",
    hdrs = ["reverse_pc_table.h"],
    # Avoid non-trivial dependencies here, as this library will be linked to target binaries.
    deps = ["@com_google_fuzztest//centipede:pc_info"],
)

cc_library(
    name = "rolling_hash",
    hdrs = ["rolling_hash.h"],
    copts = DISABLE_SANCOV_COPTS,
)

cc_library(
    name = "runner_cmp_trace",
    hdrs = ["runner_cmp_trace.h"],
    copts = DISABLE_SANCOV_COPTS,
    deps = ["@abseil-cpp//absl/base:core_headers"],
)

cc_library(
    name = "runner_request",
    srcs = ["runner_request.cc"],
    hdrs = ["runner_request.h"],
    copts = DISABLE_SANCOV_COPTS,
    deps = [
        # This target must have a minimal set of dependencies since it is
        # used in centipede_runner.
        ":execution_metadata",
        ":shared_memory_blob_sequence",
        "@abseil-cpp//absl/types:span",
        "@com_google_fuzztest//centipede:mutation_data",
        "@com_google_fuzztest//common:defs",
    ],
)

cc_library(
    name = "runner_utils",
    srcs = ["runner_utils.cc"],
    hdrs = ["runner_utils.h"],
    copts = DISABLE_SANCOV_COPTS,
    deps = [
        "@abseil-cpp//absl/base:nullability",
    ],
)

cc_library(
    name = "sancov_runtime",
    srcs = [
        "coverage_interceptors.cc",
        "reverse_pc_table.h",
        "runner_dl_info.cc",
        "runner_dl_info.h",
        "runtime_interceptors.cc",
        "sancov_callbacks.cc",
        "sancov_object_array.cc",
        "sancov_object_array.h",
        "sancov_state.cc",
        "sancov_state.h",
        "@com_google_fuzztest//centipede:pc_info.h",
        "@com_google_fuzztest//common:defs.h",
    ],
    hdrs = [
        "sancov_runtime.h",
    ],
    copts = DISABLE_SANCOV_COPTS,
    deps = [
        ":callstack",
        ":engine_abi",
        ":execution_metadata",
        ":foreach_nonzero",
        ":int_utils",
        ":interceptor_utils",
        ":runner_cmp_trace",
        ":runner_utils",
        "@abseil-cpp//absl/base:core_headers",
        "@abseil-cpp//absl/base:nullability",
        "@abseil-cpp//absl/numeric:bits",
        "@abseil-cpp//absl/types:span",
        "@com_google_fuzztest//centipede:feature",
    ],
)

cc_library(
    name = "sanitizer_interface",
    srcs = ["sanitizer_interface.cc"],
    hdrs = ["sanitizer_interface.h"],
    # Link statically and unconditionally into the main executable so the
    # strong sanitizer error summary hook overrides the default weak definition
    # in the sanitizer runtime (which is also linked statically into the
    # executable).
    linkstatic = True,
    deps = [
        ":symbol_name_utils",
        "@abseil-cpp//absl/base:config",
        "@abseil-cpp//absl/base:core_headers",
        "@abseil-cpp//absl/debugging:stacktrace",
        "@abseil-cpp//absl/debugging:symbolize",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/types:span",
        "@com_google_fuzztest//common:logging",
    ] + select({
        "//conditions:default": [],
    }),
    alwayslink = True,
)

cc_library(
    name = "shared_memory_blob_sequence",
    srcs = ["shared_memory_blob_sequence.cc"],
    hdrs = ["shared_memory_blob_sequence.h"],
    copts = DISABLE_SANCOV_COPTS,
    linkopts = select({
        "@platforms//os:macos": [],
        "//conditions:default": [
            "-lrt",  # for shm_open
        ],
    }),
    deps = ["@abseil-cpp//absl/base:nullability"],
    # don't add any dependencies.
)

cc_library(
    name = "symbol_name_utils",
    hdrs = ["symbol_name_utils.h"],
    deps = ["@abseil-cpp//absl/strings:string_view"],
)

### Tests

cc_test(
    name = "callstack_test",
    srcs = ["callstack_test.cc"],
    deps = [
        ":callstack",
        "@abseil-cpp//absl/base:nullability",
        "@abseil-cpp//absl/container:flat_hash_set",
        "@com_google_fuzztest//common:defs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "execution_metadata_test",
    srcs = ["execution_metadata_test.cc"],
    deps = [
        ":execution_metadata",
        ":shared_memory_blob_sequence",
        "@com_google_fuzztest//common:defs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "foreach_nonzero_test",
    srcs = ["foreach_nonzero_test.cc"],
    deps = [
        ":foreach_nonzero",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "int_utils_test",
    srcs = ["int_utils_test.cc"],
    deps = [
        ":int_utils",
        "@abseil-cpp//absl/container:flat_hash_set",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "reverse_pc_table_test",
    srcs = ["reverse_pc_table_test.cc"],
    deps = [
        ":reverse_pc_table",
        "@com_google_fuzztest//centipede:pc_info",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "rolling_hash_test",
    srcs = ["rolling_hash_test.cc"],
    deps = [
        ":rolling_hash",
        "@com_google_fuzztest//centipede:feature",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "runner_cmp_trace_test",
    srcs = ["runner_cmp_trace_test.cc"],
    deps = [
        ":runner_cmp_trace",
        "@abseil-cpp//absl/base:nullability",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "runner_utils_test",
    srcs = ["runner_utils_test.cc"],
    deps = [
        ":runner_utils",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "sanitizer_interface_test",
    srcs = ["sanitizer_interface_test.cc"],
    deps = [
        ":sanitizer_interface",
        "@abseil-cpp//absl/base:config",
        "@abseil-cpp//absl/base:core_headers",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/types:span",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "shared_memory_blob_sequence_test",
    srcs = ["shared_memory_blob_sequence_test.cc"],
    deps = [
        ":shared_memory_blob_sequence",
        "@googletest//:gtest_main",
    ],
)
