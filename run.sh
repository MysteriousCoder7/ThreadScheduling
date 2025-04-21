#!/bin/bash
mkdir -p build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make

echo "Generating call graph:"
./CallGraphExtractor -p .. ../MultiProcessorEnv.cpp


echo -e "\nDetecting thread usage:"
./ThreadDetector ../MultiProcessorEnv.cpp

cd ..

echo -e "\nConverting call graph to threads:"
g++ MTS.cpp -o MTS
./MTS

echo -e "\nSimulating based on generated threads:"
g++ ThreadSimulation.cpp -o thread_sim
./thread_sim

python3 plot_metrics.py