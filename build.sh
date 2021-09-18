#!/bin/bash

# This gets the directory of build.sh
# It should be in the project root
# This allows us to run build.sh from any directory, not just the project root
cd "$(dirname "$0")"

# build project in $project_root/build
cmake --preset=default "${@}"
cmake --build --preset=default
