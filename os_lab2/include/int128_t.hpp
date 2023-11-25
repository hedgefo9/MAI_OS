#ifndef INT128_THPPINCLUDED
#define INT128_THPPINCLUDED

#include <iostream>

class int128_t {
    __int128 x = 0;

public:
    int128_t() = default;
    ~int128_t() = default;

    int128_t(__int128 a);
    void read();
    void print();
    void print(__int128 a);

    void operator+=(const int128_t& other);
    int128_t operator/(const int128_t& other) const;
};

#include "../src/int128_t.cpp"

#endif