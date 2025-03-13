#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <dispatch/dispatch.h>

#define MAX_VARS 3000
#define TOLERANCE 1e-9

int var_count;
float matrix[MAX_VARS][MAX_VARS + 1];
float solutions[MAX_VARS];
int thread_limit;
dispatch_semaphore_t thread_semaphore;

typedef struct {
    int current_row;
    int lead_element;
} GaussThreadData;

void display_system() {
    printf("\nСистема уравнений:\n");
    for (int row = 1; row <= var_count; row++) {
        for (int col = 1; col <= var_count; col++) {
            printf("%.2f*x%d", matrix[row][col], col);
            if (col < var_count) printf(" + ");
        }
        printf(" = %.2f\n", matrix[row][var_count + 1]);    
    }
}

void create_random_matrix() {
    srand(5678);
    for (int row = 1; row <= var_count; row++) {
        for (int col = 1; col <= var_count; col++) {
            matrix[row][col] = (rand() % 20) - 10;
        }
        matrix[row][var_count + 1] = (rand() % 20) - 10;
    }
}

void input_matrix() {
    for (int row = 1; row <= var_count; row++) {
        for (int col = 1; col <= var_count + 1; col++) {
            printf("Введите коэффициент matrix[%d][%d]: ", row, col);
            while (scanf("%f", &matrix[row][col]) != 1) {
                fprintf(stderr, "Ошибка: введите корректное значение.\n");
                while (getchar() != '\n');
            }
        }
    }
}

void *gauss_step(void *arg) {
    GaussThreadData *data = (GaussThreadData *)arg;
    int i = data->current_row; // Текущая строка
    int pivot = data->lead_element; // Ведущий элемент (строка)

    if (fabs(matrix[pivot][pivot]) < TOLERANCE) {
        dispatch_semaphore_signal(thread_semaphore);
        pthread_exit(NULL);
    }

    float factor = matrix[i][pivot] / matrix[pivot][pivot]; // Вычисляем коэффициент
    for (int col = pivot; col <= var_count; col++) { // Обновляем элементы строки
        matrix[i][col] -= factor * matrix[pivot][col];
    }
    matrix[i][var_count + 1] -= factor * matrix[pivot][var_count + 1];

    dispatch_semaphore_signal(thread_semaphore);
    pthread_exit(NULL);
}

void back_substitution() {
    for (int row = var_count; row >= 1; row--) {
        float sum = matrix[row][var_count + 1];
        for (int col = row + 1; col <= var_count; col++) {
            sum -= matrix[row][col] * solutions[col];
        }
        solutions[row] = sum / matrix[row][row];
    }
}

void display_solutions() {
    printf("\nРешения системы:\n");
    for (int i = 1; i <= var_count; i++) {
        printf("x%d = %.6f\n", i, solutions[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <число потоков>\n", argv[0]);
        return EXIT_FAILURE;
    }

    thread_limit = atoi(argv[1]);
    if (thread_limit <= 0) {
        fprintf(stderr, "Ошибка: количество потоков должно быть положительным.\n");
        return EXIT_FAILURE;
    }

    thread_semaphore = dispatch_semaphore_create(thread_limit); //семафор ограничивает число одновременно работающих потоков. Позволяет не превышать thread_limit

    printf("Введите количество переменных (1-%d): ", MAX_VARS);
    while (scanf("%d", &var_count) != 1 || var_count <= 0 || var_count > MAX_VARS) {
        fprintf(stderr, "Ошибка: некорректное значение. Попробуйте снова.\n");
        while (getchar() != '\n');
    }

    printf("Выберите ввод: 1 - вручную, 2 - случайная генерация: ");
    int mode;
    while (scanf("%d", &mode) != 1 || (mode != 1 && mode != 2)) {
        fprintf(stderr, "Ошибка: некорректный выбор.\n");
        while (getchar() != '\n');
    }

    (mode == 1) ? input_matrix() : create_random_matrix();

    display_system();

    pthread_t threads[MAX_VARS];
    GaussThreadData thread_data[MAX_VARS];

    clock_t start = clock();

    //Прямой ход метода Гаусса с потоками. Программа распараллеливается
    for (int pivot = 1; pivot <= var_count; pivot++) { // Выбираем ведущий элемент
        for (int row = pivot + 1; row <= var_count; row++) { // Все строки ниже pivot
            thread_data[row - pivot - 1].current_row = row;
            thread_data[row - pivot - 1].lead_element = pivot;

            dispatch_semaphore_wait(thread_semaphore, DISPATCH_TIME_FOREVER);   // Ждём свободного потока (если лимит достигнут, задерживаем выполнение).

            if (pthread_create(&threads[row - pivot - 1], NULL, gauss_step, &thread_data[row - pivot - 1]) != 0) {
                fprintf(stderr, "Ошибка: не удалось создать поток.\n");
                return EXIT_FAILURE;
            }
        }

        for (int i = 0; i < var_count - pivot; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    back_substitution();    //Запускаем обратный ход, вычисляя значения переменных.
    display_solutions();

    clock_t end = clock();
    printf("\nВремя выполнения: %.6f секунд\n", (double)(end - start) / CLOCKS_PER_SEC);

    return EXIT_SUCCESS;
}
