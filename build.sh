EXENAME=Renderer
PRESET=clang-release

mkdir -p build

rm bin/$CONFIG/Renderer
cmake --preset $PRESET

if [[ $1 == "clean" ]]; then
    echo "clean-build:"
    cmake --build --preset $PRESET --clean-first
else
    cmake --build --preset $PRESET
fi

[ -f "build/$PRESET/compile_commands.json" ] && cp -f build/$PRESET/compile_commands.json build/
./bin/Renderer
