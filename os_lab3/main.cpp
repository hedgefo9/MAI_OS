#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <cstring>

using namespace std;

int main() {
    string line;
    getline(cin, line);

    const char *filepath = "./data.bin";
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    size_t line_size = line.size() * sizeof(char);

    ftruncate(fd, line_size);

    char *map = (char *) mmap(nullptr, line_size + 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < line_size; i++) {
        map[i] = line[i];
    }

    if (msync(map, line_size, MS_SYNC) == -1) {
        perror("Could not sync the file to disk");
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        cerr << "Fork error" << endl;
        return 1;
    }

    if (pid1 == 0) {
        if (execl("./first_child", "./first_child") == -1) {
            cerr << "Execl error" << strerror(errno) << endl;
            return 1;
        }
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        cerr << "Fork error" << endl;
        return 1;
    }

    if (pid2 == 0) {
        if (execl("./second_child", "./second_child") == -1) {
            cerr << "Execl error" << strerror(errno) << endl;
            return 1;
        }
    }

    wait(nullptr);

    if (pid1 > 0 and pid2 > 0) {
        write(STDOUT_FILENO, map, line_size);

        if (munmap(map, line_size + 1) == -1) {
            close(fd);
            perror("Error un-mmapping the file");
            exit(EXIT_FAILURE);
        }

        close(fd);
    }

    return 0;
}