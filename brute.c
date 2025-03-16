#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#define N 25  
#define SUBGRID 5  

// Pritn the grid
void printGrid(int grid[N][N]) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            printf("%d ", grid[row][col]);
        }
        printf("\n");
    }
}

// Check if a number is valid in a given cell
bool isValid(int grid[N][N], int row, int col, int num) {
    for (int i = 0; i < N; i++) {
        if (grid[row][i] == num || grid[i][col] == num) return false;
    }

    int startRow = row - row % SUBGRID, startCol = col - col % SUBGRID;
    for (int i = 0; i < SUBGRID; i++) {
        for (int j = 0; j < SUBGRID; j++) {
            if (grid[startRow + i][startCol + j] == num) return false;
        }
    }
    return true;
}

// Brute-force function that systematically fills empty cells
bool bruteForceSolve(int grid[N][N]) {
    int row, col;
    bool isEmpty = false;

    // Find first empty cell
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            if (grid[row][col] == 0) {
                isEmpty = true;
                break;
            }
        }
        if (isEmpty) break;
    }

    // If no empty cells are left, the Sudoku is solved
    if (!isEmpty) return true;

    // Try numbers from 1-N in order
    for (int num = 1; num <= N; num++) {
        if (isValid(grid, row, col, num)) {
            grid[row][col] = num;

            if (bruteForceSolve(grid)) return true; // Recursively continue

            // If placing num didn't lead to a solution, reset and try next
            grid[row][col] = 0;
        }
    }

    return false;  // No valid number found, triggering backtracking
}

// Driver function
int main() {
int grid[N][N] = {

    {0, 2, 3, 4, 5, 6, 7, 8, 0, 10, 11, 0, 13, 14, 15, 16, 0, 18, 19, 20, 21, 22, 23, 0, 25},
    {6, 7, 8, 9, 10, 0, 12, 0, 14, 15, 0, 17, 18, 0, 20, 21, 22, 0, 24, 25, 1, 2, 3, 4, 5},
    {11, 12, 13, 14, 0, 16, 17, 18, 19, 0, 21, 22, 23, 24, 25, 0, 0, 3, 4, 5, 6, 7, 8, 9, 0},
    {16, 17, 0, 19, 20, 0, 22, 23, 24, 25, 1, 2, 0, 4, 5, 6, 7, 8, 9, 10, 0, 12, 13, 14, 0},
    {21, 22, 23, 24, 25, 0, 2, 3, 4, 0, 6, 7, 8, 0, 10, 11, 12, 13, 0, 15, 0, 17, 18, 19, 20},
    {2, 3, 4, 5, 6, 7, 0, 9, 10, 0, 12, 13, 14, 15, 0, 17, 18, 19, 0, 21, 22, 23, 24, 25, 0},
    {7, 8, 9, 10, 11, 0, 13, 14, 15, 16, 0, 18, 19, 20, 21, 0, 23, 24, 25, 1, 2, 3, 0, 5, 6},
    {12, 13, 14, 15, 16, 0, 18, 19, 20, 21, 22, 0, 24, 25, 1, 2, 3, 0, 5, 6, 7, 8, 9, 0, 11},
    {17, 18, 19, 20, 21, 0, 23, 24, 25, 1, 2, 3, 4, 0, 6, 7, 8, 9, 10, 0, 12, 13, 14, 15, 16},
    {22, 23, 24, 25, 1, 0, 3, 4, 5, 6, 7, 8, 9, 0, 11, 12, 13, 14, 15, 0, 17, 18, 19, 20, 0},
    {3, 4, 5, 6, 7, 0, 9, 10, 0, 12, 13, 0, 15, 16, 0, 18, 19, 20, 21, 0, 23, 24, 25, 1, 2},
    {8, 9, 10, 11, 12, 0, 14, 15, 16, 0, 18, 19, 20, 21, 0, 23, 24, 25, 1, 0, 3, 4, 5, 6, 7},
    {13, 14, 15, 16, 0, 18, 19, 20, 0, 22, 23, 24, 25, 0, 2, 3, 4, 5, 6, 0, 8, 9, 10, 11, 12},
    {18, 19, 0, 21, 22, 23, 24, 25, 1, 0, 3, 4, 5, 6, 7, 0, 9, 10, 11, 12, 0, 14, 15, 16, 17},
    {23, 24, 25, 1, 2, 0, 4, 5, 6, 7, 0, 9, 10, 11, 12, 0, 14, 15, 16, 17, 0, 19, 20, 21, 22},
    {4, 5, 6, 7, 0, 9, 10, 11, 12, 0, 14, 15, 16, 17, 0, 19, 20, 21, 22, 0, 24, 25, 1, 2, 3},
    {9, 10, 11, 12, 13, 0, 15, 16, 17, 0, 19, 20, 21, 22, 0, 24, 25, 1, 2, 0, 4, 5, 6, 7, 8},
    {14, 15, 16, 17, 0, 19, 20, 21, 22, 0, 24, 25, 1, 2, 0, 4, 5, 6, 7, 0, 9, 10, 11, 12, 13},
    {19, 20, 21, 22, 0, 24, 25, 1, 2, 0, 4, 5, 6, 7, 0, 9, 10, 11, 12, 0, 14, 15, 16, 17, 18},
    {24, 25, 1, 2, 3, 0, 5, 6, 7, 8, 0, 10, 11, 12, 13, 0, 15, 16, 17, 18, 0, 20, 21, 22, 23},
    {5, 6, 7, 8, 9, 0, 11, 12, 13, 14, 0, 16, 17, 18, 19, 0, 21, 22, 23, 24, 0, 1, 2, 3, 4},
    {10, 11, 12, 13, 0, 15, 16, 17, 18, 0, 20, 21, 22, 23, 0, 25, 1, 2, 3, 0, 5, 6, 7, 8, 9},
    {15, 16, 17, 18, 0, 20, 21, 22, 23, 0, 25, 1, 2, 3, 0, 5, 6, 7, 8, 0, 10, 11, 12, 13, 14},
    {20, 21, 22, 23, 0, 25, 1, 2, 3, 0, 5, 6, 7, 8, 0, 10, 11, 12, 13, 0, 15, 16, 17, 18, 19},
    {25, 1, 2, 3, 4, 0, 6, 7, 8, 9, 0, 11, 12, 13, 14, 0, 16, 17, 18, 19, 0, 21, 22, 23, 24}
};




    printf("Original 25x25 Sudoku Puzzle:\n");
    printGrid(grid);

    clock_t start = clock();

    if (bruteForceSolve(grid)) {
        clock_t end = clock();
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
        printf("\nSolved 25x25 Sudoku:\n");
        printGrid(grid);
        printf("\nTime taken: %.3f milliseconds\n", time_taken);
    } else {
        printf("\nNo solution exists.\n");
    }

    return 0;
}

