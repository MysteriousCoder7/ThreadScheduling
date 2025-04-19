#include <iostream>
#include <thread>
#include <vector>

void task1() {
    std::cout << "Task 1 is running\n";
}

void task2() {
    std::cout << "Task 2 is running\n";
}

void task3() {
    std::cout << "Task 3 is running\n";
}

void task4() {
    std::cout << "Task 4 is running\n";
}

void task5() {
    std::cout << "Task 5 is running\n";
}

void task6() {
    std::cout << "Task 6 is running\n";
}

void task7() {
    std::cout << "Task 7 is running\n";
}

void task8() {
    std::cout << "Task 8 is running\n";
}

void task9() {
    std::cout << "Task 9 is running\n";
}

void task10() {
    std::cout << "Task 10 is running\n";
}

int main() {
    std::thread t1(task1);
    std::thread t2(task2);
    std::thread t3(task3);
    std::thread t4(task4);
    std::thread t5(task5);
    std::thread t6(task6);
    std::thread t7(task7);
    std::thread t8(task8);
    std::thread t9(task9);
    std::thread t10(task10);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();

    return 0;
}
