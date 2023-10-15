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
    int pipe3[2];
    if (pipe(pipe1) == -1 or pipe(pipe2) == -1 or pipe(pipe3) == -1) {
        cerr << "Pipe error" << endl;
        return 1;
    }

    size_t line_len = line.length();
    write(pipe1[1], &line_len, sizeof(size_t));
    write(pipe1[1], line.c_str(), sizeof(char) * line.length());

    pid_t pid1 = fork();
    if (pid1 == -1) {
        cerr << "Fork error" << endl;
        return 1;
    }

    if (pid1 == 0) {
        close(pipe1[1]);
        size_t received_line_len;
        read(pipe1[0], &received_line_len, sizeof(size_t));
        char received_line[received_line_len + 1];
        read(pipe1[0], received_line, received_line_len * sizeof(char));
        received_line[received_line_len] = '\0';

        close(pipe3[0]);
        if (dup2(pipe3[1], STDOUT_FILENO) == -1) {
            cerr << "Dup2 error" << endl;
            return 1;
        }

        if (execl("./first_child", "./first_child", received_line, nullptr) == -1) {
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
        close(pipe3[1]);
        if (dup2(pipe3[0], STDIN_FILENO) == -1) {
            cerr << "Dup2 error" << endl;
            return 1;
        }
        string received_line;
        getline(cin, received_line);

        close(pipe2[0]);
        if (dup2(pipe2[1], STDOUT_FILENO) == -1) {
            cerr << "Dup2 error" << endl;
            return 1;
        }

        if (execl("./second_child", "./second_child", received_line.c_str(), nullptr) == -1) {
            cerr << "Execl error" << strerror(errno) << endl;
            return 1;
        }
    }

    wait(nullptr);

    if (pid1 > 0 and pid2 > 0) {
        close(pipe2[1]);
        if (dup2(pipe2[0], STDIN_FILENO) == -1) {
            cerr << "Dup2 error" << endl;
            return 1;
        }
        getline(cin, line);
        cout << line << endl;
    }

    return 0;
}