load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    data = glob(["data/*.textproto"]),
    deps = [
        "//src/scrabble:board_layout",
        "//src/scrabble:board",
        "@glog",
    ],
)
