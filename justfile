default:
	just --list

format:
	find ./src -name '*xx' -print0 | xargs -0 clang-format -i --style file

commands:
	cmake -S. -Bbuild -DCMAKE_EXPORT_COMPILE_COMMANDS=true

test:
	cmake \
		-S. \
		-Bbuild \
		-DCMAKE_CXX_FLAGS_DEBUG="-O0 -ggdb -fsanitize=undefined -fsanitize=unreachable -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fno-sanitize-recover=all"

	cd build && make test

clean:
	-rm -r build
