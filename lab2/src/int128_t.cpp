
#include "../include/int128_t.hpp"

int128_t::int128_t(__int128 a): x(a) {}

void int128_t::read() {
    __int128 curr_x = 0, f = 1;
    char ch = getchar();
    while (!isdigit(ch) && !(ch >= 'A' && ch <= 'F')) {
        if (ch == '-') {
            f = -1;
            ch = getchar();
            break;
        }
        ch = getchar();
    }
    while (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F')) {
        curr_x = curr_x * 16;
        if ('0' <= ch && ch <= '9') {
            curr_x += ch - '0';
        } else if ('A' <= ch && ch <= 'F') {
            curr_x += (ch - 'A') + 10;
        }
        ch = getchar();
    }
    x = curr_x * f;
}

void int128_t::print() {
    print(x);
}

void int128_t::print(__int128 a) {
    if (a < 0) {
        putchar('-');
        a = -a;
    }
    if (a > 15) print(a / 16);
    __int128 curr_digit = a % 16;
    if (0 <= curr_digit && curr_digit <= 9) {
        putchar(curr_digit + '0');
    } else if (10 <= curr_digit && curr_digit <= 15) {
        putchar((curr_digit - 10) + 'A');
    }

}

void int128_t::operator+=(const int128_t& other) {
    x += other.x;
}

int128_t int128_t::operator/(const int128_t& other) const {
    return {x / other.x};
}