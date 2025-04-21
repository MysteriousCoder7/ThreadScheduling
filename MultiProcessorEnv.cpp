// multiprocessor_sim_with_dag.cpp
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <random>
#include <unordered_map>
#include <chrono>

#include "dag_scheduler.hpp"

// Phase 1 - Task + Queue + Processors
struct Task {
    int id;
    std::vector<SubTask> subtasks;

    void work() const {
        std::cout << "[Task " << id << "] Starting DAG process\n";
        DAGScheduler scheduler(subtasks);
        scheduler.execute();
        std::cout << "[Task " << id << "] DAG process complete\n";
    }
};

class TaskQueue {
private:
    std::queue<Task> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool shutdownFlag = false;

public:
    void push(const Task& task) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(task);
        cv.notify_one();
    }

    bool pop(Task& task) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return !queue.empty() || shutdownFlag; });
        if (shutdownFlag && queue.empty()) return false;
        task = queue.front();
        queue.pop();
        return true;
    }

    void shutdown() {
        std::unique_lock<std::mutex> lock(mtx);
        shutdownFlag = true;
        cv.notify_all();
    }
};

class Processor {
private:
    int id;
    TaskQueue& taskQueue;
    std::atomic<int>& completedTasks;

public:
    Processor(int id, std::atomic<int>& completedTasks, TaskQueue& queue)
        : id(id), completedTasks(completedTasks), taskQueue(queue) {}

    void operator()() {
        Task task;
        while (taskQueue.pop(task)) {
            std::cout << "Processor " << id << " executing Task " << task.id << "\n";
            task.work();
            completedTasks++;
        }
        std::cout << "Processor " << id << " shutting down.\n";
    }
};

int main() {
    const int NUM_PROCESSORS = 3;
    const int NUM_TASKS = 3;

    TaskQueue taskQueue;
    std::atomic<int> completedTasks{0};

    std::vector<std::thread> processors;
    for (int i = 0; i < NUM_PROCESSORS; ++i) {
        processors.emplace_back(Processor(i + 1, completedTasks, taskQueue));
    }

    for (int i = 0; i < NUM_TASKS; ++i) {
        std::vector<SubTask> subtasks = {
            createSubTask(1, {}),
            createSubTask(2, {1}),
            createSubTask(3, {1}),
            createSubTask(4, {2, 3}),
            createSubTask(5, {4})
        };
        taskQueue.push(Task{i + 1, subtasks});
    }

    while (completedTasks < NUM_TASKS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    taskQueue.shutdown();

    for (auto& p : processors) {
        if (p.joinable()) p.join();
    }

    std::cout << "All tasks processed.\n";
    return 0;
}