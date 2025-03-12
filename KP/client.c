#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 100
#define PIPE_NAME_SIZE 25
#define BOARD_SIZE 10

void print_board(char board[BOARD_SIZE][BOARD_SIZE]) {
    printf("  ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", i);
    }
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

void place_ships(char board[BOARD_SIZE][BOARD_SIZE]) {
    printf("Place your ships (e.g., 0 0 for (0,0)): \n");
    for (int i = 0; i < 5; i++) { // Размещаем 5 кораблей
        int x, y;
        printf("Ship %d: ", i + 1);
        scanf("%d %d", &x, &y);
        board[x][y] = 'S';
    }
}

void create_game(int server_fd, char *game_name) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "CREATE_GAME %s", game_name);
    write(server_fd, buffer, BUFFER_SIZE);
    read(server_fd, buffer, BUFFER_SIZE);
    printf("Server response: %s\n", buffer);
}

void join_game(int server_fd, char *game_name) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "JOIN_GAME %s", game_name);
    write(server_fd, buffer, BUFFER_SIZE);
    read(server_fd, buffer, BUFFER_SIZE);
    if (strcmp(buffer, "GAME_NOT_FOUND") == 0) {
        printf("Game not found. Please try again.\n");
        return;
    }
    printf("Server response: %s\n", buffer);
}

void make_move(int server_fd, char *game_name, char board[BOARD_SIZE][BOARD_SIZE]) {
    while (1) {
        int x, y;
        printf("Your move (x y): ");
        scanf("%d %d", &x, &y);
        char buffer[BUFFER_SIZE];
        sprintf(buffer, "MOVE %s %d %d", game_name, x, y);
        write(server_fd, buffer, BUFFER_SIZE);
        read(server_fd, buffer, BUFFER_SIZE);
        if (strcmp(buffer, "HIT") == 0) {
            printf("Hit!\n");
            board[x][y] = 'X';
        } else if (strcmp(buffer, "MISS") == 0) {
            printf("Miss!\n");
            board[x][y] = 'M';
        } else if (strcmp(buffer, "WIN") == 0) {
            printf("You win!\n");
            break;
        } else if (strcmp(buffer, "LOSE") == 0) {
            printf("You lose!\n");
            break;
        } else if (strcmp(buffer, "GAME_OVER") == 0) {
            printf("Game over!\n");
            break;
        }
        print_board(board);
    }
}

int main() {
    char login[20];
    printf("Enter your login: ");
    scanf("%s", login);

    char client_pipe[PIPE_NAME_SIZE];
    snprintf(client_pipe, PIPE_NAME_SIZE, "%s_pipe", login);
    mkfifo(client_pipe, 0666);

    int server_fd = open("server_pipe", O_WRONLY);
    write(server_fd, client_pipe, PIPE_NAME_SIZE);

    int choice;
    printf("1. Create game\n2. Join game\nChoice: ");
    scanf("%d", &choice);

    char game_name[20];
    if (choice == 1) {
        printf("Enter game name: ");
        scanf("%s", game_name);
        create_game(server_fd, game_name);
    } else if (choice == 2) {
        printf("Enter game name to join: ");
        scanf("%s", game_name);
        join_game(server_fd, game_name);
    }

    char board[BOARD_SIZE][BOARD_SIZE];
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '.';
        }
    }

    place_ships(board);
    print_board(board);

    make_move(server_fd, game_name, board);

    close(server_fd);
    unlink(client_pipe);
    return 0;
}