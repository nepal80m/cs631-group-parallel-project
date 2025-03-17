#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Function to check if a number can be placed in a cell
int is_valid(int **board, int row, int col, int num, int N, int BLOCK_SIZE) {
    // Check row and column
    for (int i = 0; i < N; i++) {
        if (board[row][i] == num || board[i][col] == num) {
            return 0;
        }
    }

    // Check subgrid
    int startRow = (row / BLOCK_SIZE) * BLOCK_SIZE;
    int startCol = (col / BLOCK_SIZE) * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (board[startRow + i][startCol + j] == num) {
                return 0;
            }
        }
    }

    return 1;
}

// Backtracking solver
int solve_sudoku_serial(int **board, int N, int BLOCK_SIZE) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (board[row][col] == 0) {
                for (int num = 1; num <= N; num++) {
                    if (is_valid(board, row, col, num, N, BLOCK_SIZE)) {
                        board[row][col] = num;

                        if (solve_sudoku_serial(board, N, BLOCK_SIZE)) {
                            return 1;
                        }

                        board[row][col] = 0; // Backtrack
                    }
                }
                return 0; // No valid number found
            }
        }
    }
    return 1; // Sudoku solved
}

// Function to print the Sudoku board
void print_board(int **board, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%2d ", board[i][j]);
        }
        printf("\n");
    }
}

// Function to dynamically allocate memory for the board
int **allocate_board(int N) {
    int **board = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        board[i] = (int *)malloc(N * sizeof(int));
    }
    return board;
}

// Function to free memory allocated for the board
void free_board(int **board, int N) {
    for (int i = 0; i < N; i++) {
        free(board[i]);
    }
    free(board);
}

int main() {
    int N, BLOCK_SIZE;

    // Input grid size
    printf("Enter the size of the Sudoku board (e.g., 4, 9, 16, 25): ");
    scanf("%d", &N);
    BLOCK_SIZE = (int)sqrt(N);

    if (BLOCK_SIZE * BLOCK_SIZE != N) {
        printf("Invalid board size! Size must be a perfect square (e.g., 4, 9, 16, 25).\n");
        return 1;
    }

    // Allocate memory for the board
    int **board = allocate_board(N);

    // Input the initial board configuration
    printf("Enter the initial Sudoku board (use 0 for empty cells):\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            scanf("%d", &board[i][j]);
        }
    }

    printf("\nInitial Sudoku Board:\n");
    print_board(board, N);

    clock_t start = clock();
    solve_sudoku_serial(board, N, BLOCK_SIZE);
    clock_t end = clock();

    printf("\nSudoku Solved Successfully:\n");
    print_board(board, N);

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTime taken by serial solver: %f ms\n", time_taken*1000);

    // Free memory
    free_board(board, N);

    return 0;
}

