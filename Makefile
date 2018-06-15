all: lib/libprism.a bin/test bin/simd bin/sandbox
	@:

import_prism_libs:
	@:

obj/src/prism/graphics.o: src/prism/graphics.cc /home/joel/Desktop/projects/ctk/src/ctk/data.h /home/joel/Desktop/projects/ctk/src/ctk/memory.h src/prism/graphics.h src/prism/utilities.h src/prism/defines.h src/prism/memory.h src/prism/vulkan.h src/prism/debug/graphics.inl
	@echo compiling $<
	@mkdir -p obj/src/prism
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -DPRISM_DEBUG -I/home/joel/Desktop/projects/ctk/src -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include $< -o $@

obj/src/prism/vulkan.o: src/prism/vulkan.cc /home/joel/Desktop/projects/ctk/src/ctk/data.h src/prism/vulkan.h src/prism/defines.h
	@echo compiling $<
	@mkdir -p obj/src/prism
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -DPRISM_DEBUG -I/home/joel/Desktop/projects/ctk/src -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include $< -o $@

obj/src/prism/utilities.o: src/prism/utilities.cc src/prism/utilities.h
	@echo compiling $<
	@mkdir -p obj/src/prism
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -DPRISM_DEBUG -I/home/joel/Desktop/projects/ctk/src -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include $< -o $@

obj/src/prism/system.o: src/prism/system.cc src/prism/system.h src/prism/utilities.h src/prism/defines.h
	@echo compiling $<
	@mkdir -p obj/src/prism
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -DPRISM_DEBUG -I/home/joel/Desktop/projects/ctk/src -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include $< -o $@

lib/libprism.a: obj/src/prism/graphics.o obj/src/prism/vulkan.o obj/src/prism/utilities.o obj/src/prism/system.o
	@echo linking $@
	@mkdir -p lib
	@ar rvs $@ $^

bin/lib/libvulkan.so.1: /home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib/libvulkan.so.1
	@echo 'copying $< -> $@'
	@mkdir -p bin/lib
	@cp /home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib/libvulkan.so.1 bin/lib/libvulkan.so.1

import_test_libs: bin/lib/libvulkan.so.1
	@:

obj/src/test.o: src/test.cc src/prism/system.h src/prism/graphics.h /home/joel/Desktop/projects/ctk/src/ctk/yaml.h
	@echo compiling $<
	@mkdir -p obj/src
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -DPRISM_DEBUG -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include -I/home/joel/Desktop/projects/ctk/src $< -o $@

bin/test: obj/src/test.o lib/libprism.a /home/joel/Desktop/projects/ctk/lib/libctk.a
	@echo linking $@
	@mkdir -p bin
	@g++ $^ -L/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib -Llib -L/home/joel/Desktop/projects/ctk/lib -lglfw3 -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp -lvulkan -l:libyaml.a -lprism -lctk -Wl,-rpath,'$$ORIGIN/lib' -o $@

import_simd_libs: bin/lib/libvulkan.so.1
	@:

obj/src/simd.o: src/simd.cc
	@echo compiling $<
	@mkdir -p obj/src
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -mavx2 -O3 -DPRISM_DEBUG -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include -I/home/joel/Desktop/projects/ctk/src $< -o $@

bin/simd: obj/src/simd.o lib/libprism.a /home/joel/Desktop/projects/ctk/lib/libctk.a
	@echo linking $@
	@mkdir -p bin
	@g++ $^ -L/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib -Llib -L/home/joel/Desktop/projects/ctk/lib -lglfw3 -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp -lvulkan -l:libyaml.a -lprism -lctk -Wl,-rpath,'$$ORIGIN/lib' -o $@

import_sandbox_libs: bin/lib/libvulkan.so.1
	@:

obj/src/sandbox.o: src/sandbox.cc src/prism/memory.h
	@echo compiling $<
	@mkdir -p obj/src
	@g++ -std=c++14 -ggdb -Wall -Wextra -pedantic-errors -c -DPRISM_DEBUG -Isrc -I/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/include -I/home/joel/Desktop/projects/ctk/src $< -o $@

bin/sandbox: obj/src/sandbox.o lib/libprism.a /home/joel/Desktop/projects/ctk/lib/libctk.a
	@echo linking $@
	@mkdir -p bin
	@g++ $^ -L/home/joel/Desktop/packages/VulkanSDK/1.1.73.0/x86_64/lib -Llib -L/home/joel/Desktop/projects/ctk/lib -lglfw3 -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp -lvulkan -l:libyaml.a -lprism -lctk -Wl,-rpath,'$$ORIGIN/lib' -o $@

