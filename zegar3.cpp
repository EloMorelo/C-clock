#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <random>

const int NUM_PROCESSES = 5;
const int SIMULATION_STEPS = 10;

std::vector<std::atomic<int>> clocks(NUM_PROCESSES);
std::vector<std::mutex> clock_mutexes(NUM_PROCESSES);
const std::vector<int> baseIncrements = {1, 2, 3, 4, 5};
const std::vector<std::pair<int, int>> messageSequence = {
    {0, 1}, {2, 3}, {1, 4}, {3, 0}, {4, 2},
    {1, 3}, {0, 2}, {4, 1}, {3, 4}, {2, 0}
};

void processFunction(int id) {
    std::random_device random;
    std::mt19937 gen(random());
    std::uniform_int_distribution<> dis(1, 5);

    for (int step = 0; step < SIMULATION_STEPS; ++step) {
        int randomIncrement = baseIncrements[id] + dis(gen);

        {
            std::lock_guard<std::mutex> lock(clock_mutexes[id]);
            clocks[id] += randomIncrement;
            std::cout << "Proces " << id << " inkrementuje swoj zegar o " 
                      << randomIncrement << " do " << clocks[id] 
                      << " (krok " << step << ")" << std::endl;
        }

        for (const auto& pair : messageSequence) {
            if (pair.first == id && step == pair.second) {
                int recipient = pair.second;
                int sender_time;
                {
                    std::lock_guard<std::mutex> lock(clock_mutexes[id]);
                    sender_time = clocks[id];
                }

                std::lock_guard<std::mutex> lock(clock_mutexes[recipient]);
                clocks[recipient] = std::max(clocks[recipient].load(), sender_time);
                std::cout << "Proces " << id << " wysyla swoj czas " << sender_time 
                          << " do procesu " << recipient << ". Zegar procesu " 
                          << recipient << " ustawiony na " << clocks[recipient] 
                          << " (krok " << step << ")" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        clocks[i] = 0;
    }
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        threads.emplace_back(processFunction, i);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "\nStan koncowy zegarow procesow:\n";
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        std::cout << "Zegar procesu " << i << ": " << clocks[i] << std::endl;
    }

    return 0;
}
