#!/bin/bash
mkdir -p build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make

echo "Generating call graph:"
./CallGraphExtractor -p .. ../example.cpp


echo -e "\nDetecting thread usage:"
./ThreadDetector ../example.cpp
