#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 8  // Max parallel threads to prevent excessive CPU use

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for thread safety
int solved = 0;  // Global flag to stop all threads when a solution is found

typedef struct {
    int sudoku[9][9];
} SudokuGrid;

void print_sudoku(int sudoku[9][9]) {
    for (int j = 0; j < 9; j++) {
        for (int i = 0; i < 9; i++) {
            printf("%d ", sudoku[j][i]);
        }
        printf("\n");
    }
}

int is_valid(int sudoku[9][9], int num, int row, int col) {
    for (int i = 0; i < 9; i++) {
        if (sudoku[row][i] == num || sudoku[i][col] == num) return 0;
    }

    int row_start = row - row % 3;
    int col_start = col - col % 3;
    for (int i = row_start; i < row_start + 3; i++) {
        for (int j = col_start; j < col_start + 3; j++) {
            if (sudoku[i][j] == num) return 0;
        }
    }
    return 1;
}

int find_empty_cell(int sudoku[9][9], int *row, int *col) {
    for (*row = 0; *row < 9; (*row)++) {
        for (*col = 0; *col < 9; (*col)++) {
            if (sudoku[*row][*col] == 0) return 1;
        }
    }
    return 0;
}

void *solve_sudoku(void *arg) {
    SudokuGrid *grid = (SudokuGrid *)arg;
    
    if (solved) {
        free(grid);
        pthread_exit(NULL);
    }

    int row, col;
    if (!find_empty_cell(grid->sudoku, &row, &col)) {
        pthread_mutex_lock(&mutex);
        if (!solved) {
            solved = 1;
            printf("Solution found:\n");
            print_sudoku(grid->sudoku);
        }
        pthread_mutex_unlock(&mutex);
        free(grid);
        pthread_exit(NULL);
    }

    pthread_t threads[9];
    int thread_count = 0;

    for (int num = 1; num <= 9; num++) {
        if (is_valid(grid->sudoku, num, row, col)) {
            SudokuGrid *new_grid = malloc(sizeof(SudokuGrid));
            memcpy(new_grid->sudoku, grid->sudoku, 9 * 9 * sizeof(int));
            new_grid->sudoku[row][col] = num;

            if (thread_count < MAX_THREADS) {
                pthread_create(&threads[thread_count], NULL, solve_sudoku, new_grid);
                thread_count++;
            } else {
                solve_sudoku(new_grid);
            }
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    free(grid);
    pthread_exit(NULL);
}

void start_solver(int sudoku[9][9]) {
    SudokuGrid *grid = malloc(sizeof(SudokuGrid));
    memcpy(grid->sudoku, sudoku, 9 * 9 * sizeof(int));

    pthread_t thread;
    pthread_create(&thread, NULL, solve_sudoku, grid);
    pthread_join(thread, NULL);
}

int main() {
    int sudoku[9][9] = {
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

    printf("Input Sudoku:\n");
    print_sudoku(sudoku);

    clock_t start = clock();
    start_solver(sudoku);
    clock_t end = clock();

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Time taken: %.2f ms\n", time_taken);

    return 0;
}

