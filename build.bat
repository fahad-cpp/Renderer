if exist build\Renderer.exe del build\Renderer.exe
pushd build
cmake ..
cmake --build .
popd
start build\Renderer.exe