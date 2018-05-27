all: lib/libprism.a bin/test ./cpp_make.js
	@:

import_prism_libs: 
	@:

obj/src/prism/graphics.o: src/prism/graphics.cc
	@echo compiling $<
	@mkdir -p obj/src/prism
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include $< -o $@

obj/src/prism/system.o: src/prism/system.cc src/prism/system.h
	@echo compiling $<
	@mkdir -p obj/src/prism
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include $< -o $@

lib/libprism.a: obj/src/prism/graphics.o obj/src/prism/system.o
	@echo linking $@
	@mkdir -p lib
	@ar rvs $@ $^

bin/lib/libvulkan.so.1: /home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib/libvulkan.so.1
	@echo 'copying $< -> $@'
	@mkdir -p bin/lib
	@cp /home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib/libvulkan.so.1 bin/lib/libvulkan.so.1

import_test_libs: bin/lib/libvulkan.so.1
	@:

obj/src/test.o: src/test.cc src/prism/system.h /home/joel/Desktop/projects/ctk/src/ctk/yaml.h
	@echo compiling $<
	@mkdir -p obj/src
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -Isrc -I/home/joel/Desktop/projects/ctk/src $< -o $@

bin/test: obj/src/test.o lib/libprism.a /home/joel/Desktop/projects/ctk/lib/libctk.a
	@echo linking $@
	@mkdir -p bin
	@g++ $^ -L/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib -Llib -L/home/joel/Desktop/projects/ctk/lib -lglfw3 -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp -lvulkan -l:libyaml.a -lprism -lctk -Wl,-rpath,'$$ORIGIN/lib' -o $@

