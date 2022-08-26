cc_library(
    name = "anagram_map",
    srcs = ["anagram_map.cpp"],
    hdrs = ["anagram_map.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/scrabble:tiles",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/numeric:int128",
        "@glog",
    ],
)

cc_test(
    name = "anagram_map_test",
    srcs = ["anagram_map_test.cpp"],
    data = glob(["testdata/*.txt"]),
    deps = [
        ":anagram_map",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
        "@glog",
        "@range-v3",
    ],
)
