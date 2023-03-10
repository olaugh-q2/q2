load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
    name = "bag",
    srcs = ["bag.cpp"],
    hdrs = ["bag.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":tiles",
        "@glog",
    ],
)

cc_test(
    name = "bag_test",
    srcs = ["bag_test.cpp"],
    data = glob(["testdata/*.textproto"]),
    deps = [
        ":bag",
        ":tiles",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "board_layout",
    srcs = ["board_layout.cpp"],
    hdrs = ["board_layout.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":board",
        ":board_spec_cc_proto",
        ":tiles",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/types:optional",
        "@glog",
    ],
)

proto_library(
    name = "board_spec_proto",
    srcs = ["board_spec.proto"],
)

cc_test(
    name = "board_layout_test",
    srcs = ["board_layout_test.cpp"],
    data = glob(["testdata/*.textproto"]),
    deps = [
        ":board_layout",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_proto_library(
    name = "board_spec_cc_proto",
    deps = [":board_spec_proto"],
)

cc_library(
    name = "board",
    srcs = ["board.cpp"],
    hdrs = ["board.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":move",
        ":primes",
        ":strings",
        ":tiles",
    ],
)

cc_library(
    name = "tiles",
    srcs = ["tiles.cpp"],
    hdrs = ["tiles.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":primes",
        ":strings",
        ":tiles_spec_cc_proto",
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
    data = glob(["testdata/*.textproto"]),
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
    visibility = ["//visibility:public"],
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

cc_library(
    name = "move",
    srcs = ["move.cpp"],
    hdrs = ["move.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":tiles",
        "@com_google_absl//absl/status:statusor",
        "@glog",
    ],
)

cc_test(
    name = "move_test",
    srcs = ["move_test.cpp"],
    data = glob(["testdata/*.textproto"]),
    deps = [
        ":move",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "move_finder",
    srcs = ["move_finder.cpp"],
    hdrs = ["move_finder.h"],
    copts = ["-std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        ":bag",
        ":board",
        ":board_layout",
        ":move",
        ":primes",
        ":rack",
        ":tiles",
        "//src/anagram:anagram_map",
        "//src/leaves:leaves",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/types:optional",
        "@glog",
        "@range-v3",
    ],
)

cc_test(
    name = "move_finder_test",
    srcs = ["move_finder_test.cpp"],
    data = glob([
        "testdata/*.qam",
        "testdata/*.qlv",
        "testdata/*.textproto",
    ]),
    deps = [
        ":move_finder",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)

cc_binary(
    name = "move_finder_benchmark",
    srcs = ["move_finder_benchmark.cpp"],
    data = glob([
        "testdata/*.qam",
        "testdata/*.qlv",
        "testdata/*.textproto",
    ]),
    deps = [
        ":move_finder",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@glog",
    ],
)

cc_library(
    name = "rack",
    srcs = ["rack.cpp"],
    hdrs = ["rack.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":primes",
        ":strings",
        ":tiles",
        "@com_google_absl//absl/numeric:int128",
        "@glog",
    ],
)

cc_test(
    name = "rack_test",
    srcs = ["rack_test.cpp"],
    data = glob(["testdata/*.textproto"]),
    deps = [
        ":rack",
        ":tiles",
        "@com_google_googletest//:gtest_main",
    ],
)