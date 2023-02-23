cc_library(
    name = "leaves",
    srcs = ["leaves.cpp"],
    hdrs = ["leaves.h"],
    visibility = ["//visibility:public"],
    deps = [
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
    data = glob(["testdata/*.csv", "testdata/*.textproto"]),
    deps = [
        ":leaves",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)
