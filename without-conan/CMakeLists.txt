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

cmake_minimum_required(VERSION 3.14)
project(milvus_sdk_example LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

include_guard(GLOBAL)

include(FetchContent)

FetchContent_Declare(
    milvus-sdk
    GIT_REPOSITORY    https://github.com/milvus-io/milvus-sdk-cpp.git
    GIT_TAG           2.6
    GIT_SHALLOW       TRUE
    GIT_PROGRESS      TRUE
    UPDATE_DISCONNECTED TRUE
)

FetchContent_MakeAvailable(milvus-sdk)

message(STATUS "  milvus-sdk_SOURCE_DIR: ${milvus-sdk_SOURCE_DIR}")
message(STATUS "  milvus-sdk_BINARY_DIR: ${milvus-sdk_BINARY_DIR}")

include_directories(${milvus-sdk_SOURCE_DIR}/src/include)
link_directories(${milvus-sdk_BINARY_DIR}/src)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src src_files)
add_executable(my_program ${src_files})


target_link_libraries(my_program milvus_sdk)

