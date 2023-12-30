
#include <iostream>
#define PAGE_SIZE 4096

enum client_query_type {
    send_msg,
    create_group,
    join_group,
    leave_group,
    exit_all
};

void write_string(size_t &pos, const std::string &str, char *buffer) {
    for (char c : str) {
        buffer[pos] = c;
        ++pos;
    }
}

void read_string(size_t &pos, std::string &str, char *buffer) {
    for (; pos < PAGE_SIZE;) {
        char c = buffer[pos];
        buffer[pos] = '\0';
        if (c == '\0' || c == EOF) break;
        str += c;
        ++pos;
    }
    ++pos;
}
