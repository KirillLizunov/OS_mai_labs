#include <iostream>
#include <string>
#include <vector>
#include <zmq.hpp>
#include <chrono>
#include <unistd.h>

struct Node {
    int id;
    std::string availability;
};

class Timer {
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    bool running = false;

public:
    void start() {
        start_time = std::chrono::steady_clock::now();
        running = true;
    }

    void stop() {
        if (running) {
            end_time = std::chrono::steady_clock::now();
            running = false;
        }
    }

    int time() {
        if (running) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
        } else {
            return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        }
    }
};

int main() {
    std::vector<Node> nodes;
    zmq::context_t context(1);

    // pub для команд
    zmq::socket_t command_pub(context, zmq::socket_type::pub);
    command_pub.bind("tcp://localhost:5555");

    // sub для heartbeat
    zmq::socket_t heartbeat_sub(context, zmq::socket_type::sub);
    heartbeat_sub.connect("tcp://localhost:5556");
    heartbeat_sub.set(zmq::sockopt::subscribe, "");

    while (true) {
        std::string command;
        std::cin >> command;

        if (command == "create") {
            int node_id;
            std::cin >> node_id;
            auto it = std::find_if(nodes.begin(), nodes.end(), [node_id](const Node& node) { return node.id == node_id; });
            if (it != nodes.end()) {
                std::cout << "Такой id уже существует" << std::endl;
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    execl("./worker", "./worker", std::to_string(node_id).c_str(), nullptr);
                }
                nodes.push_back({node_id, "available"});
                std::cout << "Ok: Node " << node_id << " created" << std::endl;
            }
        } else if (command == "exec") {
            int node_id;
            std::string subcommand;
            std::cin >> node_id >> subcommand;
            auto it = std::find_if(nodes.begin(), nodes.end(), [node_id](const Node& node) { return node.id == node_id; });
            if (it == nodes.end()) {
                std::cout << "Такого id не существует" << std::endl;
            } else {
                std::string full_command = std::to_string(node_id) + " " + subcommand;
                zmq::message_t message(full_command.size());
                memcpy(message.data(), full_command.data(), full_command.size());
                command_pub.send(message, zmq::send_flags::none);
            }
        } else if (command == "heartbit") {
            int period;
            std::cin >> period;
            if (period <= 0) {
                std::cout << "Error: uncorrect input\n";
                continue;
            }

            std::string full_heartbit = std::to_string(-100) + " " + std::to_string(period) + " " + command;
            zmq::message_t message(full_heartbit.size());
            memcpy(message.data(), full_heartbit.data(), full_heartbit.size());
            command_pub.send(message, zmq::send_flags::none);

            Timer tm;
            tm.start();
            sleep(0.1);
            zmq::pollitem_t items[] = {
                { heartbeat_sub, 0, ZMQ_POLLIN, 0 }
            };

            while (true) {
                if (tm.time() >= 4 * period - 150) {
                    int count = 0;
                    for (auto& node : nodes) {
                        if (node.availability == "available") {
                            count++;
                            std::cout << "Node " << node.id << " is available" << std::endl;
                        }
                    }
                    if (count == 0) {
                        std::cout << "No nodes are available" << std::endl;
                    }
                    tm.stop();
                    break;
                }

                zmq::poll(&items[0], 1, period);

                if (items[0].revents & ZMQ_POLLIN) {
                    zmq::message_t reply;
                    heartbeat_sub.recv(reply, zmq::recv_flags::none);
                    std::string id_str = reply.to_string();
                    int id = std::stoi(id_str);
                    for (auto& node : nodes) {
                        if (node.id == id) {
                            node.availability = "available";
                        }
                    }
                }
            }
        } else if (command == "exit") {
            return 0;
        }
    }
    return 0;
}