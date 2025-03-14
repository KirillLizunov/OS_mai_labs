#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 2
#define PIPE_NAME_SIZE 25
#define BUFFER_SIZE 100
#define BOARD_SIZE 10
#define GAME_FILE "game.txt"

typedef struct {
    char login[20];
    int fd;
    char board[BOARD_SIZE][BOARD_SIZE]; // Игровое поле
    int ships_left; // Количество оставшихся кораблей
} Client;

typedef struct {
    char name[20];
    Client players[2];
    int turn; // Индекс игрока, который сейчас ходит
    int game_over; // Флаг окончания игры
} Game;

Game games[10];
int game_count = 0;

void init_board(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '.'; // Инициализация пустого поля
        }
    }
}

void write_game_name(const char *game_name) {
    FILE *file = fopen(GAME_FILE, "w");
    if (file) {
        fprintf(file, "%s", game_name);
        fclose(file);
        printf("Game name '%s' written to file.\n", game_name);
    } else {
        perror("Failed to open game file");
    }
}

int check_game_name(const char *game_name) {
    FILE *file = fopen(GAME_FILE, "r");
    if (!file) {
        perror("Failed to open game file");
        return 0; // Файл не существует
    }
    char existing_game_name[20];
    if (fscanf(file, "%s", existing_game_name) == 1) {
        fclose(file);
        printf("Checking game name '%s' against '%s'.\n", game_name, existing_game_name);
        return strcmp(existing_game_name, game_name) == 0;
    }
    fclose(file);
    return 0;
}

void handle_client(int client_fd, char *client_pipe) {
    char buffer[BUFFER_SIZE];
    read(client_fd, buffer, BUFFER_SIZE);

    if (strncmp(buffer, "CREATE_GAME", 11) == 0) {
        // Создание игры
        char game_name[20];
        strcpy(game_name, buffer + 12);
        write_game_name(game_name);
        strcpy(games[game_count].name, game_name);
        games[game_count].players[0].fd = client_fd;
        strcpy(games[game_count].players[0].login, client_pipe);
        init_board(games[game_count].players[0].board);
        games[game_count].players[0].ships_left = 5; // Устанавливаем 5 кораблей
        games[game_count].turn = 0;
        games[game_count].game_over = 0;
        write(client_fd, "GAME_CREATED", 12);
        game_count++;
    } else if (strncmp(buffer, "JOIN_GAME", 9) == 0) {
        // Присоединение к игре
        char game_name[20];
        strcpy(game_name, buffer + 10);
        if (check_game_name(game_name)) {
            for (int i = 0; i < game_count; i++) {
                if (strcmp(games[i].name, game_name) == 0) {
                    games[i].players[1].fd = client_fd;
                    strcpy(games[i].players[1].login, client_pipe);
                    init_board(games[i].players[1].board);
                    games[i].players[1].ships_left = 5; // Устанавливаем 5 кораблей
                    write(client_fd, "GAME_JOINED", 11);
                    break;
                }
            }
        } else {
            write(client_fd, "GAME_NOT_FOUND", 14);
        }
    } else if (strncmp(buffer, "MOVE", 4) == 0) {
        // Обработка хода
        char game_name[20];
        int x, y;
        sscanf(buffer, "MOVE %s %d %d", game_name, &x, &y);
        
        for (int i = 0; i < game_count; i++) {
            if (strcmp(games[i].name, game_name) == 0) {
                if (games[i].game_over) {
                    write(games[i].players[games[i].turn].fd, "GAME_OVER", 9);
                    break;
                }

                int target_player = (games[i].turn + 1) % 2;
                char result = games[i].players[target_player].board[x][y];

                if (result == 'S') {
                    games[i].players[target_player].board[x][y] = 'X'; // Попадание
                    games[i].players[target_player].ships_left--;

                    if (games[i].players[target_player].ships_left == 0) {
                        write(games[i].players[games[i].turn].fd, "WIN", 3);
                        write(games[i].players[target_player].fd, "LOSE", 4);
                        games[i].game_over = 1;
                    } else {
                        printf("Sending response: HIT\n");  // <-- Добавляем перед write()
                        write(games[i].players[games[i].turn].fd, "HIT", 3);
                    }
                } else {
                    games[i].players[target_player].board[x][y] = 'M'; // Промах
                    printf("Sending response: MISS\n");  // <-- Добавляем перед write()
                    write(games[i].players[games[i].turn].fd, "MISS", 4);
                }
                games[i].turn = target_player; // Передача хода другому игроку
                break;
            }
        }
    }
}

void cleanup() {
    unlink("server_pipe");
    unlink(GAME_FILE);
}

int main() {
    atexit(cleanup);
    signal(SIGINT, cleanup);

    mkfifo("server_pipe", 0666);
    int server_fd = open("server_pipe", O_RDONLY);

    while (1) {
        char client_pipe[PIPE_NAME_SIZE];
        read(server_fd, client_pipe, PIPE_NAME_SIZE);
        int client_fd = open(client_pipe, O_WRONLY);
        handle_client(client_fd, client_pipe);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}