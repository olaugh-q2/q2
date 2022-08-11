cc_library(
    name = "anagram_map",
    srcs = ["anagram_map.cpp"],
    hdrs = ["anagram_map.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/numeric:int128",
        "@glog",
    ],
)

cc_test(
    name = "anagram_map_test",
    srcs = ["anagram_map_test.cpp"],
    deps = [
        ":anagram_map",
        "@com_google_googletest//:gtest_main",
    ],
    data = glob(["testdata/*.txt"]),
)