#include <stdio.h> 
#include <dlfcn.h> 
#include <stdlib.h>
 
typedef int (*PrimeCountFunc)(int, int); 
typedef char* (*TranslationFunc)(long);

int main() {
    void* handle = dlopen("./libnaive.so", RTLD_LAZY); 
    if (!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror()); 
        return 1;
    }

    PrimeCountFunc PrimeCount = (PrimeCountFunc)dlsym(handle, "PrimeCount"); 
    TranslationFunc translation = (TranslationFunc)dlsym(handle, "translation");

    if (!PrimeCount || !translation) {
        fprintf(stderr, "Error loading symbols: %s\n", dlerror()); 
        dlclose(handle);
        return 1;
    }

    int is_naive = 1; 
    int command;

    while (1) {
        printf("Enter command (0 - switch library, 1 - PrimeCount, 2 - translation): "); 
        scanf("%d", &command);

        if (command == 0) { 
            dlclose(handle);
            
            if (is_naive) {
                handle = dlopen("./liboptimized.so", RTLD_LAZY);
                if (!handle) {
                    fprintf(stderr, "Error switching to liboptimized: %s\n", dlerror()); 
                    return 1;
                }
                printf("\nSwitched to optimized library\n\n");
            } else {
                handle = dlopen("./libnaive.so", RTLD_LAZY); 
                if (!handle) {
                    fprintf(stderr, "Error switching to libnaive: %s\n", dlerror()); 
                    return 1;
                }
                printf("\nSwitched to naive library\n\n");
            }
            
            is_naive = !is_naive;

            PrimeCount = (PrimeCountFunc)dlsym(handle, "PrimeCount"); 
            translation = (TranslationFunc)dlsym(handle, "translation");

            if (!PrimeCount || !translation) {
                fprintf(stderr, "Error loading symbols: %s\n", dlerror()); 
                dlclose(handle);
                return 1;
            }
        } 
        else if (command == 1) { 
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
            }
        }
    }

    dlclose(handle); 
    return 0;
}
