#include <stdbool.h> 
#include <stdlib.h>

int PrimeCount(int A, int B) {  //поиск простых чисел наивным методом   
    int count = 0;

    for (int i = A; i <= B; i++) { 
        bool is_prime = true;
        if (i < 2) continue;
        
        for (int j = 2; j * j <= i; j++) { 
            if (i % j == 0) {
                is_prime = false; 
                break;
            }
        }
        
        if (is_prime) count++;
    }
    
    return count;
}

char* translation(long x) {     //перевод числа в двоичную систему
    char* result = (char*)malloc(65);   // Выделение памяти под строку (до 64 бит + '\0')
    if (!result) return NULL;

    int index = 0; 
    do {
        result[index++] = (x % 2) + '0'; // Получаем младший бит и записываем как символ
        x /= 2;
    } while (x > 0); 
    
    result[index] = '\0';

    for (int i = 0; i < index / 2; i++) {   // Переворачиваем строку
        char temp = result[i];
        result[i] = result[index - 1 - i]; 
        result[index - 1 - i] = temp;
    }
    
    return result;
}
