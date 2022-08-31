load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
    name = "tiles",
    srcs = ["tiles.cpp"],
    hdrs = ["tiles.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":primes",
        ":strings",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/numeric:int128",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_prod",
        "@glog",
    ],
)

cc_test(
    name = "tiles_test",
    srcs = ["tiles_test.cpp"],
    deps = [
        ":strings",
        ":tiles",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "primes",
    srcs = ["primes.cpp"],
    hdrs = ["primes.h"],
    visibility = ["//visibility:public"],
    deps = ["@glog"],
)

cc_test(
    name = "primes_test",
    srcs = ["primes_test.cpp"],
    deps = [
        ":primes",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "strings",
    srcs = ["strings.cpp"],
    hdrs = ["strings.h"],
    deps = ["@glog"],
)

proto_library(
    name = "tiles_spec_proto",
    srcs = ["tiles_spec.proto"],
)

cc_proto_library(
    name = "tiles_spec_cc_proto",
    deps = [":tiles_spec_proto"],
)