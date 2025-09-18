## milvus-sdk-cpp-example
An example to use milvus cpp sdk in your project.
This project downloads milvus-sdk-cpp project from github and compile it. There will be an executable binary named "my_program" under the "cmake_build", which is compiled by the src/main.cpp. If you have an active Milvus server, you can run this program to connect the milvus.


## Compile and run
```sh
make
make run
```
1. `make` command to compile the project, it downloads milvus-sdk-cpp&gRPC for the first time.
2. `make run` command to run the example program, make sure you have an active Milvus server before you running this command.

### Compile with external pre-installed gRPC
By default, the milvus-sdk-cpp project downloads gRPC from github for the first time to make. You can skip the downloading if you have a pre-installed gRPC. Additional, you need to add the gRPC path into the LD_LIBRARY_PATH(or DYLD_LIBRARY_PATH) before you run the make command.
```sh
export LD_LIBRARY_PATH=/path/to/grpc/lib:$LD_LIBRARY_PATH
make GRPC_PATH=/path/to/grpc
make run
```

### Compile with BUILD_TYPE
By default, the project is compiled with "Debug". You can specify the BUILD_TYPE in make command.
```sh
make BUILD_TYPE=Release
make run
```

## Notes
- It might cost 30 minutes for the first time to make, because the milvus-sdk-cpp depends on gRPC,
gRPC repo is quite large and has many dependencies. 

- The output of milvus-sdk-cpp is a shared library named "libmilvus_sdk.so" which dynamicly links to gRPC libs. You can find it under the `cmake_build/_deps/milvus-sdk-build/src`

- The gRPC libs are built under the `cmake_build/_deps/grpc-build`, the required version of gRPC is defined here: https://github.com/milvus-io/milvus-sdk-cpp/blob/627dcde2c74062bbd98acf88ab325b28b8ffa7cf/cmake/ThirdPartyPackages.cmake#L22. If you want to use an external gRPC, make sure the version is compatible with the required version.

- You can use LD_LIBRARY_PATH to specify a gRPC lib path for "libmilvus_sdk.so".