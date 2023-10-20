cc_library(
    name = "magpie_rack",
    srcs = ["magpie_rack.cpp"],
    hdrs = ["magpie_rack.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/container:flat_hash_map",        
        "@glog",
    ],
)

cc_test(
    name = "magpie_rack_test",
    srcs = ["magpie_rack_test.cpp"],
    deps = [
        ":magpie_rack",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "ort_tool",
    srcs = ["ort_tool.cpp"],
    deps = [
        ":magpie_rack",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@glog",
    ],
)