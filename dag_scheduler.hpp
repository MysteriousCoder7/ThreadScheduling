// dag_scheduler.hpp
#ifndef DAG_SCHEDULER_HPP
#define DAG_SCHEDULER_HPP

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <unordered_map>
#include <chrono>

struct SubTask {
    int id;
    std::function<void()> work;
    std::vector<int> dependencies;
};

class DAGScheduler {
private:
    std::unordered_map<int, SubTask> subtasks;
    std::unordered_map<int, int> indegree;
    std::unordered_map<int, std::vector<int>> graph;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> completed{0};
    int total;

public:
    DAGScheduler(const std::vector<SubTask>& taskList) {
        total = taskList.size();
        for (const auto& task : taskList) {
            subtasks[task.id] = task;
            indegree[task.id] = task.dependencies.size();
            for (int dep : task.dependencies) {
                graph[dep].push_back(task.id);
            }
        }
    }

    void execute() {
        std::queue<int> readyQueue;

        for (const auto& [id, degree] : indegree) {
            if (degree == 0) readyQueue.push(id);
        }

        while (completed < total) {
            std::vector<std::thread> threads;
            std::vector<int> currentBatch;

            {
                std::unique_lock<std::mutex> lock(mtx);
                while (!readyQueue.empty()) {
                    currentBatch.push_back(readyQueue.front());
                    readyQueue.pop();
                }
            }

            for (int id : currentBatch) {
                threads.emplace_back([this, id]() {
                    std::cout << "Subtask " << id << " started\n";
                    subtasks[id].work();
                    std::cout << "Subtask " << id << " finished\n";
                    this->onTaskComplete(id);
                });
            }

            for (auto& t : threads) {
                if (t.joinable()) t.join();
            }
        }
    }

    void onTaskComplete(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        for (int child : graph[id]) {
            indegree[child]--;
            if (indegree[child] == 0) {
                cv.notify_all();
            }
        }
        completed++;
    }
};

inline SubTask createSubTask(int id, std::vector<int> dependencies) {
    return SubTask{id, [id]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + id * 50));
    }, dependencies};
}

#endif // DAG_SCHEDULER_HPP
