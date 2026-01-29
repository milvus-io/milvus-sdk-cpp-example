# Milvus SDK C++ Examples

Examples demonstrating how to use the Milvus C++ SDK in your project.

## Project Structure

This repository contains two examples with different dependency management approaches:

```
milvus-sdk-cpp-example/
├── without-conan/          # Example WITHOUT Conan (BUILD_FROM_CONAN=OFF)
│   ├── src/main.cpp
│   ├── CMakeLists.txt
│   ├── build.sh
│   └── Makefile
│
└── with-conan/             # Example WITH Conan (BUILD_FROM_CONAN=ON)
    ├── src/main.cpp
    ├── CMakeLists.txt
    ├── conanfile.txt
    ├── build.sh
    └── Makefile
```

## Quick Start

### Without Conan (Self-contained)
```bash
cd without-conan
make build
./cmake_build/my_program
```

### With Conan (Pre-built dependencies)
```bash
cd with-conan
make build
./cmake_build/my_program
```

## Comparison

| Feature | without-conan | with-conan |
|---------|--------------|------------|
| **BUILD_FROM_CONAN** | OFF | ON |
| **Package Manager** | None | Conan 2.x |
| **Dependencies** | Built from source (FetchContent) | Pre-built packages |
| **Build Time** | always slow | fast after the first time |
| **Disk Usage** | Per-project | Shared cache |
| **Prerequisites** | CMake + Git | CMake + Git + Conan |

## Prerequisites

### Common
- CMake 3.14+
- C++14 compatible compiler
- Git

### Additional for with-conan
- Conan 2.x: `pip install conan`

## Which One to Use?

- **Choose `without-conan`** if you want simplicity and don't have Conan installed
- **Choose `with-conan`** if you want faster builds and use Conan in your workflow

Both examples build the same application and provide the same functionality.
