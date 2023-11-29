#include <cmath>
#include <dlfcn.h>
#include <iomanip>
#include <iostream>

int main() {
    int lib_num = 0;
    std::cout << "Enter lib number (1 or 2):" << std::endl;
    std::cin >> lib_num;

    if (lib_num != 1 && lib_num != 2) {
        perror("Not a lib number");
        return -1;
    }

    int64_t command = 0;
    const char *libs_list[2] = {"./lib1.so", "./lib2.so"};
    void *lib_header;
    --lib_num;
    lib_header = dlopen(libs_list[lib_num], RTLD_LAZY);
    if (lib_header == NULL) {
        perror(dlerror());
        return -1;
    }

    float (*Pi)(size_t k);
    int64_t *(*MySort)(int64_t *arr, size_t sz);

    MySort = (int64_t *(*)(int64_t *, size_t)) dlsym(lib_header, "MySort");
    if (MySort == NULL) {
        perror(dlerror());
        return -1;
    }

    Pi = (float (*)(size_t)) dlsym(lib_header, "Pi");
    if (Pi == NULL) {
        perror(dlerror());
        return -1;
    }

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

        } else if (command == 0) {

            if (dlclose(lib_header) != 0) {
                perror(dlerror());
                return -1;
            }
            lib_num = (lib_num + 1) % 2;
            lib_header = dlopen(libs_list[lib_num], RTLD_LAZY);
            if (lib_header == NULL) {
                perror(dlerror());
                return -1;
            }

            MySort = (int64_t *(*)(int64_t *, size_t)) dlsym(lib_header, "MySort");
            if (MySort == NULL) {
                perror(dlerror());
                return -1;
            }

            Pi = (float (*)(size_t)) dlsym(lib_header, "Pi");
            if (Pi == NULL) {
                perror(dlerror());
                return -1;
            }

            std::cout << "Libs have been successfully switched. Current is lib" << lib_num + 1 << std::endl;
        } else {
            std::cout << "Wrong command" << std::endl;
        }

        std::cout << "Enter command:" << std::endl;
    }
    if (dlclose(lib_header) != 0) {
        perror(dlerror());
        return -1;
    }

    return 0;
}