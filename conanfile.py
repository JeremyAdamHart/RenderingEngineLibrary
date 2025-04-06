# from conan import ConanFile
# from conan.tools.cmake import Cmake
# from conan.tools.files import copy

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class Recipe(ConanFile):
    name = "jorl"
    version = "0.0.1"

    exports_sources = "include/*", "core/*", "exts/*", "CMakePresets.json", "CMakeLists.txt", "cmake/*"
    generators = "CMakeToolchain", "CMakeDeps"

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

        self.options['glad'].spec = 'gl'
        self.options['glad'].gl_profile = 'core'
        self.options['glad'].gl_version = '4.6'



    def layout(self):
        cmake_layout(self)

    # def generate(self):
    #     tc = CMakeToolchain(self)
    #     tc.generate()
    #     deps = CMakeDeps

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("freetype/2.13.3")
        self.requires("glad/0.1.36")
        self.requires("glm/1.0.1")
        self.requires("stb/cci.20240531")
        self.requires("tinyply/2.3.4")
        self.requires("openvr/1.16.8")

    def package_info(self):
        self.cpp_info.components["core"].libs = ["jorl_core"]
        # self.cpp_info.components["core"].set_property("cmake_target_name", "jorl::core")
        self.cpp_info.components["exts"].libs = ["jorl_exts"]
        self.cpp_info.components["exts"].requires = ["core"]
        # self.cpp_info.components["exts"].set_property("cmake_target_name", "jorl::exts")

        # self.cpp_info.components["rendering"].libs = ["rendering"]
        # self.cpp_info.components["rendering"].requires = ["algorithms"]
        # self.cpp_info.components["rendering"].set_property("cmake_target_name", "rendering")