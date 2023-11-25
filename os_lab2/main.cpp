#include <iostream>
#include <fstream>
#include <vector>
#include "include/int128_t.hpp"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>

std::vector<int128_t> numbers;
std::vector<int128_t> part_sums;

struct msg_to_thread {
    size_t thread_count;
    size_t thread_order_id;
};

void* part_sum(void *args) {
    auto *msg = (msg_to_thread *) args;
    size_t thread_order_id = msg->thread_order_id;
    size_t thread_count = msg->thread_count;
    for (size_t i = thread_order_id; i < numbers.size();) {
        part_sums[thread_order_id] += numbers[i];
        i += thread_count;
    }
    pthread_exit(0);
}

int main() {

    size_t quantity;
    int fd = open("input.txt", std::ios::in);
    if (dup2(fd, STDIN_FILENO) == -1) {
        std::cerr << "Dup2 error" << std::endl;
    }
    std::cin >> quantity;
    numbers.resize(quantity);
    for (size_t i = 0; i < quantity; ++i) {
        int128_t a{};
        a.read();
        numbers[i] = a;
    }

    auto t0 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::now());

    for (size_t j = 1; j <= 4; ++j) {

        part_sums.clear();
        const size_t thread_count = j;
        part_sums.resize(thread_count);

        auto start = std::chrono::high_resolution_clock::now();
        pthread_t tid[thread_count];
        for (size_t i = 0; i < thread_count; i++) {
            auto *msg = (msg_to_thread *) malloc(sizeof(msg_to_thread));
            msg->thread_count = thread_count;
            msg->thread_order_id = i;
            if (pthread_create(&tid[i], NULL, part_sum, (void *) msg) != 0) {
                std::cerr << "Pthread create error" << std::endl;
            }
        }
        for (size_t i = 0; i < thread_count; i++) {
            if (pthread_join(tid[i], NULL) != 0) {
                std::cerr << "Pthread join error" << std::endl;
            }
        }

        int128_t final_sum(0);
        for (size_t i = 0; i < thread_count; ++i) {
            final_sum += part_sums[i];
        }
        int128_t average(final_sum / numbers.size());
        clock_t end = clock();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        if (j == 1) {
            t0 = duration;
            average.print();
            std::cout << std::endl;

        }

        size_t duration_ms = duration.count() / 1000;
        double accel = static_cast<double>(t0.count()) / static_cast<double>(duration.count());
        double efficiency = accel / static_cast<double>(thread_count);
        printf("%ld;%ld;%f;%f\n", thread_count, duration_ms, accel, efficiency);
        // printf("%ld;%f\n", thread_count, efficiency);
    }
    return 0;
}