#include <stdbool.h> 
#include <stdlib.h> 
#include <string.h>

int PrimeCount(int A, int B) { 
    if (B < 2) return 0;  // Если верхний предел меньше 2, простых чисел нет

    // Выделяем память под массив для Решета Эратосфена
    bool* sieve = (bool*)malloc((B + 1) * sizeof(bool)); 
    if (!sieve) return 0;

    // Изначально считаем, что все числа - простые
    memset(sieve, true, (B + 1) * sizeof(bool));

    // 0 и 1 не являются простыми
    sieve[0] = sieve[1] = false;

    // Основной алгоритм
    for (int i = 2; i * i <= B; i++) { 
        if (sieve[i]) {  // Если i - простое
            for (int j = i * i; j <= B; j += i) { 
                sieve[j] = false;  // Все кратные i - составные
            }
        }
    }

    // Подсчитываем количество простых чисел в диапазоне [A, B]
    int count = 0;
    for (int i = A; i <= B; i++) { 
        if (sieve[i]) count++;
    }

    free(sieve);  // Освобождаем память
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
