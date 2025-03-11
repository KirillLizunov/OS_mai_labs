# Используем Ubuntu как базовый образ
FROM ubuntu:latest

# Устанавливаем необходимые утилиты
RUN apt update && apt install -y build-essential strace

# Создаём структуру папок
WORKDIR /app/lab1

# Копируем исходный код
COPY src /app/lab1/src

# Компилируем программы
WORKDIR /app/lab1/src
RUN gcc parent.c -o parent && gcc child.c -o child

# Разрешаем выполнение
RUN chmod +x parent child

# Указываем точку входа с использованием strace
CMD ["strace", "-o", "/app/lab1/src/strace.log", "./parent"]
