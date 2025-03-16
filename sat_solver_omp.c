#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#define N 25
#define SUBGRID 5
#define MAX_VARS (N * N * N)  
#define MAX_CLAUSES 300000    
#define MAX_CLAUSE_SIZE 70    

int clauses[MAX_CLAUSES][MAX_CLAUSE_SIZE];  // CNF clauses
int clause_count = 0;
omp_lock_t clause_lock; // Lock for safely adding clauses

// Convert 3D Sudoku representation to SAT variable (1-based indexing)
int var(int row, int col, int num) {
    return (row * N * N) + (col * N) + num + 1;
}

// Add a clause to the CNF formula
void addClause(int literals[], int size) {
    omp_set_lock(&clause_lock);
    if (clause_count >= MAX_CLAUSES) {
        fprintf(stderr, "Error: Exceeded maximum clause storage!\n");
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        clauses[clause_count][i] = literals[i];
    }
    clause_count++;
    omp_unset_lock(&clause_lock);
}

// Print Sudoku Grid
void printGrid(int grid[N][N]) {
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            printf("%d ", grid[r][c]);
        }
        printf("\n");
    }
}

// Ensure each cell has at least one number (1-N)
void encodeCellConstraints() {
    #pragma omp parallel for
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            int clause[N];
            for (int n = 0; n < N; n++) {
                clause[n] = var(r, c, n);
            }
            addClause(clause, N);
        }
    }
}

// Ensure each cell has at most one number
void encodeUniqueCellConstraints() {
    #pragma omp parallel for
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            for (int n1 = 0; n1 < N; n1++) {
                for (int n2 = n1 + 1; n2 < N; n2++) {
                    int clause[] = {-var(r, c, n1), -var(r, c, n2)};
                    addClause(clause, 2);
                }
            }
        }
    }
}

// Ensure each row has unique numbers
void encodeRowConstraints() {
    #pragma omp parallel for
    for (int r = 0; r < N; r++) {
        for (int n = 0; n < N; n++) {
            int clause[N];
            for (int c = 0; c < N; c++) {
                clause[c] = var(r, c, n);
            }
            addClause(clause, N);
        }
    }
}

// Ensure each column has unique numbers
void encodeColConstraints() {
    #pragma omp parallel for
    for (int c = 0; c < N; c++) {
        for (int n = 0; n < N; n++) {
            int clause[N];
            for (int r = 0; r < N; r++) {
                clause[r] = var(r, c, n);
            }
            addClause(clause, N);
        }
    }
}

// Ensure each sqrtN x sqrtN subgrid has unique numbers
void encodeSubgridConstraints() {
    #pragma omp parallel for collapse(2)
    for (int box_r = 0; box_r < SUBGRID; box_r++) {
        for (int box_c = 0; box_c < SUBGRID; box_c++) {
            for (int n = 0; n < N; n++) {
                int clause[N];
                int idx = 0;
                for (int r = 0; r < SUBGRID; r++) {
                    for (int c = 0; c < SUBGRID; c++) {
                        clause[idx++] = var(box_r * SUBGRID + r, box_c * SUBGRID + c, n);
                    }
                }
                addClause(clause, N);
            }
        }
    }
}

// Encode the given Sudoku puzzle as CNF clauses
void encodeSudoku(int grid[N][N]) {
    encodeCellConstraints();
    encodeUniqueCellConstraints();
    encodeRowConstraints();
    encodeColConstraints();
    encodeSubgridConstraints();

    // Encode the initial numbers from the puzzle
    #pragma omp parallel for collapse(2)
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (grid[r][c] != 0) {
                int clause[] = {var(r, c, grid[r][c] - 1)};
                addClause(clause, 1);
            }
        }
    }
}

// Write the CNF formula to a file
void writeCNF(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    fprintf(file, "p cnf %d %d\n", MAX_VARS, clause_count);
    for (int i = 0; i < clause_count; i++) {
        for (int j = 0; j < MAX_CLAUSE_SIZE && clauses[i][j] != 0; j++) {
            fprintf(file, "%d ", clauses[i][j]);
        }
        fprintf(file, "0\n");
    }
    fclose(file);
}

// Solve Sudoku using MiniSat
void solveSudoku(int grid[N][N]) {
    omp_init_lock(&clause_lock);
    encodeSudoku(grid);
    writeCNF("sudoku.cnf");

    printf("\nRunning MiniSat...\n");
    system("minisat sudoku.cnf sudoku.out");

    omp_destroy_lock(&clause_lock);
}

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



    printf("Original Sudoku Puzzle:\n");
    printGrid(grid);

    solveSudoku(grid);
    
    printf("\nSolved Sudoku:\n");
    printGrid(grid);
    
    return 0;
}
