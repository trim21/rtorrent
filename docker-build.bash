if [[ $(uname -m) == "aarch64" ]]; then
  sed -i 's/architecture = \"all\"/architecture = \"arm64\"/' BUILD.bazel
elif [[ $(uname -m) == "x86_64" ]]; then
  sed -i 's/architecture = \"all\"/architecture = \"amd64\"/' BUILD.bazel
fi
