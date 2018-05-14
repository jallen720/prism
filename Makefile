all: bin/game
	@:

import_game_libs: 
	@:

obj/src/game/test.o: src/game/test.cc src/game/test.h
	@echo compiling $<
	@mkdir -p obj/src/game
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc $< -o $@

obj/src/engine/window.o: src/engine/window.cc src/engine/window.h
	@echo compiling $<
	@mkdir -p obj/src/engine
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc $< -o $@

obj/src/utils/file.o: src/utils/file.cc src/utils/file.h
	@echo compiling $<
	@mkdir -p obj/src/utils
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc $< -o $@

obj/src/utils/yaml.o: src/utils/yaml.cc src/utils/yaml.h
	@echo compiling $<
	@mkdir -p obj/src/utils
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc $< -o $@

obj/src/main.o: src/main.cc 
	@echo compiling $<
	@mkdir -p obj/src
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc $< -o $@

bin/game: obj/src/game/test.o obj/src/engine/window.o obj/src/utils/file.o obj/src/utils/yaml.o obj/src/main.o
	@echo linking $@
	@mkdir -p bin
	@g++ $^ -l:libyaml.a -Wl,-rpath,'$$ORIGIN/lib' -o $@

