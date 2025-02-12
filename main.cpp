#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <cmath>

std::mutex mtx;

double sum_single_thread(const std::vector<int>& vec) {
    double total_sum = 0;
    for (int num : vec) {
        total_sum += std::sqrt(num);
    }
    return total_sum;
}

void sum_multi_thread(const std::vector<int>& vec, double& total_sum, int start, int end) {
    double local_sum = 0;
    for (int i = start; i < end; ++i) {
        local_sum += std::sqrt(vec[i]);
    }
    std::lock_guard<std::mutex> guard(mtx);
    total_sum += local_sum;
}

int main() {
    const int size = 1000000;
    const int num_threads = 4;

    std::vector<int> vec(size);

    std::mt19937 gen(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> dis(1, 100);

    for (int i = 0; i < size; ++i) {
        vec[i] = dis(gen);
    }

    auto start = std::chrono::high_resolution_clock::now();
    double total_sum_single = sum_single_thread(vec);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_single = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Сумма квадратных корней (один поток): " << total_sum_single << std::endl;
    std::cout << "Время выполнения (один поток): " << duration_single.count() << " мс" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    double total_sum_multi = 0;
    std::vector<std::thread> threads;
    int chunk_size = size / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        int start_idx = i * chunk_size;
        int end_idx = (i == num_threads - 1) ? size : (i + 1) * chunk_size;
        threads.push_back(std::thread(sum_multi_thread, std::cref(vec), std::ref(total_sum_multi), start_idx, end_idx));
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    end = std::chrono::high_resolution_clock::now();
    auto duration_multi = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Сумма квадратных корней (многопоточно): " << total_sum_multi << std::endl;
    std::cout << "Время выполнения (многопоточно): " << duration_multi.count() << " мс" << std::endl;

    return 0;
}
