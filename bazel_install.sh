#!/usr/bin/env bash

main() {
    version="v1.12.0"
    dest="/usr/local/bin/bazel"
    sudo wget "https://github.com/bazelbuild/bazelisk/releases/download/${version}/bazelisk-linux-amd64" -O "${dest}"
    sudo chmod +x /usr/local/bin/bazel
    echo "Bazelisk installed as the 'bazel' binary to '${dest}'"
}

main