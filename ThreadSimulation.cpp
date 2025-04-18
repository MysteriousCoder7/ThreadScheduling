#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <map>
#include <random>

// Simulated thread structure
struct SimThread {
    int id;
    std::string name;
    int burstTime;  // simulated time it takes to complete (ms)
    bool completed = false;

    SimThread(int id, std::string name, int burstTime)
        : id(id), name(name), burstTime(burstTime) {}
};

// Simulated Scheduler
class ThreadScheduler {
private:
    std::queue<SimThread> readyQueue;
    int timeQuantum = 1000;  // 1 sec quantum for round robin

public:
    void addThread(const SimThread& t) {
        readyQueue.push(t);
        std::cout << "[+] Added thread " << t.name << " with burst time: " << t.burstTime << "ms\n";
    }

    void runFIFO() {
        std::cout << "\n--- FIFO Scheduling Start ---\n";
        while (!readyQueue.empty()) {
            SimThread t = readyQueue.front();
            readyQueue.pop();
            std::cout << "[*] Running: " << t.name << " (burst: " << t.burstTime << "ms)...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(t.burstTime));
            std::cout << "[✓] Completed: " << t.name << "\n";
        }
        std::cout << "--- FIFO Scheduling Done ---\n";
    }

    void runRoundRobin() {
        std::cout << "\n--- Round Robin Scheduling Start ---\n";
        std::queue<SimThread> tempQueue;

        while (!readyQueue.empty()) {
            SimThread t = readyQueue.front();
            readyQueue.pop();

            if (t.burstTime <= timeQuantum) {
                std::cout << "[*] Running: " << t.name << " for " << t.burstTime << "ms\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(t.burstTime));
                std::cout << "[✓] Completed: " << t.name << "\n";
            } else {
                std::cout << "[*] Running: " << t.name << " for " << timeQuantum << "ms (Remaining: " << t.burstTime - timeQuantum << "ms)\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(timeQuantum));
                t.burstTime -= timeQuantum;
                tempQueue.push(t);
            }

            if (readyQueue.empty() && !tempQueue.empty()) {
                std::swap(readyQueue, tempQueue);
            }
        }
        std::cout << "--- Round Robin Scheduling Done ---\n";
    }
};

int main() {
    ThreadScheduler scheduler;

    // Simulate 5 threads with random burst times
    for (int i = 1; i <= 5; ++i) {
        int burst = (rand() % 2000) + 500;  // 500ms to 2500ms
        scheduler.addThread(SimThread(i, "T" + std::to_string(i), burst));
    }

    // Choose a scheduling policy to simulate
    scheduler.runFIFO();
    // scheduler.runRoundRobin();

    return 0;
}
