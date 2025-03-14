#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <map>
#include <pthread.h>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include "ClientSocket.hpp"

const size_t MAX_LEN_LOGIN = 11;
const size_t MAX_LEN_FILE_NAME = MAX_LEN_LOGIN + 5;

std::map<std::string, ClientSocket> sockets;
pthread_mutex_t sockets_lock = PTHREAD_MUTEX_INITIALIZER;

void* requests_thread(void* args) {
    auto socket{static_cast<ClientSocket*>(args)};
    ClientSocket* other_socket = nullptr;
    
    while (true) {
        std::string ans = socket->receive(sizeof(char));
        if (ans == "n") {
            socket->set_search_status(true);
            return nullptr;
        } else if (ans == "q") {
            pthread_mutex_lock(&sockets_lock);
            sockets.erase(socket->get_login());
            pthread_mutex_unlock(&sockets_lock);
            return nullptr;
        } else {
            try {
                std::string login = socket->receive(sizeof(char) * MAX_LEN_LOGIN);
                pthread_mutex_lock(&sockets_lock);
                other_socket = &sockets.at(login);
                pthread_mutex_unlock(&sockets_lock);
                if (other_socket->is_searching()) {
                    other_socket->send(socket->get_login());
                    ans = other_socket->receive(sizeof(char));
                    if (ans == "y") {
                        socket->send("y");
                        break;
                    } else if (ans == "q") {
                        pthread_mutex_lock(&sockets_lock);
                        sockets.erase(other_socket->get_login());
                        pthread_mutex_unlock(&sockets_lock);
                    }
                }
                socket->send("n");
            }
            catch (...) {
                socket->send("n");
                pthread_mutex_unlock(&sockets_lock);
            }
        }
    }

    socket->set_search_status(false);
    other_socket->set_search_status(false);
    socket->receive(sizeof(char) * 2);
    other_socket->receive(sizeof(char) * 2);
    socket->send("1");
    other_socket->send("2");
    
    bool end = false;
    while (!end) {
        while (true) {
            std::string cell = socket->receive(sizeof(char) * 2);
            other_socket->send(cell);
            std::string res = other_socket->receive(sizeof(char));
            socket->send(res);
            if (res == "t") {
                std::string end_msg = socket->receive(sizeof(char));
                other_socket->send(end_msg);
                if (end_msg == "t") {
                    other_socket->send(socket->get_login());
                    end = true;
                    break;
                }
            } else {
                break;
            }
        }

        if (end) break;

        while (true) {
            std::string cell = other_socket->receive(sizeof(char) * 2);
            socket->send(cell);
            std::string res = socket->receive(sizeof(char));
            other_socket->send(res);
            if (res == "t") {
                std::string end_msg = other_socket->receive(sizeof(char));
                socket->send(end_msg);
                if (end_msg == "t") {
                    socket->send(other_socket->get_login());
                    end = true;
                    break;
                }
            } else {
                break;
            }
        }
    }

    pthread_mutex_lock(&sockets_lock);
    sockets.erase(socket->get_login());
    sockets.erase(other_socket->get_login());
    pthread_mutex_unlock(&sockets_lock);

    return nullptr;
}

int main() {
    const std::string path = "./tmp";
    
    int kq = kqueue();
    if (kq == -1) {
        throw std::runtime_error("Couldn't create kqueue");
    }

    int dir_fd = open(path.c_str(), O_RDONLY);
    if (dir_fd == -1) {
        throw std::runtime_error("Couldn't open directory: " + path);
    }

    struct kevent change;
    EV_SET(&change, dir_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, nullptr);

    auto last_update = std::chrono::system_clock::now();

    while (!sockets.empty() || std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_update) < std::chrono::minutes{3}) {
        struct kevent event;
        int nev = kevent(kq, &change, 1, &event, 1, nullptr);
        
        if (nev > 0) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                std::string file_name = entry.path().filename();
                if (file_name.size() >= 4 && file_name.substr(file_name.size() - 4) == "_rep") {
                    last_update = std::chrono::system_clock::now();
                    pthread_t thread;
                    std::string login{file_name.substr(0, file_name.size() - 4)};
                    pthread_mutex_lock(&sockets_lock);
                    sockets.try_emplace(login, login);
                    pthread_create(&thread, nullptr, requests_thread, (void*) &sockets.at(login));
                    pthread_mutex_unlock(&sockets_lock);
                    pthread_detach(thread);
                }
            }
        }
    }

    close(dir_fd);
    close(kq);
    return 0;
}
