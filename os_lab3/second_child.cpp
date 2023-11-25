
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <sys/stat.h>

using namespace std;

int main(int argc, const char *argv[]) {

    const char *filepath = "./data.bin";
    int fd = open(filepath, O_RDWR | O_APPEND, 0666);

    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    struct stat fileInfo = {0};

    if (fstat(fd, &fileInfo) == -1) {
        perror("Error getting the file size");
        exit(EXIT_FAILURE);
    }

    ftruncate(fd, fileInfo.st_size);

    char *map = (char *) mmap(nullptr, fileInfo.st_size + 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }


    char *c = map;
    for (size_t i = 0; i < fileInfo.st_size; ++i) {
        if (*c == ' ') {
            *c = '_';
        }
        ++c;
    }

    if (msync(map, fileInfo.st_size + 1, MS_SYNC) == -1) {
        perror("Could not sync the file to disk");
    }

    if (munmap(map, fileInfo.st_size + 1) == -1) {
        close(fd);
        perror("Error un-mmapping the file");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return 0;
}
