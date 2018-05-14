const COMPILER_OPTIONS = [ "std=c++14", "ggdb", "Wall", "Wextra", "pedantic-errors", "c" ];

// game
const GAME_SRC_DIR = "src";

module.exports =
{
    "source_extension": "cc",
    "header_extension": "h",
    "targets":
    {
        "game":
        {
            "type": "application",
            "main": `${ GAME_SRC_DIR }/main`,
            "source_dirs":
            [
                `${ GAME_SRC_DIR }/game`,
                `${ GAME_SRC_DIR }/engine`,
                `${ GAME_SRC_DIR }/utils`,
            ],
            "include_dirs":
            [
                GAME_SRC_DIR,
            ],
            "library_dirs": [],
            "libraries":
            [
                "libyaml.a"
            ],
            "library_import_paths":
            [
            ],
            "pkg_config": [],
            "compiler_options": COMPILER_OPTIONS,
            "linker_options": [ "Wl,-rpath,'$$ORIGIN/lib'" ],
        },
    }
};
