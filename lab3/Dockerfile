FROM debian:latest

WORKDIR /app

# Устанавливаем strace и необходимые пакеты
RUN apt-get update && apt-get install -y strace cmake g++ make

# Копируем весь проект
COPY . /app

# Собираем проект
RUN cmake -B build . && cmake --build build

# Создаём папку для логов и пустой strace.log
RUN mkdir -p /app/logs && touch /app/strace.log

# Запускаем strace и логируем
# CMD ["strace", "-o", "/app/logs/strace.log", "/app/build/main"]
CMD ["sh", "-c", "strace -o /app/strace.log /app/build/main && sleep 5"]

