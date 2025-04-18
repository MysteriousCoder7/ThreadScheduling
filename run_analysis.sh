#!/bin/bash
mkdir -p build && cd build
cmake ..
make

echo "Generating call graph:"
./CallGraphExtractor -p .. ../MultiProcessorEnv.cpp


echo -e "\nDetecting thread usage:"
./ThreadDetector ../MultiProcessorEnv.cpp
