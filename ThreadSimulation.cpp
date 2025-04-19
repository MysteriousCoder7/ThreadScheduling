#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <random>
#include <vector>
#include <map>
#include <iomanip>
#include <ctime>

struct SimThread {
    int id;
    std::string name;
    int burstTime;
    int remainingBurstTime;  // This tracks remaining burst time

    SimThread(int id, std::string name, int burstTime)
        : id(id), name(name), burstTime(burstTime), remainingBurstTime(burstTime) {}
};

// Utility to get current time string
std::string currentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&t);

    char buffer[10];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", now_tm);
    return std::string(buffer);
}

class ThreadScheduler {
private:
    std::queue<SimThread> taskQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool done = false;

    std::map<int, std::vector<std::string>> processorAssignments;
    std::mutex printMutex;
    int timeQuantum;  // Time slice for Round Robin scheduling

public:
    ThreadScheduler(int tq) : timeQuantum(tq) {}

    void addThread(const SimThread& t) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(t);
        std::cout << "[+] Added thread " << t.name << " with burst time: " << t.burstTime << "ms\n";
        cv.notify_one();
    }

    void runWithProcessors(int numProcessors) {
        std::vector<std::thread> processors;

        for (int i = 0; i < numProcessors; ++i) {
            processors.emplace_back([this, i]() {
                while (true) {
                    SimThread t(0, "", 0);
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        cv.wait(lock, [this]() { return !taskQueue.empty() || done; });

                        if (taskQueue.empty() && done)
                            return;

                        t = taskQueue.front();
                        taskQueue.pop();
                        processorAssignments[i].push_back(t.name);
                    }

                    {
                        std::lock_guard<std::mutex> lock(printMutex);
                        std::cout << "[" << currentTime() << "] [P" << i << "] Assigned: " << t.name
                                  << " (burst: " << t.burstTime << "ms)\n";
                    }

                    // Run for the time quantum or remaining time, whichever is smaller
                    int timeToRun = std::min(timeQuantum, t.remainingBurstTime);
                    std::this_thread::sleep_for(std::chrono::milliseconds(timeToRun));
                    t.remainingBurstTime -= timeToRun;

                    // If there is remaining burst time, push it back into the queue
                    if (t.remainingBurstTime > 0) {
                        {
                            std::lock_guard<std::mutex> lock(queueMutex);
                            taskQueue.push(t);
                        }
                        {
                            std::lock_guard<std::mutex> lock(printMutex);
                            std::cout << "[" << currentTime() << "] [P" << i << "] Preempted: " << t.name
                                      << " (remaining burst: " << t.remainingBurstTime << "ms)\n";
                        }
                    } else {
                        {
                            std::lock_guard<std::mutex> lock(printMutex);
                            std::cout << "[" << currentTime() << "] [P" << i << "] Completed: " << t.name << "\n";
                        }
                    }
                }
            });
        }

        for (auto& p : processors)
            p.join();

        printAssignmentSummary(numProcessors);
    }

    void printAssignmentSummary(int numProcessors) {
        std::cout << "\n=== Processor Assignment Summary ===\n";
        for (int i = 0; i < numProcessors; ++i) {
            std::cout << "Processor P" << i << " handled: ";
            if (processorAssignments[i].empty()) {
                std::cout << "None";
            } else {
                for (const auto& tName : processorAssignments[i]) {
                    std::cout << tName << " ";
                }
            }
            std::cout << "\n";
        }
    }

    void markDone() {
        std::lock_guard<std::mutex> lock(queueMutex);
        done = true;
        cv.notify_all();
    }
};

int main() {
    ThreadScheduler scheduler(500);  // 500ms time quantum for Round Robin

    const int numThreads = 10;
    const int numProcessors = 4;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(500, 2500); // burst times between 500–2500 ms

    for (int i = 1; i <= numThreads; ++i) {
        int burst = dist(gen);
        scheduler.addThread(SimThread(i, "T" + std::to_string(i), burst));
    }

    std::thread schedulerThread([&scheduler, numProcessors]() {
        scheduler.runWithProcessors(numProcessors);
    });

    scheduler.markDone();
    schedulerThread.join();

    return 0;
}
