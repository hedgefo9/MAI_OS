#include <cmath>
#include <iomanip>
#include <iostream>

extern "C" float Pi(size_t k);
extern "C" int64_t *MySort(int64_t *arr, size_t sz);

float Pi(size_t k) {
    std::cout << "It's a method which uses Leibniz series!" << std::endl;

    float pi = 0;
    int curr_sign = 1, denominator = 1;
    for (size_t i = 0; i < k; ++i) {
        pi += static_cast<float>(curr_sign) * (4.0 / static_cast<float>(denominator));
        curr_sign *= -1;
        denominator += 2;
    }

    return pi;
}

int64_t *MySort(int64_t *arr, size_t sz) {
    std::cout << "It's Bubble sort!" << std::endl;

    auto *sorted_arr = new int64_t[sz];
    std::copy(arr, arr + sz, sorted_arr);

    for (size_t i = 0; i < sz - 1; ++i) {
        for (size_t j = 0; j < sz - i - 1; ++j) {
            if (sorted_arr[j] > sorted_arr[j + 1]) {
                std::swap(sorted_arr[j], sorted_arr[j + 1]);
            }
        }
    }

    return sorted_arr;
}