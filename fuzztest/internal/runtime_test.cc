// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "./fuzztest/internal/runtime.h"

#if defined(__linux__)
#include <signal.h>
#endif

#include <string>
#include <tuple>
#include <utility>

#include "gtest/gtest.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "./fuzztest/domain_core.h"
#include "./fuzztest/internal/configuration.h"
#include "./fuzztest/internal/test_protobuf.pb.h"

namespace fuzztest::internal {
namespace {

TEST(OnFailureTest, Output) {
  auto& runtime = Runtime::instance();
  const auto get_failure = [&] {
    std::string s;
    runtime.PrintReport(&s);
    return s;
  };
  // Disabled by default.
  EXPECT_EQ(get_failure(), "");

  FuzzTest test({"SUITE_NAME", "TEST_NAME", "FILE", 123}, nullptr);
  Configuration configuration;
  std::tuple args(17, std::string("ABC"));
  const RuntimeStats stats = {absl::FromUnixNanos(0), 1, 2, 3, 4, 5};
#if defined(__linux__)
  static int old_sigtrap_count = 0;
  struct sigaction old_sigtrap = {};
  old_sigtrap.sa_sigaction = [](int, siginfo_t*, void*) {
    ++old_sigtrap_count;
    // Simulate sanitizer death callback running after HandleCrash chains to
    // the previous signal handler.
    Runtime::instance().PrintReportOnDefaultSink();
  };
  old_sigtrap.sa_flags = SA_SIGINFO;
  ASSERT_EQ(sigaction(SIGTRAP, &old_sigtrap, nullptr), 0);
#endif
  runtime.EnableReporter(&stats, [] { return absl::FromUnixNanos(1979); });
  int crash_metadata_listener_calls = 0;
  runtime.RegisterCrashMetadataListener(
      [&](absl::string_view, auto) { ++crash_metadata_listener_calls; });
  runtime.SetRunMode(RunMode::kFuzz);
  UntypedDomain domain = TupleOf(Arbitrary<int>(), Arbitrary<std::string>());
  GenericDomainCorpusType generic_args(
      std::in_place_type<std::tuple<int, std::string>>, args);
  Runtime::Args debug_args{generic_args, domain};
  runtime.SetCurrentTest(&test, &configuration);
  runtime.SetCurrentArgs(&debug_args);
  const std::string report = get_failure();
  EXPECT_EQ(crash_metadata_listener_calls, 1);

  EXPECT_TRUE(absl::StartsWith(report, R"(
=================================================================
=== Fuzzing stats

Elapsed time: 1.979us
Total runs: 1
Edges covered: 2
Total edges: 3
Corpus size: 4
Max stack used: 5

=================================================================
=== BUG FOUND!

FILE:123: Counterexample found for SUITE_NAME.TEST_NAME.
The test fails with input:
argument 0: 17
argument 1: "ABC"
)"));

  EXPECT_TRUE(absl::StrContains(report, R"(
=================================================================
=== Regression test draft

TEST(SUITE_NAME, TEST_NAMERegression) {
  TEST_NAME(
    17,
    "ABC"
  );
}

Please note that the code generated above is best effort and is intended
to be used as a draft regression test.
For reproducing findings please rely on file based reproduction.

=================================================================
)"));

  // Once a report is printed, subsequent PrintReport calls or crashing signals
  // in the same EnableReporter session must not emit a duplicate report.
  EXPECT_EQ(get_failure(), "");
  EXPECT_EQ(crash_metadata_listener_calls, 1);
#if defined(__linux__)
  ASSERT_EQ(raise(SIGTRAP), 0);
  EXPECT_EQ(old_sigtrap_count, 1);
  EXPECT_EQ(crash_metadata_listener_calls, 1);

  // Re-enabling the reporter allows printing again, and when HandleCrash chains
  // to an old handler that triggers the death callback, the report is emitted
  // only once.
  runtime.EnableReporter(&stats, [] { return absl::FromUnixNanos(1979); });
  ASSERT_EQ(raise(SIGTRAP), 0);
  EXPECT_EQ(old_sigtrap_count, 2);
  EXPECT_EQ(crash_metadata_listener_calls, 2);
#endif

  runtime.EnableReporter(&stats, [] { return absl::FromUnixNanos(1979); });
  runtime.DisableReporter();
  EXPECT_EQ(get_failure(), "");
}

TEST(RegressionTestNameForCrashingInputTest,
     ReturnsRegressionTestNameForValidInputFilename) {
  const absl::StatusOr<std::string> regression_test_name =
      RegressionTestNameForCrashingInput("MySuite.MyTest",
                                         "/path/to/bug_id-crash_sig-input_sig");
  ASSERT_TRUE(regression_test_name.ok());
  EXPECT_EQ(*regression_test_name, "MySuite.MyTest/Regression/bug_id");
}

TEST(RegressionTestNameForCrashingInputTest,
     ReturnsErrorForInvalidInputFilename) {
  const absl::StatusOr<std::string> regression_test_name =
      RegressionTestNameForCrashingInput("MySuite.MyTest",
                                         "single_dash-is_invalid");
  EXPECT_FALSE(regression_test_name.ok());
}

}  // namespace
}  // namespace fuzztest::internal
