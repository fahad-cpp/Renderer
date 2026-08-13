EXENAME=Renderer
PRESET=clang-release

mkdir -p build

rm bin/$CONFIG/Renderer
cmake --preset $PRESET
cmake --build --preset $PRESET

[ -f "build/$PRESET/compile_commands.json" ] && cp -f build/$PRESET/compile_commands.json build/
./bin/Renderer
