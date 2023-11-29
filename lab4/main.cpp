#include <cmath>
#include <iomanip>
#include <iostream>

extern "C" float Pi(size_t k);
extern "C" int64_t *MySort(int64_t *arr, size_t sz);

int main() {
    int command = 0;
    std::cout << "Enter command (1 - calc Pi, 2 - sort array):" << std::endl;

    while (std::cin >> command) {
        if (command == 1) {

            std::cout << "Enter quantity of steps to calc Pi" << std::endl;
            size_t k;
            std::cin >> k;
            std::cout << std::setprecision(10) << Pi(k) << std::endl;

        } else if (command == 2) {

            std::cout << "Enter quantity of numbers which must be sorted" << std::endl;
            size_t a;
            std::cin >> a;
            std::cout << "Enter these numbers" << std::endl;
            auto *arr = new int64_t[a];
            for (size_t i = 0; i < a; ++i) {
                std::cin >> *(arr + i);
            }
            auto *res = MySort(arr, a);
            std::cout << std::setprecision(10);
            for (size_t i = 0; i < a; ++i) {
                std::cout << *(res + i) << "\t";
            }
            std::cout << std::endl;

        } else {
            std::cout << "Wrong command" << std::endl;
        }

        std::cout << "Enter command:" << std::endl;
    }
    return 0;
}