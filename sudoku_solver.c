#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Include for timing

void print_sudoku(int sudoku[9][9])
{
    printf("The Sudoku contains:\n");
    for (int j = 0; j < 9; j++)
    {
        for (int i = 0; i < 9; i++)
        {
            printf("%d  ", sudoku[j][i]);
        }
        printf("\n");
    }
}

int check_square(int sudoku[9][9], int num, int row, int col)
{
    int row_start = row - row % 3;
    int col_start = col - col % 3;
    for (int i = row_start; i < row_start + 3; i++)
    {
        for (int j = col_start; j < col_start + 3; j++)
        {
            if (sudoku[i][j] == num)
            {
                return 0;
            }
        }
    }
    return 1;
}

int check_sudoku(int sudoku[9][9], int num, int row, int col)
{
    for (int i = 0; i < 9; i++)
    {
        if (num == sudoku[i][col] || num == sudoku[row][i])
        {
            return 0;
        }
    }
    return check_square(sudoku, num, row, col);
}

int find_unassigned(int sudoku[9][9], int *row, int *col)
{
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (sudoku[i][j] == 0)
            {
                *row = i;
                *col = j;
                return 1;
            }
        }
    }
    return 0;
}

int sudoku_solver_serial(int sudoku[9][9])
{
    int row, col;
    if (!find_unassigned(sudoku, &row, &col))
    {
        return 1; // Puzzle solved
    }

    for (int num = 1; num <= 9; num++)
    {
        if (check_sudoku(sudoku, num, row, col))
        {
            sudoku[row][col] = num;
            if (sudoku_solver_serial(sudoku))
            {
                return 1;
            }
            sudoku[row][col] = 0;
        }
    }
    return 0;
}

void solve_sudoku_serial(int sudoku[9][9])
{
    sudoku_solver_serial(sudoku);
}

#ifndef __testing
int main()
{
    int Sudoku[9][9] = {
        {1, 0, 6, 0, 0, 2, 3, 0, 0},
        {0, 5, 0, 0, 0, 6, 0, 9, 1},
        {0, 0, 9, 5, 0, 1, 4, 6, 2},
        {0, 3, 7, 9, 0, 5, 0, 0, 0},
        {5, 8, 1, 0, 2, 7, 9, 0, 0},
        {0, 0, 0, 4, 0, 8, 1, 5, 7},
        {0, 0, 0, 2, 6, 0, 5, 4, 0},
        {0, 0, 4, 1, 5, 0, 6, 0, 9},
        {9, 0, 0, 8, 7, 4, 2, 1, 0}};

    printf("Input puzzle is:\n");
    print_sudoku(Sudoku);

    // Start timer
    clock_t start = clock();

    solve_sudoku_serial(Sudoku);

    // Stop timer
    clock_t end = clock();

    printf("Solution is:\n");
    print_sudoku(Sudoku);

    // Calculate and print elapsed time in milliseconds
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Time taken to solve (serial): %.2f ms\n", time_taken);

    return 0;
}
#endif
