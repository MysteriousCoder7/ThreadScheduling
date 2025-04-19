1.sudo apt install clang llvm llvm-dev libclang-dev cmake g++
2.chmod +x run_analysis.sh
3../run_analysis.sh
4.g++ -std=c++17 -o thread_sim ThreadSimulation.cpp -pthread
5../thread_sim
