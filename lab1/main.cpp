#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <cstring>

using namespace std;

int main() {
    string line;
    getline(cin, line);

    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) == -1 or pipe(pipe2) == -1) {
        cerr << "Pipe error" << endl;
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        cerr << "Fork error" << endl;
        return 1;
    }

    if (pid1 == 0) {
        close(pipe1[0]);
        if (dup2(pipe1[1], STDOUT_FILENO) == -1) {
            cerr << "Dup2 error" << endl;
            return 1;
        }

        if (execl("./first_child", "./first_child", line.c_str(), nullptr) == -1) {
            cerr << "Execl error" << strerror(errno) << endl;
            return 1;
        }

    }

    close(pipe1[1]);

    pid_t pid2 = fork();
    if (pid2 == -1) {
        cerr << "Fork error" << endl;
        return 1;
    }

    if (dup2(pipe1[0], STDIN_FILENO) == -1) {
        cerr << "Dup2 error" << endl;
        return 1;
    }

    if (pid2 == 0) {
        close(pipe2[0]);
        getline(cin, line);
        close(pipe1[0]);
        if (dup2(pipe2[1], STDOUT_FILENO) == -1) {
            cerr << "Dup2 error" << endl;
            return 1;
        }

        if (execl("./second_child", "./second_child", line.c_str(), nullptr) == -1) {
            cerr << "Execl error" << strerror(errno) << endl;
            return 1;
        }
    }

    wait(nullptr);
    close(pipe1[0]);
    close(pipe2[1]);

    if (pid1 > 0 and pid2 > 0) {
        if (dup2(pipe2[0], STDIN_FILENO) == -1) {
            cerr << "Dup2 error" << endl;
            return 1;
        }
        getline(cin, line);
        cout << line << endl;
    }

    return 0;
}