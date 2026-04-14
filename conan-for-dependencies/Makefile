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
PWD 				:= $(shell pwd)
BUILD_OUTPUT_DIR	:= cmake_build
SHARED				?= OFF

.DEFAULT_GOAL := build

build:
	@echo "Building (SHARED=$(SHARED)) ..."
	@SHARED=$(SHARED) bash build.sh

clean:
	@echo "Cleaning ..."
	@rm -fr $(BUILD_OUTPUT_DIR)
	@rm -f CMakeUserPresets.json
	@echo "Done"

run:
	@echo "Running ..."
	# 1. GRPC_VERBOSITY=ERROR and GLOG_minloglevel=3 silence the informational
	# startup logs from gRPC and Abseil (the "I0000 ..." lines printed to stderr
	# before absl::InitializeLog() is called). Not errors, just noise.
	# 2. conanrun.sh sets LD_LIBRARY_PATH to the Conan cache folders so that
	# shared-library builds (SHARED=ON) can find their .so files at runtime.
	# Harmless for static builds (SHARED=OFF).
	@bash -c "source $(BUILD_OUTPUT_DIR)/conanrun.sh && GRPC_VERBOSITY=ERROR GLOG_minloglevel=3 $(BUILD_OUTPUT_DIR)/my_program"

.PHONY: build clean run
