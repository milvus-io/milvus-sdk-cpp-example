# Milvus SDK C++ Example - Without Conan

This example demonstrates how to build a C++ application using the [Milvus C++ SDK](https://github.com/milvus-io/milvus-sdk-cpp) without a package manager. All dependencies are built from source via CMake FetchContent.

## Prerequisites

- CMake 3.14 or higher
- C++14 compatible compiler
- Git

## Configuration

This example uses:
- **BUILD_FROM_CONAN=OFF**: The milvus-sdk-cpp library builds all dependencies from source
- **BUILD_SHARED_LIBS**: Controlled by the `SHARED` make variable (default `ON`)

## Build Instructions

`make` is equivalent to `make build`.

```bash
# Default: dynamic link (SHARED=ON)
make

# Static link
make SHARED=OFF

# Clean build artifacts
make clean
```

Or manually:

```bash
mkdir -p cmake_build && cd cmake_build
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DGRPC_PATH=${GRPC_PATH} -DBUILD_SHARED_LIBS=ON -DBUILD_FROM_CONAN=OFF ../
make -j8
```

### Build with a pre-installed gRPC

By default, milvus-sdk-cpp downloads and builds gRPC from source. To use a pre-installed gRPC, set the `GRPC_PATH` variable and add its lib path to `LD_LIBRARY_PATH` (or `DYLD_LIBRARY_PATH` on macOS):

```bash
export GRPC_PATH=/path/to/grpc
export LD_LIBRARY_PATH=$GRPC_PATH/lib:$LD_LIBRARY_PATH
make build
```

### Build with a specific BUILD_TYPE

By default, the build type is Debug. You can specify it:

```bash
make BUILD_TYPE=Release
make BUILD_TYPE=Release SHARED=OFF
```

## Run

After a successful build:

```bash
make run
```

Or directly:

```bash
./cmake_build/my_program
```

Make sure you have an active Milvus server before running.

## How It Works

1. **CMake FetchContent** downloads milvus-sdk-cpp from GitHub
2. **milvus-sdk-cpp** is configured with `BUILD_FROM_CONAN=OFF`, so it builds all dependencies (gRPC, Protobuf, etc.) from source
3. The example application links against the `milvus_sdk` library — `.so` when `SHARED=ON`, `.a` when `SHARED=OFF`. `target_link_libraries(my_program milvus_sdk)` in `CMakeLists.txt` works for both.

## Notes

- The first build downloads and compiles gRPC and all its dependencies from source, which is significantly slower than subsequent builds.
- With `SHARED=ON` (default), the SDK output is `libmilvus_sdk.so` under `cmake_build/_deps/milvus-sdk-build/src`, and it dynamically links to the gRPC `.so` files.
- With `SHARED=OFF`, the output is `libmilvus_sdk.a` and the final `my_program` is a single self-contained binary (larger size, no `.so` dependencies beyond libstdc++/libc).
- If you use a pre-installed gRPC, make sure its version is compatible with the version required by milvus-sdk-cpp (see the [ThirdPartyPackages.cmake](https://github.com/milvus-io/milvus-sdk-cpp/blob/v2.6.2/cmake/ThirdPartyPackages.cmake) in the SDK repo).

