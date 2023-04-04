load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
    name = "anagram_map",
    srcs = ["anagram_map.cpp"],
    hdrs = ["anagram_map.h"],
    copts = ["-std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        ":anagram_map_cc_proto",
        "//src/scrabble:tiles",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/numeric:int128",
        "@com_google_absl//absl/status",
        "@com_google_protobuf//:protobuf",
        "@glog",
        "@range-v3",
    ],
)

cc_test(
    name = "anagram_map_test",
    srcs = ["anagram_map_test.cpp"],
    copts = ["-std=c++14"],
    data = glob([
        "testdata/*.txt",
        "testdata/*.qam",
        "testdata/*.textproto",
    ]),
    deps = [
        ":anagram_map",
        ":anagram_map_cc_proto",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
        "@glog",
        "@range-v3",
    ],
)

cc_binary(
    name = "anagram_tool",
    srcs = ["anagram_tool.cpp"],
    deps = [
        ":anagram_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@glog",
    ],
)

proto_library(
    name = "anagram_map_proto",
    srcs = ["anagram_map.proto"],
)

cc_proto_library(
    name = "anagram_map_cc_proto",
    deps = [":anagram_map_proto"],
)
