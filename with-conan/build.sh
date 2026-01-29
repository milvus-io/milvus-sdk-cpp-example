#!/usr/bin/env bash

# Licensed to the LF AI & Data foundation under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

BUILD_OUTPUT_DIR="cmake_build"

# Set build type to match Conan profile (Release)
BUILD_TYPE="${BUILD_TYPE:-Release}"

# Create build directory if it doesn't exist
if [[ ! -d ${BUILD_OUTPUT_DIR} ]]; then
  mkdir -p ${BUILD_OUTPUT_DIR}
fi

# Install Conan dependencies into the build directory
echo "Installing Conan dependencies into ${BUILD_OUTPUT_DIR}..."
conan install . --output-folder=${BUILD_OUTPUT_DIR} --build=missing -s compiler.cppstd=14

cd ${BUILD_OUTPUT_DIR}

# remove make cache since build.sh -l use default variables
# force update the variables each time
make rebuild_cache >/dev/null 2>&1

CMAKE_CMD="cmake \
-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
-DBUILD_FROM_CONAN=ON \
../"

echo ${CMAKE_CMD}
${CMAKE_CMD}

make -j8
