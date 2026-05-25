#!/bin/bash

cmake -B build-win \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-win
