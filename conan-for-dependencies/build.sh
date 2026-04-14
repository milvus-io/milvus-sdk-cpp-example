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
SHARED="${SHARED:-OFF}"

# Map SHARED flag to Conan shared options for transitive deps.
if [[ "${SHARED}" == "ON" ]]; then
  CONAN_SHARED_OPTS="-o grpc/*:shared=True -o protobuf/*:shared=True -o abseil/*:shared=True"
else
  CONAN_SHARED_OPTS=""
fi

# Create build directory if it doesn't exist
if [[ ! -d ${BUILD_OUTPUT_DIR} ]]; then
  mkdir -p ${BUILD_OUTPUT_DIR}
fi

# Install Conan dependencies into the build directory
# -s compiler.cppstd=14: host libraries (linked into binary) use C++14 to match milvus-sdk-cpp
# -s:b compiler.cppstd=17: build-time tools (protoc) require abseil/20250127.0 which needs C++17.
#   The default build profile uses gnu17, but ConanCenter only has abseil binaries for 17 (not gnu17).
#   Protobuf's recipe validates that abseil's cppstd matches exactly, so we set 17 explicitly to
#   avoid the "Protobuf and abseil must be built with the same compiler.cppstd setting" error.
echo "Installing Conan dependencies into ${BUILD_OUTPUT_DIR}..."
conan install . --output-folder=${BUILD_OUTPUT_DIR} --build=missing -s compiler.cppstd=14 -s:b compiler.cppstd=17 -s build_type=${BUILD_TYPE} ${CONAN_SHARED_OPTS}

cd ${BUILD_OUTPUT_DIR}

# remove make cache since build.sh -l use default variables
# force update the variables each time
make rebuild_cache >/dev/null 2>&1

CMAKE_CMD="cmake \
-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
-DBUILD_SHARED_LIBS=${SHARED} \
-DBUILD_FROM_CONAN=ON \
../"

echo ${CMAKE_CMD}
${CMAKE_CMD}

# Source conanrun.sh to set LD_LIBRARY_PATH so that build tools (like protoc)
# can find their shared-library dependencies (libabsl_*.so) when SHARED=ON.
# Harmless for static builds.
source conanrun.sh
make -j8
