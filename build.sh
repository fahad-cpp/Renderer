CONFIG="Debug"
rm bin/$CONFIG/Renderer
cmake -S . -G Ninja -B build -DCMAKE_BUILD_TYPE=$CONFIG -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 2>build/error.txt
cmake --build build --config $CONFIG --parallel >build/error.txt

./bin/$CONFIG/Renderer
