#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include "utils.cpp"

std::unordered_map<std::string, std::unordered_set<std::string>> group_chats{};
std::unordered_set<std::string> clients{};

void *handle_client(void *args) {
    std::string login = *static_cast<std::string *>(args);
    int fd = open(("./chats/user_" + login + ".chat").c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, PAGE_SIZE);

    char *buff_from_client = static_cast<char *>(mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    buff_from_client[0] = '0';
    msync(buff_from_client, 1, MS_SYNC);

    while (true) {
        while (buff_from_client[0] != '1');
        std::string tmp;
        size_t i = 1;

        // receive recipient_login and msg_text
        read_string(i, tmp, buff_from_client);
        int query_type = std::stoi(tmp);

        switch (query_type) {
            case client_query_type::send_msg: {

                std::string recipient_login;
                std::string msg_text;
                read_string(i, recipient_login, buff_from_client);
                read_string(i, msg_text, buff_from_client);

                // check if recipient is a user or a group
                if (group_chats.find(recipient_login) == group_chats.end()) {
                    if (clients.find(recipient_login) == clients.end()) {
                        std::cout << "Client with login '" << recipient_login << "' doesn't exist" << std::endl;
                        break;
                    }
                    int fd_other_client = open(("./chats/server_" + recipient_login + ".chat").c_str(), O_RDWR,
                                               S_IRUSR | S_IWUSR);
                    ftruncate(fd, PAGE_SIZE);
                    char *buff_to_other_client = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE,
                                                                          MAP_SHARED,
                                                                          fd_other_client, 0));
                    while (buff_to_other_client[0] != '0');

                    // write info
                    size_t j = 1;
                    std::string full_msg;
                    full_msg += login;
                    full_msg += '\0';
                    full_msg += msg_text;
                    write_string(j, full_msg, buff_to_other_client);

                    // close session
                    buff_to_other_client[j] = EOF;
                    buff_to_other_client[0] = '1';
                    munmap(buff_to_other_client, PAGE_SIZE);
                    close(fd_other_client);
                } else {
                    for (const auto& curr_recipient_login : group_chats[recipient_login]) {
                        int fd_other_client = open(("./chats/server_" + curr_recipient_login + ".chat").c_str(), O_RDWR,
                                                   S_IRUSR | S_IWUSR);
                        ftruncate(fd, PAGE_SIZE);
                        char *buff_to_other_client = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE,
                                                                              MAP_SHARED,
                                                                              fd_other_client, 0));
                        while (buff_to_other_client[0] != '0');

                        // write info
                        size_t j = 1;
                        std::string full_msg;
                        full_msg += recipient_login;
                        full_msg += " / ";
                        full_msg += login;
                        full_msg += '\0';
                        full_msg += msg_text;
                        write_string(j, full_msg, buff_to_other_client);

                        // close session
                        buff_to_other_client[j] = EOF;
                        buff_to_other_client[0] = '1';
                        munmap(buff_to_other_client, PAGE_SIZE);
                        close(fd_other_client);
                    }
                }
                break;
            }
            case client_query_type::create_group: {
                std::string group_name;
                read_string(i, group_name, buff_from_client);

                if (group_chats.find(group_name) == group_chats.end()) {
                    group_chats[group_name] = { login };
                } else {
                    std::cout << "Group with the name '" << group_name << "' already exists! Please try again" << std::endl;
                }

                break;
            }
            case client_query_type::join_group: {
                std::string group_name;
                read_string(i, group_name, buff_from_client);

                if (group_chats.find(group_name) != group_chats.end()) {
                    group_chats[group_name].insert(login);
                } else {
                    std::cout << "Group with the name '" << group_name << "' doesn't exist! Please try again." << std::endl;
                }

                break;
            }
            case client_query_type::leave_group: {
                std::string group_name;
                read_string(i, group_name, buff_from_client);

                if (group_chats.find(group_name) != group_chats.end()) {
                    group_chats[group_name].erase(login);
                } else {
                    std::cout << "Group with the name '" << group_name << "' doesn't exist! Please try again." << std::endl;
                }

                break;
            }
            case client_query_type::exit_all: {
                for (auto &c : group_chats) {
                    if (c.second.find(login) != c.second.end()) {
                        c.second.erase(login);
                    }
                }
                buff_from_client[0] = '0';
                munmap(buff_from_client, PAGE_SIZE);
                close(fd);

                return nullptr;
            }
        }

        // update info
        buff_from_client[0] = '0';
        msync(buff_from_client, i + 1, MS_SYNC);

    }

    munmap(buff_from_client, PAGE_SIZE);
    close(fd);
}

void *handle_registration(void *args) {
    int fd = open("./chats/registration.chat", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, PAGE_SIZE);

    char *from_clients = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    from_clients[0] = '0';
    msync(from_clients, 1, MS_SYNC);

    while (true) {
        while (from_clients[0] != '1');
        std::string sender_login;
        size_t i = 1;
        read_string(i, sender_login, from_clients);

        clients.insert(sender_login);
        pthread_t client_thread;
        pthread_create(&client_thread, nullptr, handle_client, &sender_login);
        std::cout << "Client with login '" << sender_login << "' has jumped into the server!" << std::endl;

        from_clients[0] = '0';
        msync(from_clients, i + 1, MS_SYNC);
    }

    munmap(from_clients, PAGE_SIZE);
    close(fd);
}

int main() {

    pthread_t registration_thread;
    pthread_create(&registration_thread, nullptr, handle_registration, nullptr);

    std::cout << "Available commands:" << std::endl;
    std::cout << "1) See group chats and members in them." << std::endl;
    while (true) {
        std::cout << "Enter command type:" << std::endl;
        int command_type;
        std::cin >> command_type;
        switch (command_type) {
            case 1: {
                for (const auto& c : group_chats) {
                    std::cout << "Clients in group chat '" << c.first << "' are: ";
                    for (const auto& d : c.second) {
                        std::cout << d << "; ";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
                break;
            }
            default: {
                std::cout << "Wrong type of command! Try again." << std::endl;
                break;
            }
        }
    }
}
