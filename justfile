config := "Debug"

cmake_build_type := if config =~ "^(Debug|Release|RelWithDebInfo|MinSizeRel)$" {
	config
} else {
	error("Config must be Debug, Release, RelWithDebInfo or MinSizeRel")
}

default:
	just --list

format:
	find ./src -name '*xx' -print0 | xargs -0 clang-format -i --style file

commands:
	cmake \
		-S. \
		-Bbuild \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=true \
		"-DCMAKE_BUILD_TYPE={{cmake_build_type}}" \
		-DCMAKE_CXX_FLAGS_DEBUG="-O0 -ggdb -fsanitize=undefined -fsanitize=unreachable -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fno-sanitize-recover=all"

catch2:
	cmake \
		-S./test/Catch2 \
		-B./build/Catch2 \
		"-DCMAKE_BUILD_TYPE={{cmake_build_type}}" \
		"-DCMAKE_PREFIX_PATH=$PWD/build/deps" \
		"-DCMAKE_INSTALL_PREFIX=$PWD/build/deps" \
		-DCMAKE_CXX_FLAGS_DEBUG="-O0 -ggdb -fsanitize=undefined -fsanitize=unreachable -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fno-sanitize-recover=all"

	cmake --build ./build/Catch2
	cmake --install ./build/Catch2

test: catch2
	cmake \
		-S. \
		-B./build \
		-DARGS2_TESTS=ON \
		"-DCMAKE_PREFIX_PATH=$PWD/build/deps" \
		"-DCMAKE_BUILD_TYPE={{cmake_build_type}}" \
		-DCMAKE_CXX_FLAGS_DEBUG="-O0 -ggdb -fsanitize=undefined -fsanitize=unreachable -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fno-sanitize-recover=all"

	cmake --build ./build
	cd build && make test

clean:
	-rm -r build
