# Milvus SDK C++ Examples

Examples demonstrating how to use the [Milvus C++ SDK](https://github.com/milvus-io/milvus-sdk-cpp) in your project.

## Project Structure

This repository contains three examples with different dependency management approaches:

```
milvus-sdk-cpp-example/
├── without-conan/          # No Conan, all dependencies built from source
│   ├── src/main.cpp
│   ├── CMakeLists.txt
│   ├── build.sh
│   └── Makefile
│
├── conan-for-dependencies/ # Conan manages SDK dependencies, SDK built from source
│   ├── src/main.cpp
│   ├── CMakeLists.txt
│   ├── conanfile.py
│   ├── build.sh
│   └── Makefile
│
└── conan-managed/          # Conan manages the SDK itself as a package
    ├── src/main.cpp
    ├── CMakeLists.txt
    ├── conanfile.py
    ├── build.sh
    └── Makefile
```

## Quick Start

`make` is equivalent to `make build` in all three examples.

### Without Conan (Self-contained)
```bash
cd without-conan
make                  # default: SHARED=ON (dynamic link)
make run
```

### Conan for Dependencies (Pre-built dependencies)
```bash
cd conan-for-dependencies
make                  # default: SHARED=OFF (static link)
make run
```

### Conan Managed (SDK as a Conan package)
Requires `milvus-sdk-cpp/2.6.2` published to a Conan remote.
```bash
cd conan-managed
make                  # default: SHARED=OFF (static link)
make run
```

## Static vs Dynamic Linking

All three examples accept a `SHARED` variable to control linkage of
`libmilvus_sdk` (and its gRPC / protobuf / abseil dependencies):

```bash
make SHARED=ON        # dynamic link — libmilvus_sdk.so + dep .so files at runtime
make SHARED=OFF       # static link  — everything baked into my_program (larger binary)
```

| Approach | Default `SHARED` | `SHARED=ON` builds | `SHARED=OFF` builds |
|---|---|---|---|
| without-conan | `ON` | `.so` from FetchContent | `.a` from FetchContent |
| conan-for-dependencies | `OFF` | Conan deps built as `shared=True` | Conan deps built as `shared=False` |
| conan-managed | `OFF` | Requires `milvus-sdk-cpp:shared=True` variant in the Conan remote | Uses the default (static) variant |

Note: `CMakeLists.txt` itself does **not** change between the two modes.
`target_link_libraries(my_program milvus_sdk)` works for both — CMake picks
`.so` or `.a` based on `BUILD_SHARED_LIBS`.

## Comparison

| Feature | without-conan | conan-for-dependencies | conan-managed |
|---------|--------------|----------------------|---------------|
| **Package Manager** | None | Conan 2.x | Conan 2.x |
| **SDK** | Built from source | Built from source | Conan package |
| **Dependencies** | Built from source | Conan pre-built | Conan pre-built |
| **Build Time** | Slowest | Medium | Fastest |
| **Dep Version Sync** | Automatic | Manual | Automatic |
| **Prerequisites** | CMake + Git | CMake + Git + Conan | CMake + Conan + Remote artifactory |

## Prerequisites

### Common
- CMake 3.14+
- C++14 compatible compiler

### Additional for without-conan
- Git

### Additional for conan-for-dependencies and conan-managed
- Conan 2.x: `pip install conan`

### Additional for conan-managed
- `milvus-sdk-cpp` package published to a Conan remote

## Which One to Use?

- **Choose `without-conan`** if you want simplicity and don't have Conan installed, all dependencies are downloaded by CMake FetchContent, including the milvus-sdk-cpp
- **Choose `conan-for-dependencies`** if you want CMake FetchContent to download milvus-sdk-cpp and use Conan to manage its dependencies
- **Choose `conan-managed`** if you want Conan to manage milvus-sdk-cpp as apackage(recommended for production, and milvus-sdk-cpp has been published to a Conan remote artifactory)

All examples build the same application and provide the same functionality.
