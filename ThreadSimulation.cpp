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
#include <fstream>
#include <filesystem>
#include <sstream>

struct SimThread {
    int id;
    std::string name;
    int burstTime;
    int remainingBurstTime;

    SimThread(int id, std::string name, int burstTime)
        : id(id), name(name), burstTime(burstTime), remainingBurstTime(burstTime) {}
};

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
    int timeQuantum;
    int activeTasks = 0;

    std::map<int, int> taskCount;
    std::map<int, long long> execTime;

    std::vector<int> turnaroundTimes;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    std::vector<std::string> executionLog;
    int totalThreads = 0;

public:
    ThreadScheduler(int tq) : timeQuantum(tq) {}

    void addThread(const SimThread& t) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(t);
        activeTasks++;
        totalThreads++;
        std::cout << "[+] Added thread " << t.name << " with burst time: " << t.burstTime << "ms\n";
        cv.notify_one();
    }

    void runWithProcessors(int numProcessors) {
        startTime = std::chrono::high_resolution_clock::now();

        auto worker = [&](int processorId) {
            while (true) {
                SimThread t(0, "", 0);
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    cv.wait(lock, [&]() { return !taskQueue.empty() || done; });
                    if (taskQueue.empty() && done) break;
                    if (!taskQueue.empty()) {
                        t = taskQueue.front();
                        taskQueue.pop();
                    } else {
                        continue;
                    }
                }

                auto startThread = std::chrono::high_resolution_clock::now();
                while (t.remainingBurstTime > 0) {
                    auto start = std::chrono::high_resolution_clock::now();

                    int exec = std::min(timeQuantum, t.remainingBurstTime);
                    std::this_thread::sleep_for(std::chrono::milliseconds(exec));
                    t.remainingBurstTime -= exec;

                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                    {
                        std::lock_guard<std::mutex> lock(printMutex);
                        processorAssignments[processorId].push_back(t.name);
                        taskCount[processorId]++;
                        execTime[processorId] += duration;

                        std::ostringstream log;
                        log << "[" << currentTime() << "] Processor " << processorId
                            << " executed " << t.name << " for " << exec << "ms, Remaining: " << t.remainingBurstTime << "ms";
                        executionLog.push_back(log.str());
                        std::cout << log.str() << "\n";
                    }
                }

                auto endThread = std::chrono::high_resolution_clock::now();
                int turnaround = std::chrono::duration_cast<std::chrono::milliseconds>(endThread - startTime).count();
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    turnaroundTimes.push_back(turnaround);
                    activeTasks--;
                    if (activeTasks == 0) done = true;
                    cv.notify_all();
                }
            }
        };

        std::vector<std::thread> processors;
        for (int i = 0; i < numProcessors; ++i) {
            processors.emplace_back(worker, i);
        }
        for (auto& p : processors) {
            p.join();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        double totalExecutionTime = std::chrono::duration<double>(endTime - startTime).count();

        double avgTAT = 0.0;
        for (int t : turnaroundTimes) avgTAT += t;
        avgTAT /= turnaroundTimes.size();

        double throughput = static_cast<double>(turnaroundTimes.size()) / totalExecutionTime;

        double totalBusyTime = 0;
        for (const auto& [_, t] : execTime) totalBusyTime += t / 1000.0;
        double cpuUtilization = (totalBusyTime / (numProcessors * totalExecutionTime)) * 100.0;

        // Write metrics in tabular CSV format
        std::ofstream out("metrics.csv", std::ios::app);
        static bool headerWritten = false;
        if (!headerWritten) {
            out << "Run Type,Execution Time (s),Average Turnaround Time (ms),Throughput (tasks/sec),CPU Utilization (%)\n";
            headerWritten = true;
        }
        out << (numProcessors == 1 ? "Single Processor" : "Multiprocessor") << ","
            << totalExecutionTime << ","
            << avgTAT << ","
            << throughput << ","
            << cpuUtilization << "\n";
        out.close();

        // Write execution log in tabular CSV format
        std::ofstream logFile("execution_log.csv", std::ios::app);
        static bool logHeaderWritten = false;
        if (!logHeaderWritten) {
            logFile << "Timestamp,Processor ID,Thread Name,Execution Time (ms),Remaining Time (ms)\n";
            logHeaderWritten = true;
        }
        for (const auto& logEntry : executionLog) {
            logFile << logEntry << "\n";
        }
        logFile.close();

        std::cout << "\n==== Simulation Metrics ====" << (numProcessors == 1 ? " (Single Processor)" : " (Multiprocessor)") << " ====\n";
        std::cout << "Total Execution Time: " << totalExecutionTime << " seconds\n";
        std::cout << "Average Turnaround Time: " << avgTAT << " ms\n";
        std::cout << "Throughput: " << throughput << " tasks/sec\n";
        std::cout << "CPU Utilization: " << cpuUtilization << "%\n";
    }
};

int main() {
    ThreadScheduler scheduler(100);  // time quantum = 100ms
    scheduler.addThread(SimThread(1, "T1", 300));
    scheduler.addThread(SimThread(2, "T2", 200));
    scheduler.addThread(SimThread(3, "T3", 150));
    scheduler.addThread(SimThread(4, "T4", 450));
    scheduler.addThread(SimThread(5, "T5", 250));
    scheduler.addThread(SimThread(6, "T6", 100));
    scheduler.addThread(SimThread(7, "T7", 350));
    scheduler.addThread(SimThread(8, "T8", 200));
    scheduler.addThread(SimThread(9, "T9", 400));
    scheduler.addThread(SimThread(10, "T10", 150));

    std::cout << "\n===== MULTIPROCESSOR RUN (3 processors) =====\n";
    scheduler.runWithProcessors(3);

    std::cout << "\n===== SINGLE PROCESSOR RUN (1 processor) =====\n";
    ThreadScheduler scheduler2(100);
    scheduler2.addThread(SimThread(1, "T1", 300));
    scheduler2.addThread(SimThread(2, "T2", 200));
    scheduler2.addThread(SimThread(3, "T3", 150));
    scheduler2.addThread(SimThread(4, "T4", 450));
    scheduler2.addThread(SimThread(5, "T5", 250));
    scheduler2.addThread(SimThread(6, "T6", 100));
    scheduler2.addThread(SimThread(7, "T7", 350));
    scheduler2.addThread(SimThread(8, "T8", 200));
    scheduler2.addThread(SimThread(9, "T9", 400));
    scheduler2.addThread(SimThread(10, "T10", 150));
    scheduler2.runWithProcessors(1);

    return 0;
}
