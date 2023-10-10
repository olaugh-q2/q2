load("@com_google_protobuf//:protobuf.bzl", "cc_proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
    name = "leaves",
    srcs = ["leaves.cpp"],
    hdrs = ["leaves.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":leaves_cc_proto",
        "//src/scrabble:tiles",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_protobuf//:protobuf",
        "@glog",
    ],
)

cc_test(
    name = "leaves_test",
    srcs = ["leaves_test.cpp"],
    data = glob([
        "testdata/*.csv",
        "testdata/*.textproto",
        "testdata/*.qlv"
    ]),
    deps = [
        ":leaves",
        ":leaves_cc_proto",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)

proto_library(
    name = "leaves_proto",
    srcs = ["leaves.proto"],
)

cc_proto_library(
    name = "leaves_cc_proto",
    deps = [":leaves_proto"],
)

cc_binary(
    name = "leaves_tool",
    srcs = ["leaves_tool.cpp"],
    deps = [
        ":leaves",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@glog",
    ],
)
