#include <cmath>
#include <iomanip>
#include <iostream>

extern "C" float Pi(size_t k);
extern "C" int64_t *MySort(int64_t *arr, size_t sz);

float Pi(size_t k) {
    std::cout << "It's a method which uses the Wallis formula!" << std::endl;

    float pi = 1;
    for (int64_t i = 1; i <= k; ++i) {
        pi *= (static_cast<float>(4 * i * i) / static_cast<float>(4 * i * i - 1));
    }
    pi *= 2;

    return pi;
}

int64_t partition(int64_t *arr, int64_t low, int64_t high) {
    int64_t ref_element = arr[high];
    int64_t i = low - 1;

    for (size_t j = low; j <= high - 1; j++) {
        if (arr[j] < ref_element) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }

    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

int64_t *QuickHoarSort(int64_t *arr, int64_t low, int64_t high) {
    if (low < high) {
        int64_t pi = partition(arr, low, high);

        QuickHoarSort(arr, low, pi - 1);
        QuickHoarSort(arr, pi + 1, high);
    }

    return arr;
}

int64_t *MySort(int64_t *arr, size_t sz) {
    std::cout << "It's Hoar sort!" << std::endl;

    auto *sorted_arr = new int64_t[sz];
    std::copy(arr, arr + sz, sorted_arr);

    QuickHoarSort(sorted_arr, 0, sz - 1);

    return sorted_arr;
}