# Use milvus-sdk-cpp as a Conan package.
# The SDK and all its dependencies (grpc, protobuf, abseil, etc.)
# are managed by Conan — no FetchContent needed.
from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class MilvusSdkExample(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.requires("milvus-sdk-cpp/2.6.2@milvus/dev")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
