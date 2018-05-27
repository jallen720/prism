all: lib/libprism.a bin/test
	@:

import_prism_libs: 
	@:

obj/src/prism/system.o: src/prism/system.cc src/prism/system.h
	@echo compiling $<
	@mkdir -p obj/src/prism
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc -I/home/joel/Desktop/projects/ctk/src $< -o $@

lib/libprism.a: obj/src/prism/system.o
	@echo linking $@
	@mkdir -p lib
	@ar rvs $@ $^

import_test_libs: 
	@:

obj/src/main.o: src/main.cc src/prism/system.h
	@echo compiling $<
	@mkdir -p obj/src
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc -I/home/joel/Desktop/projects/ctk/src $< -o $@

bin/test: obj/src/main.o lib/libprism.a /home/joel/Desktop/projects/ctk/lib/libctk.a
	@echo linking $@
	@mkdir -p bin
	@g++ $^ -Llib -L/home/joel/Desktop/projects/ctk/lib -l:libyaml.a -lprism -lctk -Wl,-rpath,'$$ORIGIN/lib' -o $@

