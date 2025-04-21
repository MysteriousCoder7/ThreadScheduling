#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <ctime>

struct Function {
    std::string name;
    std::vector<std::string> dependencies;
    bool isMajor = false;
    int burstTime = 0;
};

class CallGraphProcessor {
private:
    std::unordered_map<std::string, Function> functions;
    std::unordered_map<std::string, std::unordered_set<std::string>> dependencyGraph;
    std::unordered_map<std::string, int> callCount;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> inStack;
    std::vector<std::vector<std::string>> threadGroups;
    int maxThreads = 20;
    int currentThreadCount = 0;

    bool detectCycle(const std::string& funcName, std::unordered_set<std::string>& cycle) {
        if (inStack.count(funcName)) {
            cycle.insert(funcName);
            return true;
        }
        if (visited.count(funcName)) return false;

        visited.insert(funcName);
        inStack.insert(funcName);

        for (const auto& dep : dependencyGraph[funcName]) {
            if (detectCycle(dep, cycle)) {
                cycle.insert(funcName);
                return true;
            }
        }

        inStack.erase(funcName);
        return false;
    }

    void groupCycles() {
        for (const auto& entry : functions) {
            const auto& funcName = entry.first;
            std::unordered_set<std::string> cycle;
            if (detectCycle(funcName, cycle)) {
                threadGroups.push_back({cycle.begin(), cycle.end()});
            }
        }
    }

    void assignThreads() {
        int threadId = 1;
        std::unordered_set<std::string> alreadyGrouped;

        std::ofstream detailed("threads.txt");
        std::ofstream grouped("thread_grouped.txt");

        // Handle cyclic groups first
        for (auto& cycleGroup : threadGroups) {
            if (currentThreadCount >= maxThreads) break;

            int totalBurstTime = 0;
            std::stringstream detailedLine;
            std::stringstream groupedLine;

            std::string threadLabel = "T" + std::to_string(threadId);
            detailedLine << threadId << " " << threadLabel << " ";
            groupedLine << threadId << " " << threadLabel << " ";

            for (const auto& func : cycleGroup) {
                detailedLine << func << " ";
                totalBurstTime += functions[func].burstTime;
                alreadyGrouped.insert(func);
            }

            groupedLine << totalBurstTime;
            detailed << detailedLine.str() << "\n";
            grouped << groupedLine.str() << "\n";

            currentThreadCount++;
            threadId++;
        }

        // Handle remaining major functions
        for (const auto& entry : functions) {
            if (currentThreadCount >= maxThreads) break;
            const auto& funcName = entry.first;

            if (alreadyGrouped.count(funcName) == 0 && entry.second.isMajor) {
                int bt = entry.second.burstTime;
                std::string threadLabel = "T" + std::to_string(threadId);
                detailed << threadId << " " << threadLabel << " " << funcName << "\n";
                grouped << threadId << " " << threadLabel << " " << bt << "\n";
                currentThreadCount++;
                threadId++;
            }
        }

        detailed.close();
        grouped.close();
    }

    void identifyMajorFunctions() {
        for (auto& entry : functions) {
            const auto& funcName = entry.first;
            const auto& func = entry.second;

            if (func.dependencies.size() > 2 || callCount[funcName] > 1) {
                entry.second.isMajor = true;
            }
        }
    }

public:
    void parseCallGraph(const std::string& fileName) {
        std::ifstream file(fileName);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Error opening callgraph.txt\n";
            return;
        }

        std::srand(std::time(0));  // Seed RNG

        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string funcName, depName;

            if (ss >> funcName) {
                Function func{funcName, {}};
                while (ss >> depName) {
                    func.dependencies.push_back(depName);
                    dependencyGraph[funcName].insert(depName);
                    callCount[depName]++;
                }

                // Assign random burst time between 5 and 50
                func.burstTime = 300 + std::rand() % 101; // Range: 300 to 400

                functions[funcName] = func;
            }
        }
        file.close();
        identifyMajorFunctions();
    }

    void process() {
        groupCycles();
        assignThreads();
    }
};

int main() {
    CallGraphProcessor processor;
    processor.parseCallGraph("build/callgraph.txt");
    processor.process();
    std::cout << "[✔] threads.txt and thread_grouped.txt generated with indices and burst times.\n";
    return 0;
}
