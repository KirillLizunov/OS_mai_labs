FROM ubuntu:latest

# Устанавливаем strace
RUN apt update && apt install -y strace

WORKDIR /app
COPY main /app/main

# Делаем файл исполняемым
RUN chmod +x /app/main

# Запускаем strace на main
CMD ["strace", "-o", "/tmp/strace.log", "/app/main", "4"]
