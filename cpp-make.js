const PRISM_SRC_DIR = "src";
const PRISM_DEFINES = [ "PRISM_DEBUG" ];
const PRISM_COMPILER_OPTIONS = [ "std=c++14", "ggdb", "Wall", "Wextra", "pedantic-errors", "c" ];
const CTK_DIR = "<PROJECTS>/ctk";
const VULKAN_DIR = "<PACKAGES>/VulkanSDK/1.1.73.0/x86_64";
const VULKAN_INCLUDE_DIR = `${ VULKAN_DIR }/include`;

module.exports =
{
    "source_extension": "cc",
    "header_extension": "h",
    "partials":
    {
        "prism_test":
        {
            "type": "application",
            "defines": PRISM_DEFINES,
            "source_dirs": [],
            "include_dirs":
            [
                PRISM_SRC_DIR,
                VULKAN_INCLUDE_DIR,
                `${ CTK_DIR }/src`,
            ],
            "library_dirs": [],
            "libraries":
            [
                // prism dependencies
                "glfw3",
                "rt",
                "m",
                "dl",
                "X11",
                "pthread",
                "xcb",
                "Xau",
                "Xdmcp",
                "vulkan",

                // ctk dependencies
                "libyaml.a",
            ],
            "internal_static_library_paths":
            [
                "lib/libprism.a",
                `${ CTK_DIR }/lib/libctk.a`,
            ],
            "library_import_paths":
            [
                `${ VULKAN_DIR }/lib/libvulkan.so.1`,
            ],
            "pkg_config": [],
            "compiler_options": PRISM_COMPILER_OPTIONS,
            "linker_options": [ "Wl,-rpath,'$$ORIGIN/lib'" ],
        }
    },
    "targets":
    {
        "prism":
        {
            "type": "static_library",
            "defines": PRISM_DEFINES,
            "source_dirs":
            [
                `${ PRISM_SRC_DIR }/prism`,
            ],
            "include_dirs":
            [
                `${ CTK_DIR }/src`,
                PRISM_SRC_DIR,
                VULKAN_INCLUDE_DIR,
            ],
            "library_dirs": [],
            "libraries": [],
            "internal_static_library_paths": [],
            "library_import_paths": [],
            "pkg_config": [],
            "compiler_options": PRISM_COMPILER_OPTIONS,
            "linker_options": [],
        },
        "test":
        {
            "partial": "prism_test",
            "main": `${ PRISM_SRC_DIR }/test`,
        },
        "simd":
        {
            "partial": "prism_test",
            "main": `${ PRISM_SRC_DIR }/simd`,
            "compiler_options": [ "mavx2", "O3" ],
        },
        "sandbox":
        {
            "partial": "prism_test",
            "main": `${ PRISM_SRC_DIR }/sandbox`,
        },
    }
};
