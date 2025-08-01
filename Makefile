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
GRPC_LIB_PATH 		:= $(PWD)/$(BUILD_OUTPUT_DIR)/_deps/grpc-build

build:
	@echo "Building ..."
	@@(env bash $(PWD)/build.sh)

clean:
	@echo "Cleaning ..."
	@rm -fr $(BUILD_OUTPUT_DIR)

run:
	@echo "Add $(GRPC_LIB_PATH) into LD_LIBRARY_PATH"
	@echo "Running ..."
	@$(BUILD_OUTPUT_DIR)/my_program

.PHONY: build clean
