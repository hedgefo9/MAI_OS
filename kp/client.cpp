#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "utils.cpp"

std::string login;

void *handle_server(void *) {
    int fd = open(("./chats/server_" + login + ".chat").c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, PAGE_SIZE);

    char *from_server = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    from_server[0] = '0';
    msync(from_server, 1, MS_SYNC);

    while (true) {
        while (from_server[0] != '1');
        std::string sender_login;
        std::string msg_text;
        size_t i = 1;

        read_string(i, sender_login, from_server);
        read_string(i, msg_text, from_server);

        std::cout << "[" << sender_login << "]: " << msg_text << std::endl;

        from_server[0] = '0';
        msync(from_server, i + 1, MS_SYNC);
    }

    munmap(from_server, PAGE_SIZE);
    close(fd);
}

int main() {
    std::cout << "Enter you login:" << std::endl;
    std::cin >> login;

    // registration
    int fd_reg = open("./chats/registration.chat", O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd_reg, PAGE_SIZE);
    char *to_reg = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_reg, 0));

    // wait for server to be ready receive message
    while (to_reg[0] != '0');

    size_t k = 1;
    write_string(k, login, to_reg);

    to_reg[k] = EOF;
    to_reg[0] = '1';
    munmap(to_reg, PAGE_SIZE);
    close(fd_reg);
    usleep(50 * 1000);

    // get ready for sending messages to server
    int fd = open(("./chats/user_" + login + ".chat").c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, PAGE_SIZE);

    char *to_server = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    to_server[0] = '0';

    // attach message listener for server
    pthread_t client_thread;
    pthread_create(&client_thread, nullptr, handle_server, nullptr);

    std::cout << "Available commands:" << std::endl;
    std::cout << "1) Send message;" << std::endl;
    std::cout << "2) Create new group chat;" << std::endl;
    std::cout << "3) Join group chat;" << std::endl;
    std::cout << "4) Leave group chat;" << std::endl;
    std::cout << "5) Exit." << std::endl;

    // send messages or other information to server
    while (true) {

        std::cout << "Enter command:" << std::endl;
        int command_type;
        std::cin >> command_type;
        --command_type;

        switch (command_type) {
            case client_query_type::send_msg: {
                // wait for server to be ready receive message
                while (to_server[0] != '0');

                std::cout << "Enter recipient's login:" << std::endl;
                std::string recipient_login;
                std::cin >> recipient_login;
                std::cout << "Write your message:" << std::endl;
                std::string msg_text;
                std::cin.ignore();
                std::getline(std::cin, msg_text);

                size_t i = 1;
                std::string full_msg;
                full_msg += std::to_string(client_query_type::send_msg);
                full_msg += '\0';
                full_msg += recipient_login;
                full_msg += '\0';
                full_msg += msg_text;
                write_string(i, full_msg, to_server);

                to_server[i] = EOF;
                to_server[0] = '1';
                msync(to_server, i + 1, MS_SYNC);
                break;
            }
            case client_query_type::create_group:
            case client_query_type::join_group:
            case client_query_type::leave_group: {
                // wait for server to be ready receive message
                while (to_server[0] != '0');
                std::cout << "Enter group's name:" << std::endl;
                std::string group_name;
                std::cin >> group_name;

                size_t i = 1;
                std::string full_msg;
                full_msg += std::to_string(command_type);
                full_msg += '\0';
                full_msg += group_name;
                write_string(i, full_msg, to_server);

                to_server[i] = EOF;
                to_server[0] = '1';
                msync(to_server, i + 1, MS_SYNC);
                break;
            }
            case exit_all: {
                // wait for server to be ready receive message
                while (to_server[0] != '0');

                size_t i = 1;
                std::string full_msg;
                full_msg += std::to_string(command_type);
                write_string(i, full_msg, to_server);

                to_server[i] = EOF;
                to_server[0] = '1';
                msync(to_server, i + 1, MS_SYNC);
                munmap(to_server, PAGE_SIZE);
                close(fd);

                pthread_cancel(client_thread);
                std::cout << "Logged off." << std::endl;
                return 0;
            }
            default: {
                std::cout << "Wrong command! Please try again." << std::endl;
            }
        }
    }
}
