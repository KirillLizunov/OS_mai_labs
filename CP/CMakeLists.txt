# cmake_minimum_required(VERSION 3.10)
# project(BattleShip)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

# add_executable(client client.cpp)
# add_executable(server server.cpp)

# target_link_libraries(server Threads::Threads zmq)
cmake_minimum_required(VERSION 3.10)
project(KP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем и добавляем потоки
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Добавляем путь к заголовочным файлам zmq
include_directories("/opt/homebrew/include")

# Добавляем путь к библиотекам zmq
link_directories("/opt/homebrew/lib")

# Компиляция клиентского и серверного приложения
add_executable(client client.cpp)
add_executable(server server.cpp)

# Линкуем сервер с нужными библиотеками
target_link_libraries(server Threads::Threads zmq)



