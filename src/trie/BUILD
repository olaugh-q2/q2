cc_library(
    name = "dawg",
    srcs = ["dawg.cpp"],
    hdrs = ["dawg.h"],
    visibility = ["//visibility:public"],
    deps = [
                "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",

        "//src/scrabble:strings",
        "//src/scrabble:tiles",
        "@glog",
    ],
)

cc_test(
    name = "dawg_test",
    srcs = ["dawg_test.cpp"],
    data = glob(["testdata/*.txt", "testdata/*.textproto"]),
    deps = [
        ":dawg",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)