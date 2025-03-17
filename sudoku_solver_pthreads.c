#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>

#define N 36               // Fix grid size
#define SUBGRID 6          // sqrt(N)
#define PARALLEL_CUTOFF 2  // Only spawn threads for recursion levels < this cutoff
#define MAX_THREADS 8      // Maximum number of concurrent threads

// Global flag to indicate a solution has been found
int solved = 0;
pthread_mutex_t solved_mutex = PTHREAD_MUTEX_INITIALIZER;

// Global thread counter and its mutex
int active_threads = 0;
pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;


void print_time() {
    time_t timer;
    char buffer[26];
    struct tm *tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s\n", buffer);
}

void print_sudoku(int *sudoku, int grid_size) {
    printf("The Sudoku contains:\n");
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            printf("%2d ", sudoku[i * grid_size + j]);
        }
        printf("\n");
    }
}

int check_square(int *sudoku, int grid_size, int block_size, int num, int row, int col) {
    int row_start = row - row % block_size;
    int col_start = col - col % block_size;
    for (int i = row_start; i < row_start + block_size; i++) {
        for (int j = col_start; j < col_start + block_size; j++) {
            if (sudoku[i * grid_size + j] == num)
                return 0;
        }
    }
    return 1;
}

int check_sudoku(int *sudoku, int grid_size, int block_size, int num, int row, int col) {
    for (int i = 0; i < grid_size; i++) {
        if (sudoku[row * grid_size + i] == num || sudoku[i * grid_size + col] == num)
            return 0;
    }
    return check_square(sudoku, grid_size, block_size, num, row, col);
}

int find_unassigned(int *sudoku, int grid_size, int *row, int *col) {
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (sudoku[i * grid_size + j] == 0) {
                *row = i;
                *col = j;
                return 1;
            }
        }
    }
    return 0;
}


// Recursive Solver with Pthreads
typedef struct {
    int *sudoku;
    int grid_size;
    int block_size;
    int depth;
} solver_args_t;

// Forward declaration
int sudoku_solver_parallel_pthread(int *sudoku, int grid_size, int block_size, int depth);

// Thread wrapper function
void* solver_thread_func(void* arg) {
    solver_args_t* args = (solver_args_t*) arg;
    int result = sudoku_solver_parallel_pthread(args->sudoku, args->grid_size, args->block_size, args->depth);
    
    // Decrement the active thread count
    pthread_mutex_lock(&thread_count_mutex);
    active_threads--;
    pthread_mutex_unlock(&thread_count_mutex);
    
    free(args); // Free the allocated arguments wrapper.
    return (void*)(intptr_t) result;
}

int sudoku_solver_parallel_pthread(int *sudoku, int grid_size, int block_size, int depth) {
    // Check if a solution is already found
    pthread_mutex_lock(&solved_mutex);
    int local_solved = solved;
    pthread_mutex_unlock(&solved_mutex);
    if (local_solved)
        return 0;

    int row, col;
    if (!find_unassigned(sudoku, grid_size, &row, &col)) {
        // Puzzle solved.
        pthread_mutex_lock(&solved_mutex);
        if (!solved) {
            solved = 1;
            print_time();  
        }
        pthread_mutex_unlock(&solved_mutex);
        return 1;
    }

    int found_solution = 0;
    // Array to hold thread handles and sudoku copies from spawned threads.
    pthread_t threads[grid_size];
    int thread_count = 0;
    int *thread_sudokus[grid_size];

    for (int num = 1; num <= grid_size && !found_solution; num++) {
        if (check_sudoku(sudoku, grid_size, block_size, num, row, col)) {
            if (depth < PARALLEL_CUTOFF) {
                // Check if we can spawn a new thread.
                pthread_mutex_lock(&thread_count_mutex);
                if (active_threads < MAX_THREADS) {
                    active_threads++;
                    pthread_mutex_unlock(&thread_count_mutex);
                    
                    // Create a copy of the current sudoku.
                    int *sudoku_copy = malloc(grid_size * grid_size * sizeof(int));
                    if (!sudoku_copy) {
                        perror("Memory allocation failed");
                        exit(1);
                    }
                    memcpy(sudoku_copy, sudoku, grid_size * grid_size * sizeof(int));
                    sudoku_copy[row * grid_size + col] = num;
                    
                    // Allocate and set up arguments for the new thread.
                    solver_args_t *args = malloc(sizeof(solver_args_t));
                    if (!args) {
                        perror("Memory allocation failed");
                        exit(1);
                    }
                    args->sudoku = sudoku_copy;
                    args->grid_size = grid_size;
                    args->block_size = block_size;
                    args->depth = depth + 1;
                    
                    // Create the new thread
                    int rc = pthread_create(&threads[thread_count], NULL, solver_thread_func, (void*) args);
                    if (rc != 0) {
                        perror("pthread_create failed");
                        exit(1);
                    }
                    thread_sudokus[thread_count] = sudoku_copy;
                    thread_count++;
                } else {
                    pthread_mutex_unlock(&thread_count_mutex);
                    // If maximum threads are active, proceed serially
                    sudoku[row * grid_size + col] = num;
                    if (sudoku_solver_parallel_pthread(sudoku, grid_size, block_size, depth + 1))
                        return 1;
                    sudoku[row * grid_size + col] = 0; // Backtrack
                }
            } else {
                // Deeper recursion: continue serially
                sudoku[row * grid_size + col] = num;
                if (sudoku_solver_parallel_pthread(sudoku, grid_size, block_size, depth + 1))
                    return 1;
                sudoku[row * grid_size + col] = 0;
            }
        }
    }

    // Wait for all spawned threads
    for (int i = 0; i < thread_count; i++) {
        void* thread_result;
        pthread_join(threads[i], &thread_result);
        int result = (int)(intptr_t) thread_result;
        if (result) {
            // Copy the solved sudoku from the thread's copy
            memcpy(sudoku, thread_sudokus[i], grid_size * grid_size * sizeof(int));
            found_solution = 1;
        } else {
            free(thread_sudokus[i]);
        }
    }
    return found_solution;
}


int main() {
    // The sudoku puzzle is hard-coded here
    int static_grid[N][N] = {
        {  0,  0,  0, 16,  0, 20, 19, 33,  0, 35, 24, 12,  0,  0,  0,  4,  0, 27,  0,  0, 30,  0, 14, 34, 25, 36,  0, 10, 18,  0, 32, 15, 28,  5,  1,  8 },
        { 30,  0,  1,  8,  0,  0, 11, 36,  0, 17, 18, 20, 14,  5,  6, 15,  0, 34, 21,  7,  9,  0, 13,  0, 24,  0,  0, 12,  0, 23, 31, 16, 25, 27,  0, 10 },
        { 26,  7, 21,  4, 25, 27,  1,  3, 30,  0,  6, 34, 22, 36, 18, 10, 13, 29,  0,  0, 32,  0,  2,  8,  0, 17, 11, 16,  9,  0,  0, 33,  0,  0,  0, 12 },
        { 14,  0,  6, 34,  0,  0, 21, 10,  0, 27, 13,  0, 24,  0,  0, 12,  0, 33, 31, 17, 25, 16,  0, 20,  0,  0,  1,  0, 32,  0, 11, 36, 22,  4,  0, 29 },
        { 11, 36, 18, 10,  0, 29, 32,  0,  0,  5,  0,  8, 25, 17, 31, 16, 26,  0,  0,  0, 24, 35, 19,  0, 22,  0,  0,  0, 13, 27,  2,  0, 30, 23,  0, 34 },
        { 24,  0, 19,  5, 28,  0, 31,  0, 25,  4, 26,  0,  0, 23,  1,  0,  0,  0, 11,  0,  0, 27,  0, 10, 14, 34,  6,  8,  2, 33, 21, 17,  9,  0, 13, 20 },
        { 10, 31,  7,  0,  0, 26, 33, 30, 15,  0,  5, 32, 16,  0, 29, 22, 27, 18,  3, 14,  0,  6,  0,  0,  4, 21, 17,  0, 20, 13, 34, 24,  0, 19, 12, 28 },
        { 12, 30, 33,  2,  0,  0, 27,  9,  0, 31,  0, 26, 34,  0,  3, 24,  0, 28, 29, 13, 36, 22, 10, 11,  0,  1, 15,  0,  0,  0,  0, 18,  0,  0,  0, 21 },
        {  0,  0,  0, 22,  4, 18,  0, 14, 34,  0,  0,  2,  0, 21, 10,  9,  0, 13,  0,  0, 12,  0, 33, 28,  7, 31, 16, 25,  0,  0,  0,  0,  0,  1,  0,  0 },
        { 34,  0,  3, 14, 23,  0,  0, 11, 36,  0,  0, 25,  0, 19, 15,  0,  5, 32, 20, 26,  0, 18, 16,  0, 12, 28, 33, 30, 35, 24,  0, 22,  0,  0,  4, 13 },
        {  0, 21, 20,  9, 16, 13, 12,  0, 23,  0,  0,  0, 17, 31,  0, 25,  7, 26, 35, 30,  0,  0,  0, 32, 29,  0, 10,  0, 36, 18,  0, 14, 33,  6,  3,  0 },
        {  8, 32,  0, 28,  5,  0, 20, 18, 17, 22, 16, 13, 12,  1, 33, 14,  0,  0, 27, 31,  0,  0,  0, 25, 34,  0,  0, 19,  0,  2, 29, 26, 36, 11, 10,  9 },
        {  9,  0, 11,  0,  0, 16,  0, 12,  0, 33,  0, 35,  0, 27, 22,  0,  0,  4,  0, 34,  2,  0,  6, 23,  0,  0,  0, 36,  0,  0, 28,  8, 32, 15,  0,  5 },
        {  0, 34, 30,  3,  0, 12, 18,  4, 11,  7,  0,  0,  2, 15, 14,  0,  0,  8, 13, 27,  0, 20,  0,  0,  0,  0,  0, 33, 19,  0, 26, 10,  0, 29,  0, 17 },
        { 22,  0, 25,  0,  0,  4,  6, 34,  0,  3,  0, 23, 13, 29, 26,  0, 18,  0,  0,  0,  0, 15, 32,  0, 11, 20,  9, 17, 31,  0, 14,  0,  1, 33,  0, 35 },
        {  0,  5,  0,  0,  6,  8,  0,  0,  0,  0,  9,  0, 28, 34,  0, 33, 19, 23, 26, 29, 31,  7, 25, 36, 32, 15, 30,  0,  1, 12,  0,  0,  0, 16, 22,  0 },
        { 31, 29, 13,  0,  0, 10, 14,  0,  0,  0,  0,  0,  9, 20, 25, 17, 11, 16, 30, 12,  1, 33, 28, 35, 21, 27, 18,  0,  0,  0, 19, 34,  6,  3,  0, 23 },
        {  0,  0,  0,  0,  0, 33, 26,  0,  0,  0,  0,  0, 32,  0, 30,  0,  1,  0,  0, 10, 11, 17, 22,  4,  2,  8,  0, 34,  6,  5, 13,  0,  0, 36,  0,  0 },
        { 29, 26, 36, 31, 17, 25,  5, 32,  8,  0, 23,  1,  7, 18,  0, 11, 10, 22,  0,  2,  0, 14,  0,  6,  0,  0, 27, 21,  4,  9, 35, 28,  0, 24,  0, 19 },
        {  0,  0, 12, 19, 33, 14,  0,  0, 27, 13,  7,  0,  0,  2,  0,  1,  3,  0,  0, 25,  0, 11, 36, 21,  0, 32,  0, 28,  0, 30, 16,  9,  0, 22, 17,  0 },
        {  4,  0, 16, 11, 20, 22,  0,  2, 35, 14, 33,  0, 27,  0,  0, 21, 29,  9,  8,  0,  5, 24,  3, 19, 10, 26, 36, 31,  7, 25,  0,  0, 15, 30,  0,  1 },
        { 23,  0, 34,  1,  3,  0, 10, 26, 29, 11, 36,  9,  5, 30,  8,  0,  0, 14,  0, 18,  0, 31, 17, 22,  0, 24, 12,  2, 33,  6,  4, 13, 27, 21,  0, 25 },
        {  0, 13, 27, 21, 10,  9, 15, 28,  0, 24, 12,  0,  0, 26, 16, 31,  0, 25,  0, 32,  0,  0, 35,  1, 17, 18,  0, 11, 29, 22, 33,  0,  5, 14,  0,  0 },
        {  0,  0,  8, 30, 15,  0, 16,  0,  0, 25,  0, 21,  0, 32, 12, 28,  0,  0,  4,  0, 27, 26,  7, 13, 23,  0, 34,  1,  3,  0, 10,  0, 29, 18, 36, 31 },
        {  0, 16,  9, 20, 31, 17,  0,  0, 24,  0,  0,  0, 18,  0, 11, 27, 22,  7, 19, 23, 14, 34,  0,  0, 26, 10,  0,  0,  0, 36,  0,  0,  0,  0,  0, 15 },
        {  0, 23, 32,  0,  0,  5,  0,  0, 18, 29, 11, 10,  6,  0,  2, 34, 14,  3,  9, 20,  0, 36, 21, 27, 19, 12, 28,  0,  0,  0,  0,  4, 26, 17, 31,  7 },
        { 21,  4,  0,  0, 13,  0,  2, 23,  0, 34, 14,  3,  0, 10,  0, 29,  0,  0,  0,  0, 28,  8, 30, 15,  0, 16, 22, 20,  0, 17,  1,  0, 24,  0, 19, 33 },
        {  6,  0,  2,  0, 14, 34,  9,  0, 13, 36,  0, 27, 19, 12,  0, 23, 24,  5, 25,  4, 26,  0, 31,  0,  1,  0,  0,  0, 30,  3, 22, 29, 18,  0, 11, 16 },
        { 18,  0,  0, 29,  0,  0, 28,  5,  1,  8,  0,  0, 26, 16, 13, 20, 21, 17,  2, 35,  0, 12, 24,  0,  9,  0, 31, 27,  0,  7, 30, 23, 14,  0,  0,  3 },
        { 19,  0,  0, 12, 24, 35, 25,  7, 26, 20, 31,  0,  1, 33, 32,  8, 30,  0,  0, 16,  0, 29,  0,  0,  6,  5,  2,  0, 14,  0,  0,  0, 13, 10,  0, 36 },
        {  0, 25, 10, 26, 29, 31,  8,  1,  0,  0, 34,  0,  4,  0,  0, 18, 17, 11,  0,  6,  3,  2,  5, 14, 20,  0,  7,  0, 16, 21, 15,  0, 23, 28, 35, 24 },
        { 33,  1, 35, 32,  0, 30,  0, 25,  0, 26,  0,  0,  3, 28, 23,  6,  0,  2, 36, 21, 10,  9, 29,  0, 15,  0,  5, 24,  0, 14,  0,  0, 16, 13,  0, 22 },
        {  0, 22, 17,  0, 27, 11, 35,  0,  0,  2, 15, 14, 29,  9,  0,  0, 16,  0, 34, 19, 23, 28,  8, 24,  0, 25,  4,  0, 10, 31,  0,  1,  0, 32, 33,  0 },
        {  3, 14,  0,  6,  0, 28,  0, 21,  0,  0,  0, 31, 15,  0,  5, 30,  0, 19,  0,  0, 16, 13, 20,  0,  0,  2, 35, 32, 12,  1,  0, 25,  4,  0, 27,  0 },
        { 16,  0,  4, 13,  0,  0, 23, 19, 33, 28,  3, 24, 10, 25, 36, 26, 20, 31,  0,  1, 35, 32, 12, 30,  0, 22,  0,  0,  0, 11,  8,  6, 34,  2,  0, 14 },
        { 15,  0,  5, 24,  0,  2, 17,  0, 16,  9, 20, 22,  0, 14,  0,  0, 12,  0,  7, 11,  4, 25, 27, 31,  3, 30, 23,  6, 34, 28,  0,  0,  0,  0,  0,  0 }
    };

    // Copy the static grid into a dynamic one-dimensional array
    int *sudoku = malloc(N * N * sizeof(int));
    if (!sudoku) {
        perror("Memory allocation failed");
        exit(1);
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            sudoku[i * N + j] = static_grid[i][j];

    printf("Original Sudoku:\n");
    print_sudoku(sudoku, N);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    sudoku_solver_parallel_pthread(sudoku, N, SUBGRID, 0);
    clock_gettime(CLOCK_MONOTONIC, &end);

    long sec_diff = end.tv_sec - start.tv_sec;
    long nsec_diff = end.tv_nsec - start.tv_nsec;
    if (nsec_diff < 0) {
        sec_diff--;
        nsec_diff += 1000000000;
    }
    double total_ms = sec_diff * 1000.0 + nsec_diff / 1000000.0;

    printf("\nSolved Sudoku:\n");
    print_time();
    print_sudoku(sudoku, N);
    printf("\nTime taken (pthread with max threads): %.2f ms\n", total_ms);

    free(sudoku);
    return 0;
}


