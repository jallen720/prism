const PRISM_SRC_DIR = "src";
const CTK_DIR = "<PROJECTS>/ctk";

module.exports =
{
    "source_extension": "cc",
    "header_extension": "h",
    "targets":
    {
        "prism":
        {
            "type": "static_library",
            "source_dirs":
            [
                `${ PRISM_SRC_DIR }/prism`,
            ],
            "include_dirs":
            [
                PRISM_SRC_DIR,
                `${ CTK_DIR }/src`,
            ],
            "library_dirs": [],
            "libraries": [],
            "internal_static_library_paths": [],
            "library_import_paths": [],
            "pkg_config": [],
            "compiler_options": [ "std=c++14", "ggdb", "Wall", "Wextra", "pedantic-errors", "c" ],
            "linker_options": [ "Wl,-rpath,'$$ORIGIN/lib'" ],
        },
        "test":
        {
            "type": "application",
            "main": `${ PRISM_SRC_DIR }/main`,
            "source_dirs": [],
            "include_dirs":
            [
                PRISM_SRC_DIR,
                `${ CTK_DIR }/src`,
            ],
            "library_dirs": [],
            "libraries":
            [
                "libyaml.a",
            ],
            "internal_static_library_paths":
            [
                "lib/libprism.a",
                `${ CTK_DIR }/lib/libctk.a`,
            ],
            "library_import_paths": [],
            "pkg_config": [],
            "compiler_options": [ "std=c++14", "ggdb", "Wall", "Wextra", "pedantic-errors", "c" ],
            "linker_options": [ "Wl,-rpath,'$$ORIGIN/lib'" ],
        },
    }
};
