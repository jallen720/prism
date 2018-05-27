const PRISM_SRC_DIR = "src";
const CTK_DIR = "<PROJECTS>/ctk";
const COMPILER_OPTIONS = [ "std=c++14", "ggdb", "Wall", "Wextra", "pedantic-errors", "c" ];
const VULKAN_DIR = "<PACKAGES>/VulkanSDK/1.1.73.0/x86_64";

module.exports =
{
    "source_extension": "cc",
    "header_extension": "h",
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
                PRISM_SRC_DIR,
                `${ VULKAN_DIR }/include`,
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
            "type": "application",
            "debug": false,
            "main": `${ PRISM_SRC_DIR }/test`,
            "source_dirs": [],
            "include_dirs":
            [
                PRISM_SRC_DIR,
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
            "compiler_options": COMPILER_OPTIONS,
            "linker_options": [ "Wl,-rpath,'$$ORIGIN/lib'" ],
        },
    }
};
