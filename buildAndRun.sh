#/bin/bash

mkdir -p build
cd build

cmake ..
make

cd ..

# run
./build/bin/MemMngr
