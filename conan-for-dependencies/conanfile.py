# Dependency versions matching milvus-sdk-cpp
# The SDK uses gRPC 1.65.0 on non-macOS platforms.
from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class MilvusSdkExample(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.requires("grpc/1.65.0")
        self.requires("protobuf/5.27.0")
        self.requires("abseil/20240116.2")
        self.requires("nlohmann_json/3.11.3")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
