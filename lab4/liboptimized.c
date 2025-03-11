#include <stdbool.h> 
#include <stdlib.h> 
#include <string.h>

int PrimeCount(int A, int B) { 
    if (B < 2) return 0;

    bool* sieve = (bool*)malloc((B + 1) * sizeof(bool)); 
    if (!sieve) return 0;

    memset(sieve, true, (B + 1) * sizeof(bool));

    sieve[0] = sieve[1] = false;
    for (int i = 2; i * i <= B; i++) { 
        if (sieve[i]) {
            for (int j = i * i; j <= B; j += i) { 
                sieve[j] = false;
            }
        }
    }

    int count = 0;
    for (int i = A; i <= B; i++) { 
        if (sieve[i]) count++;
    }

    free(sieve); 
    return count;
}
 
char* translation(long x) {
    char* result = (char*)malloc(65); 
    if (!result) return NULL;

    int index = 0; 
    do {
        result[index++] = (x % 3) + '0'; 
        x /= 3;
    } while (x > 0); 

    result[index] = '\0';

    for (int i = 0; i < index / 2; i++) { 
        char temp = result[i];
        result[i] = result[index - 1 - i]; 
        result[index - 1 - i] = temp;
    }

    return result;
}
