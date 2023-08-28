load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    data = glob(["data/*.textproto"]),
    deps = [
        "//src/scrabble:board",
        "//src/scrabble:board_layout",
        "//src/scrabble:move",
        "@glog",
    ],
)

cc_binary(
    name = "neural_network",
    srcs = ["neural_network.cpp"],
    data = glob(["data/*.textproto"]),
    deps = [
        ":neural_network_lib",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@glog",
    ],
)

cc_library(
    name = "neural_network_lib",
    srcs = ["neural_network_lib.cpp"],
    hdrs = ["neural_network_lib.h"],
    deps = [
        "@glog",
    ],
)

cc_test(
    name = "neural_network_test",
    srcs = ["neural_network_test.cpp"],
    deps = [
        ":neural_network_lib",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)
