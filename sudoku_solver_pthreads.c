#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>  // Include for timing

#define MAX_THREADS 8  // Limit on threads to prevent excessive parallelism

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to synchronize access

typedef struct {
    int sudoku[9][9];
    int depth;
    int row;
    int col;
    int num;
    int *solved;
} ThreadArgs;

void print_sudoku(int sudoku[9][9]) {
    printf("The Sudoku contains:\n");
    for (int j = 0; j < 9; j++) {
        for (int i = 0; i < 9; i++) {
            printf("%d  ", sudoku[j][i]);
        }
        printf("\n");
    }
}

int check_square(int sudoku[9][9], int num, int row, int col) {
    int row_start = row - row % 3;
    int col_start = col - col % 3;
    for (int i = row_start; i < row_start + 3; i++) {
        for (int j = col_start; j < col_start + 3; j++) {
            if (sudoku[i][j] == num) {
                return 0;
            }
        }
    }
    return 1;
}

int check_sudoku(int sudoku[9][9], int num, int row, int col) {
    for (int i = 0; i < 9; i++) {
        if (num == sudoku[i][col] || num == sudoku[row][i]) {
            return 0;
        }
    }
    return check_square(sudoku, num, row, col);
}

int find_unassigned(int sudoku[9][9], int *row, int *col) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (sudoku[i][j] == 0) {
                *row = i;
                *col = j;
                return 1;
            }
        }
    }
    return 0;
}

void *sudoku_solver_thread(void *args) {
    ThreadArgs *targs = (ThreadArgs *)args;
    
    if (*(targs->solved)) {
        free(targs);
        pthread_exit(NULL);
    }

    int sudoku[9][9];
    memcpy(sudoku, targs->sudoku, 9 * 9 * sizeof(int));

    int row = targs->row;
    int col = targs->col;
    sudoku[row][col] = targs->num;

    int new_row, new_col;
    if (!find_unassigned(sudoku, &new_row, &new_col)) {
        pthread_mutex_lock(&mutex);
        *(targs->solved) = 1;
        memcpy(targs->sudoku, sudoku, 9 * 9 * sizeof(int));
        pthread_mutex_unlock(&mutex);
        free(targs);
        pthread_exit(NULL);
    }

    pthread_t threads[9];
    int thread_count = 0;
    ThreadArgs *new_targs;
    
    for (int num = 1; num <= 9; num++) {
        if (check_sudoku(sudoku, num, new_row, new_col)) {
            new_targs = (ThreadArgs *)malloc(sizeof(ThreadArgs));
            memcpy(new_targs->sudoku, sudoku, 9 * 9 * sizeof(int));
            new_targs->depth = targs->depth + 1;
            new_targs->row = new_row;
            new_targs->col = new_col;
            new_targs->num = num;
            new_targs->solved = targs->solved;

            if (targs->depth < MAX_THREADS) {
                pthread_create(&threads[thread_count], NULL, sudoku_solver_thread, new_targs);
                thread_count++;
            } else {
                sudoku_solver_thread(new_targs);
            }
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    free(targs);
    pthread_exit(NULL);
}

void solve_sudoku(int sudoku[9][9]) {
    int row, col;
    if (!find_unassigned(sudoku, &row, &col)) {
        return;
    }

    int solved = 0;
    pthread_t threads[9];
    int thread_count = 0;
    ThreadArgs *targs;

    for (int num = 1; num <= 9; num++) {
        if (check_sudoku(sudoku, num, row, col)) {
            targs = (ThreadArgs *)malloc(sizeof(ThreadArgs));
            memcpy(targs->sudoku, sudoku, 9 * 9 * sizeof(int));
            targs->depth = 0;
            targs->row = row;
            targs->col = col;
            targs->num = num;
            targs->solved = &solved;

            pthread_create(&threads[thread_count], NULL, sudoku_solver_thread, targs);
            thread_count++;
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

#ifndef __testing
int main() {
    int Sudoku[9][9] = {
        {1, 0, 6, 0, 0, 2, 3, 0, 0},
        {0, 5, 0, 0, 0, 6, 0, 9, 1},
        {0, 0, 9, 5, 0, 1, 4, 6, 2},
        {0, 3, 7, 9, 0, 5, 0, 0, 0},
        {5, 8, 1, 0, 2, 7, 9, 0, 0},
        {0, 0, 0, 4, 0, 8, 1, 5, 7},
        {0, 0, 0, 2, 6, 0, 5, 4, 0},
        {0, 0, 4, 1, 5, 0, 6, 0, 9},
        {9, 0, 0, 8, 7, 4, 2, 1, 0}
    };

    printf("Input puzzle is:\n");
    print_sudoku(Sudoku);

    // Start timer
    clock_t start = clock();

    solve_sudoku(Sudoku);

    // Stop timer
    clock_t end = clock();

    printf("Solution is:\n");
    print_sudoku(Sudoku);

    // Calculate and print elapsed time in milliseconds
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Time taken to solve: %.2f ms\n", time_taken);

    return 0;
}
#endif
