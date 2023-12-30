#include <iostream>
#include <zmq.hpp>
#include <unordered_set>

const size_t base_port = 5555;

std::string get_addr(size_t port) {
    return "tcp://localhost:" + std::to_string(port);
}

bool ping(zmq::socket_t& socket, size_t id) {
    socket.connect(get_addr(base_port + id));
    socket.send(zmq::str_buffer("pid"), zmq::send_flags::dontwait);

    zmq::message_t reply;

    socket.recv(reply, zmq::recv_flags::none);

    socket.disconnect(get_addr(base_port + id));

    return !reply.to_string().empty();

}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

int main() {
    zmq::context_t context{};
    zmq::socket_t socket{context, zmq::socket_type::req};
    socket.set(zmq::sockopt::rcvtimeo, 1000);
    socket.set(zmq::sockopt::sndtimeo, 1000);
    socket.set(zmq::sockopt::req_correlate, 1);
    socket.set(zmq::sockopt::req_relaxed, 1);

    std::vector<size_t> nodes;

    std::string req_text;
    while (true) {
        getline(std::cin, req_text);
        std::stringstream ss(req_text);

        std::string command;
        ss >> command;

        zmq::message_t reply;

        // ping
        if (!nodes.empty()) {
            socket.send(zmq::buffer("ping " + std::to_string(nodes.back())), zmq::send_flags::dontwait);
            socket.recv(reply, zmq::recv_flags::none);

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
        }

        if (command == "create") {
            size_t id;
            ss >> id;
            if (nodes.empty()) {

                int pid = fork();
                if (pid == 0) {
                    execl("./node", "./node", std::to_string(id).c_str(), (char *) 0);
                }

                socket.connect(get_addr(base_port + id));
                std::cout << "Ok: " << std::to_string(pid) << std::endl;
            } else {
                socket.send(zmq::buffer(req_text), zmq::send_flags::dontwait);

                socket.recv(reply, zmq::recv_flags::none);
                std::cout << reply.to_string() << std::endl;
            }
            nodes.push_back(id);
        } else if (command == "kill") {
            size_t id;
            ss >> id;

            if (id == nodes.front()) {
                nodes.clear();
            } else if (id == nodes.back()) {
                nodes.pop_back();
            } else {
                size_t i = 0;
                for (; i < nodes.size(); ++i) {
                    if (nodes[i] == id) {
                        break;
                    }
                }
                for (; i <= nodes.size(); ++i) {
                    nodes.pop_back();
                }
            }


            socket.send(zmq::buffer(req_text), zmq::send_flags::dontwait);
            socket.recv(reply, zmq::recv_flags::none);

            socket.send(zmq::buffer("set_last " + std::to_string(nodes.back())), zmq::send_flags::dontwait);
            socket.recv(reply, zmq::recv_flags::none);

            std::cout << "Ok" << std::endl;
        } else {
            socket.send(zmq::buffer(req_text), zmq::send_flags::dontwait);

            socket.recv(reply, zmq::recv_flags::none);
            std::cout << reply.to_string() << std::endl;
        }

/*        for (int j = 0; j < nodes.size(); ++j) {
            std::cout << nodes[j] << std::endl;
        }*/

        /*
        if (command == "create") {
            size_t id;
            std::cin >> id;

            int pid = fork();
            if (pid == 0) {
                execl("./node", "./node", std::to_string(base_port + id).c_str(), (char *) 0);
            }

/*          socket.connect(get_addr(base_port + id));


            socket.send(zmq::str_buffer("pid"), zmq::send_flags::dontwait);

            zmq::message_t reply;
            socket.recv(reply, zmq::recv_flags::none);
            nodes.insert(id);

            socket.disconnect(get_addr(base_port + id));
        } else if (command == "kill") {
            size_t id;
            std::cin >> id;

            if (nodes.count(id) == 0 || !ping(socket, id)) {
                continue;
            }

            socket.connect(get_addr(base_port + id));
            socket.send(zmq::str_buffer("exit"), zmq::send_flags::dontwait);

            zmq::message_t reply;
            socket.recv(reply, zmq::recv_flags::none);
            if (reply.to_string() == "OK") {
                std::cout << "Node with id " + std::to_string(id) + " has been successfully killed." << std::endl;
                nodes.erase(id);
            }

            socket.disconnect(get_addr(base_port + id));
        } else if (command == "ping") {
            size_t id;
            std::cin >> id;

            if (nodes.count(id) == 0) {
                std::cout << "Error: Not found." << std::endl;
                continue;
            }

            std::cout << "Ok: " << ping(socket, id) << std::endl;

        } else if (command == "exec") {
            size_t id;
            std::cin >> id;

            std::string subcommand;
            std::cin >> subcommand;

            if (nodes.count(id) == 0 || !ping(socket, id)) {
                continue;
            }

            socket.connect(get_addr(base_port + id));

            if (subcommand == "time") {
                socket.send(zmq::str_buffer("time"), zmq::send_flags::dontwait);
                zmq::message_t reply;
                socket.recv(reply, zmq::recv_flags::none);
                std::cout << "Ok:" << id << ": " << reply.to_string() << std::endl;

            } else if (subcommand == "start") {
                socket.send(zmq::str_buffer("start"), zmq::send_flags::dontwait);
                zmq::message_t reply;
                socket.recv(reply, zmq::recv_flags::none);
                std::cout << reply.to_string() << ":" << id << std::endl;
            } else if (subcommand == "stop") {
                socket.send(zmq::str_buffer("stop"), zmq::send_flags::dontwait);
                zmq::message_t reply;
                socket.recv(reply, zmq::recv_flags::none);
                std::cout << reply.to_string() << ":" << id << std::endl;
            }

            socket.disconnect(get_addr(base_port + id));
        } else if (command == "exit") {
            for (auto id : nodes) {
                socket.connect(get_addr(base_port + id));
                socket.send(zmq::str_buffer("exit"), zmq::send_flags::dontwait);

                zmq::message_t reply;
                socket.recv(reply, zmq::recv_flags::none);
                if (reply.to_string() == "OK") {
                    std::cout << "Node with id " + std::to_string(id) + " has been successfully killed." << std::endl;
                }

                socket.disconnect(get_addr(base_port + id));
            }

            socket.close();
            context.close();
            exit(0);
        }
        */
    }

}
