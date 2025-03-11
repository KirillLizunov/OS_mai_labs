# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -fPIC -Wall -Wextra

# Флаги для создания динамических библиотек
LDFLAGS = -shared

# Флаги для линковки с библиотекой dl (для динамической загрузки)
DLFLAGS = -ldl

# Имена библиотек
LIB_NAIVE = libnaive.so
LIB_OPTIMIZED = liboptimized.so

# Имена тестовых программ
PROGRAM_1 = program_1
PROGRAM_2 = program_2

# Все цели по умолчанию
all: $(LIB_NAIVE) $(LIB_OPTIMIZED) $(PROGRAM_1) $(PROGRAM_2)

# Сборка динамической библиотеки naive
$(LIB_NAIVE): libnaive.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm

# Сборка динамической библиотеки optimized
$(LIB_OPTIMIZED): liboptimized.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm

# Компиляция библиотек
libnaive.o: libnaive.c
	$(CC) $(CFLAGS) -c $< -o $@

liboptimized.o: liboptimized.c
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка тестовой программы №1 (статическое использование библиотек)
$(PROGRAM_1): program_1.c
	$(CC) -o $@ $< -L. -lnaive -lm -Wl,-rpath,.

# Сборка тестовой программы №2 (динамическая загрузка библиотек)
$(PROGRAM_2): program_2.c
	$(CC) -o $@ $< $(DLFLAGS)

# Очистка сгенерированных файлов
clean:
	rm -f *.o *.so $(PROGRAM_1) $(PROGRAM_2)

# Указание целей, которые не являются файлами
.PHONY: all clean
