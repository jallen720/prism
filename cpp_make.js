const PRISM_SRC_DIR = "src";
const CTK_DIR = "<PROJECTS>/ctk";
const COMPILER_OPTIONS = [ "std=c++14", "ggdb", "Wall", "Wextra", "pedantic-errors", "c" ];
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
            "debug": false,
            "source_dirs": [],
            "include_dirs":
            [
                PRISM_SRC_DIR,
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
            ],
            "internal_static_library_paths":
            [
                "lib/libprism.a",
            ],
            "library_import_paths":
            [
                `${ VULKAN_DIR }/lib/libvulkan.so.1`,
            ],
            "pkg_config": [],
            "compiler_options": COMPILER_OPTIONS,
            "linker_options": [ "Wl,-rpath,'$$ORIGIN/lib'" ],
        }
    },
    "targets":
    {
        "prism":
        {
            "type": "static_library",
            "debug": true,
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
            "compiler_options": COMPILER_OPTIONS,
            "linker_options": [],
        },
        "test":
        {
            "partial": "prism_test",
            "main": `${ PRISM_SRC_DIR }/test`,
            "include_dirs":
            [
                `${ CTK_DIR }/src`,
            ],
            "libraries":
            [
                // ctk dependencies
                "libyaml.a",
            ],
            "internal_static_library_paths":
            [
                `${ CTK_DIR }/lib/libctk.a`,
            ],
        },
        "sandbox":
        {
            "partial": "prism_test",
            "main": `${ PRISM_SRC_DIR }/sandbox`,
            "include_dirs":
            [
                VULKAN_INCLUDE_DIR,
            ],
        },
    }
};
