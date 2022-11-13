# The Compile process works in two steps:
# 1) build the txgasm tool. This is a tool to generate some internal memory addresses.
# After generating it, use it to generate the necessary header files. Only after this
# was successful, the actual Glide API can be build.

# build the fxgasm file
pushd glide3/src/
mkdir fxgasm_tool/bin
pushd fxgasm_tool/bin
cmake ..
cmake --build .
popd

# use fxgasm to generate the headers
./fxgasm_tool/bin/fxgasm -inline > fxinline.h || true
./fxgasm_tool/bin/fxgasm -hex > fxgasm.h || true

# go back to initial directory
popd


# build the build number file
pushd glide3/src/
mkdir fxbldno_tool/bin
pushd fxbldno_tool/bin
cmake ..
cmake --build .
popd

# use fxbldno to generate the headers
./fxbldno_tool/bin/fxbldno > fxbldno.h || true

# go back to initial directory
popd

# if that worked well, let's build the Glide3API for Voodoo4/5.
mkdir -p bin
cd bin
cmake ..
cmake --build .
