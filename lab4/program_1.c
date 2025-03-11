#include <stdio.h> 
#include "libnaive.c"

int main() {
    int command; 
    while (1) {
        printf("Enter command (0 - exit, 1 - PrimeCount, 2 - translation): "); 
        scanf("%d", &command);

        if (command == 0) {
            printf("\nGoodbye!\nHave a nice day!\n"); 
            break;
        } 

        if (command == 1) { 
            int A, B;
            printf(" Enter A and B: "); 
            scanf("%d %d", &A, &B);
            printf(" Prime count: %d\n", PrimeCount(A, B));
        } 
        else if (command == 2) { 
            long x;
            printf(" Enter number: "); 
            scanf("%ld", &x);
            char* result = translation(x); 
            if (result) {
                printf(" Result: %s\n", result); 
                free(result);
            } else {
                printf("Error in translation function.\n");
            }
        } 
        else {
            printf("Invalid command\n");
        }
    }

    return 0;
}
