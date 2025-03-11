#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_INPUT 512

int main() {
    char str_[MAX_INPUT]; // Буфер для хранения входной строки

    while (fgets(str_, MAX_INPUT, stdin) != NULL){
        if (isupper(str_[0])){ // Если строка корректна, она передаётся в STDOUT (перенаправлен в файл).
            printf("%s", str_);
        } else {
            fprintf(stderr, "error %s", str_); // в STDERR (связанный с pipe2).
        }
    }
    return 0;
}