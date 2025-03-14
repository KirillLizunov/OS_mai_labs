#include <iostream>
#include <string>
#include <zmq.hpp>
#include <chrono>
#include <thread>
#include "Timer.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: Missing node ID\n";
        return 1;
    }

    int id = std::stoi(argv[1]);
    zmq::context_t context(1);

    // SUB socket для получения команд
    zmq::socket_t subscriber(context, zmq::socket_type::sub);
    subscriber.connect("tcp://localhost:5555");
    subscriber.set(zmq::sockopt::subscribe, std::to_string(id));
    subscriber.set(zmq::sockopt::subscribe, "-100");

    // PUB socket для отправки heartbeat
    zmq::socket_t publisher(context, zmq::socket_type::pub);
    publisher.connect("tcp://localhost:5556");

    Timer timer;
    

    while (true) {
        zmq::message_t message;
        // Получаем сообщение, если оно доступно
        if (subscriber.recv(message, zmq::recv_flags::none)) {
            std::string command = message.to_string();

            if (command.find("start") != std::string::npos) {
                timer.start();
                std::cout << "Ok: " << id << std::endl;
            } else if (command.find("stop") != std::string::npos) {
                timer.stop();
                std::cout << "Ok: " << id << std::endl;
            } else if (command.find("time") != std::string::npos) {
                int t = timer.time();
                std::cout << "Ok: " << id << " " << t << std::endl;
            } else if (command.find("heartbit") != std::string::npos) {
                size_t first_space = command.find(' ');
                // Находим второй пробел
                size_t second_space = command.find(' ', first_space + 1);
                // Извлекаем период, который находится между первым и вторым пробелом
                std::string period_str = command.substr(first_space + 1, second_space - first_space - 1);
                int period = std::stoi(period_str);

                Timer hb_timer;
                hb_timer.start();

                Timer delay;
                delay.start();

                while(true){
                    
                    if(delay.time()>=period*4+200){
                        hb_timer.stop();
                        delay.stop();
                        break;
                    }
                    sleep(0.1);
                    if(hb_timer.time()>=period){
                        
                        zmq::message_t heartbeat_msg(std::to_string(id).size());
                        memcpy(heartbeat_msg.data(), std::to_string(id).c_str(), std::to_string(id).size());
                        publisher.send(heartbeat_msg, zmq::send_flags::none);
                        hb_timer.stop();
                        hb_timer.start();
                    }
                }
            }
        }

    }

    return 0;
}