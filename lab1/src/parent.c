#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 512
//Создаёт макрос, который устанавливает максимальную длину 
// строки (512 символов).
// Это помогает управлять памятью и избежать переполнения буфера.

int main() {
	int pipe1[2], pipe2[2];
    char filepath[MAX_INPUT];
    char str_[MAX_INPUT];
    char error[MAX_INPUT];
    // Объявление переменных и каналов. pipe1 — передача 
    // данных родитель → дочерний.
    // pipe2 — возврат ошибок дочерний → родитель.
    // Данные и ошибки хранятся в буферах (учётная информация о процессе).

	printf("File name: ");
	fgets(filepath, MAX_INPUT, stdin);
	filepath[strlen(filepath) - 1] = 0;
    // Запрашивает имя файла, в который будут записываться корректные строки.
    // Использует адресное пространство для хранения введённого имени файла.

	int file = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file == -1) {
		perror("file open");
		exit(1);
	}
    // Открывает файл на запись. Если файла нет, он создаётся 
    // (O_CREAT), если файл существует — очищается (O_TRUNC).
    // В случае ошибки вызывается perror() — выводит сообщение о проблеме.

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1){
   		perror("pipe");
        exit(1);
    }
    // pipe() создаёт два канала.
    // Каждый канал содержит два файловых дескриптора:
    // pipe1[0] — для чтения (родитель читает из него).
    // pipe1[1] — для записи (родитель пишет в него).

    switch (fork()){
    // fork() — системный вызов, который создаёт дочерний процесс.
    // Родительский процесс получает PID дочернего процесса.
    // Дочерний процесс получает 0.
    	case -1:
        	perror("fork");
            exit(1);
        case 0:
        	dup2(pipe1[0], STDIN_FILENO); //чтение данных из канала
            dup2(pipe2[1], STDERR_FILENO); //запись ошибок в канал
    		dup2(file, STDOUT_FILENO); // запись корректных данных в файл

            // dup2() перенаправляет стандартные потоки:
            // TDIN → pipe1[0] (чтение данных из канала).
            // STDERR → pipe2[1] (запись ошибок в канал).
            // STDOUT → file (запись корректных данных в файл).
    		close(file);
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            execlp("./child", "child", NULL);
            perror("execlp");
            exit(1);
            // execlp() заменяет текущий процесс (child.cpp).
            // Если замена не удалась, программа выводит сообщение об ошибке 
            //и завершает процесс.
        default:
        	close(pipe1[0]);
            close(pipe2[1]);

            while (1){
            	printf("String or 'e' for exit: ");
                fgets(str_, MAX_INPUT, stdin);
                if (strncmp(str_, "e", 1) == 0) break;
                write(pipe1[1], str_, strlen(str_));
            }
            // Родитель принимает строки от пользователя и отправляет 
            // их дочернему процессу через pipe1.
            // Если введено "e", программа завершает ввод.
            close(pipe1[1]);

            while(read(pipe2[0], error, MAX_INPUT) > 0) {
              	printf("%s \n", error);
            }
            // read() читает сообщения об ошибках из pipe2 и выводит их на экран.
            close(pipe2[0]);
            wait(NULL);
    }
    // Родитель вызывает wait(), чтобы дождаться завершения дочернего процесса.
    // Это предотвращает появление "зомби"-процессов.
    return 0;
}