#include "zmq.hpp"
#include "iostream"
#include <chrono>

bool is_last = true;
const size_t base_port = 5555;

std::string get_addr(size_t port) {
    return "tcp://localhost:" + std::to_string(port);
}

int main(int argc, char *argv[]) {
    zmq::context_t ctx{};
    zmq::socket_t socket(ctx, zmq::socket_type::rep);
    zmq::socket_t socket_next(ctx, zmq::socket_type::req);

    size_t my_id = std::stoull(std::string(argv[1]));
    socket.bind("tcp://localhost:" + std::to_string(base_port + my_id));

    auto begin = std::chrono::high_resolution_clock::now(),
            end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    bool is_active = false;

    while (true) {
        zmq::message_t request;
        socket.recv(request, zmq::recv_flags::none);

        std::string req_text = request.to_string();
        std::cout << "Received request:\t" + req_text << std::endl;
/*            if (is_number(reply.to_string())) {
                size_t i = 0;
                for (; i < nodes.size(); ++i) {
                    if (nodes[i] == std::stoull(reply.to_string())) {
                        break;
                    }
                }
                for (; i < nodes.size(); ++i) {
                    nodes.pop_back();
                }
            }*/
        std::cout << "Sending reply" << std::endl;

        std::stringstream ss(req_text);
        std::string command;
        ss >> command;

        if (command == "exit") {

            if (!is_last) {
                socket_next.send(zmq::buffer(req_text));
                zmq::message_t reply;
                socket_next.recv(reply, zmq::recv_flags::none);
            }

            exit(0);
        } else if (command == "create") {
            size_t id;
            ss >> id;

            if (is_last) {
                is_last = false;

                int pid = fork();
                if (pid == 0) {
                    execl("./node", "./node", std::to_string(id).c_str(), (char *) 0);
                }

                socket.send(zmq::buffer("Ok: " + std::to_string(pid)), zmq::send_flags::dontwait);
                socket_next.connect(get_addr(base_port + id));

                socket_next.set(zmq::sockopt::rcvtimeo, 1000);
                socket_next.set(zmq::sockopt::sndtimeo, 1000);

            } else {
                socket_next.send(zmq::buffer(req_text));
                zmq::message_t reply;
                socket_next.recv(reply, zmq::recv_flags::none);

                socket.send(zmq::buffer(reply.to_string()), zmq::send_flags::dontwait);
            }
        } else if (command == "kill") {
            size_t id;
            ss >> id;

            if (id == my_id) {
                if (!is_last) {
                    socket_next.send(zmq::str_buffer("exit"));
                    zmq::message_t reply;
                    socket_next.recv(reply, zmq::recv_flags::none);
                }

                socket.send(zmq::str_buffer("Ok"), zmq::send_flags::dontwait);

                exit(0);
            } else {
                if (!is_last) {
                    socket_next.send(zmq::buffer(req_text));
                    zmq::message_t reply;
                    socket_next.recv(reply, zmq::recv_flags::none);

                    socket.send(zmq::buffer(reply.to_string()), zmq::send_flags::dontwait);
                } else {
                    socket.send(zmq::buffer("Error: Not found"), zmq::send_flags::dontwait);
                }
            }

        } else if (command == "exec") {
            size_t id;
            ss >> id;

            if (id == my_id) {

                std::string subcommand;
                ss >> subcommand;
                if (subcommand == "time") {
                    if (is_active) {
                        duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::high_resolution_clock::now() - begin);
                    } else {
                        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
                    }

                    socket.send(zmq::buffer("Ok:" + std::to_string(my_id) + ": " + std::to_string(duration.count())), zmq::send_flags::dontwait);
                } else if (subcommand == "start") {
                    if (!is_active) {
                        begin = std::chrono::high_resolution_clock::now();
                        is_active = true;
                    }
                    socket.send(zmq::buffer("Ok:" + std::to_string(my_id)), zmq::send_flags::dontwait);
                } else if (subcommand == "stop") {
                    if (is_active) {
                        end = std::chrono::high_resolution_clock::now();
                        is_active = false;
                    }
                    socket.send(zmq::buffer("Ok:" + std::to_string(my_id)), zmq::send_flags::dontwait);
                }

            } else {
                if (!is_last) {
                    socket_next.send(zmq::buffer(req_text));
                    zmq::message_t reply;
                    socket_next.recv(reply, zmq::recv_flags::none);

                    socket.send(zmq::buffer(reply.to_string()), zmq::send_flags::dontwait);
                } else {
                    socket.send(zmq::buffer("Error:" + std::to_string(my_id) + ": Not found"), zmq::send_flags::dontwait);
                }
            }
        } else if (command == "ping") {
            size_t id;
            ss >> id;

            if (id == my_id) {
                socket.send(zmq::buffer("Ok"), zmq::send_flags::dontwait);
            } else {

                socket_next.send(zmq::buffer(req_text));
                zmq::message_t reply;
                socket_next.recv(reply, zmq::recv_flags::none);

                if (reply.to_string().empty()) {
                    socket.send(zmq::buffer(std::to_string(my_id)));
                    is_last = true;
                } else {
                    socket.send(zmq::buffer(reply.to_string()));
                }
            }
        } else if (command == "set_last") {
            size_t id;
            ss >> id;

            if (id == my_id) {
                is_last = true;
            } else {
                socket_next.send(zmq::buffer(req_text));

                zmq::message_t reply;
                socket_next.recv(reply, zmq::recv_flags::none);
            }
            socket.send(zmq::str_buffer("Ok"), zmq::send_flags::dontwait);
        }
    }
}
