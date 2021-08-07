# New C++ Project

## Directory Structure

***include* directory:** This directory contains header files for the project

***src* directory:** This directory contains main files at top-level

***build.sh*:** Contains a simple build script that invokes CMake. *build.sh*
can be run from any directory, not just the project root.
```bash
#!/bin/bash

# This gets the directory of build.sh
# It should be in the project root
# This allows us to run build.sh from any directory, not just the project root
project_root="$(dirname $0)"

# Create build folder in project root
mkdir -p "$project_root/build"
cd "$project_root/build"

# build project in $project_root/build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .
```
