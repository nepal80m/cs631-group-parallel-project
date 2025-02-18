#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Fisher-Yates shuffle for an integer array */
void shuffle(int *array, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <sudoku size>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0)
    {
        fprintf(stderr, "Error: sudoku size must be a positive integer.\n");
        return 1;
    }

    int base = (int)round(sqrt(n));
    if (base * base != n)
    {
        fprintf(stderr, "Error: The sudoku size must be a perfect square.\n");
        return 1;
    }

    /* Seed the random number generator */
    srand((unsigned)time(NULL));

    /* Allocate and initialize an array of indices [0, 1, ..., base-1] */
    int *indices = malloc(base * sizeof(int));
    if (!indices)
    {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }
    for (int i = 0; i < base; i++)
    {
        indices[i] = i;
    }

    /* Create arrays for band (group) ordering for rows and columns */
    int *band_rows = malloc(base * sizeof(int));
    int *band_cols = malloc(base * sizeof(int));
    if (!band_rows || !band_cols)
    {
        fprintf(stderr, "Memory allocation error\n");
        free(indices);
        return 1;
    }
    for (int i = 0; i < base; i++)
    {
        band_rows[i] = indices[i];
        band_cols[i] = indices[i];
    }
    shuffle(band_rows, base);
    shuffle(band_cols, base);

    /* Allocate arrays for the final row and column orderings */
    int *rows = malloc(n * sizeof(int));
    int *cols = malloc(n * sizeof(int));
    if (!rows || !cols)
    {
        fprintf(stderr, "Memory allocation error\n");
        free(indices);
        free(band_rows);
        free(band_cols);
        return 1;
    }

    /* Build the row ordering: for each band, shuffle the inner rows */
    int idx = 0;
    for (int i = 0; i < base; i++)
    {
        int br = band_rows[i];
        int *inner = malloc(base * sizeof(int));
        if (!inner)
        {
            fprintf(stderr, "Memory allocation error\n");
            return 1;
        }
        for (int j = 0; j < base; j++)
        {
            inner[j] = indices[j];
        }
        shuffle(inner, base);
        for (int j = 0; j < base; j++)
        {
            rows[idx++] = br * base + inner[j];
        }
        free(inner);
    }

    /* Build the column ordering: similar to rows */
    idx = 0;
    for (int i = 0; i < base; i++)
    {
        int bc = band_cols[i];
        int *inner = malloc(base * sizeof(int));
        if (!inner)
        {
            fprintf(stderr, "Memory allocation error\n");
            return 1;
        }
        for (int j = 0; j < base; j++)
        {
            inner[j] = indices[j];
        }
        shuffle(inner, base);
        for (int j = 0; j < base; j++)
        {
            cols[idx++] = bc * base + inner[j];
        }
        free(inner);
    }

    /* Generate a random permutation of numbers 1..n */
    int *nums = malloc(n * sizeof(int));
    if (!nums)
    {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }
    for (int i = 0; i < n; i++)
    {
        nums[i] = i + 1;
    }
    shuffle(nums, n);

    /* Allocate 2D arrays for the board and the puzzle */
    int **board = malloc(n * sizeof(int *));
    int **puzzle = malloc(n * sizeof(int *));
    if (!board || !puzzle)
    {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }
    for (int i = 0; i < n; i++)
    {
        board[i] = malloc(n * sizeof(int));
        puzzle[i] = malloc(n * sizeof(int));
        if (!board[i] || !puzzle[i])
        {
            fprintf(stderr, "Memory allocation error\n");
            return 1;
        }
    }

    /* Build the complete sudoku board using the base pattern:
       pattern(r, c) = (base * (r % base) + (r / base) + c) % n */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int r = rows[i];
            int c = cols[j];
            int pos = (base * (r % base) + (r / base) + c) % n;
            board[i][j] = nums[pos];
        }
    }

    /* Copy the solution into the puzzle */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            puzzle[i][j] = board[i][j];
        }
    }

    /* Remove some cells to create the puzzle.
       Here, we remove 50%% of the cells (adjust removal_rate as needed). */
    double removal_rate = 0.5;
    int total_cells = n * n;
    int cells_to_remove = (int)(removal_rate * total_cells);

    /* Create an array of cell indices from 0 to total_cells - 1 and shuffle it */
    int *cell_indices = malloc(total_cells * sizeof(int));
    if (!cell_indices)
    {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }
    for (int i = 0; i < total_cells; i++)
    {
        cell_indices[i] = i;
    }
    shuffle(cell_indices, total_cells);

    for (int i = 0; i < cells_to_remove; i++)
    {
        int index = cell_indices[i];
        int r = index / n;
        int c = index % n;
        puzzle[r][c] = 0; /* 0 represents an empty cell */
    }
    free(cell_indices);

    /* Create file names based on sudoku size */
    char puzzleFileName[64];
    char solutionFileName[64];
    sprintf(puzzleFileName, "sudoku_puzzle_%d.txt", n);
    sprintf(solutionFileName, "sudoku_solution_%d.txt", n);

    /* Write the puzzle and solution to files in a plain whitespace-delimited format.
       The first line contains the dimension, followed by n rows with n space-separated numbers. */
    FILE *puzzleFile = fopen(puzzleFileName, "w");
    FILE *solutionFile = fopen(solutionFileName, "w");
    if (!puzzleFile || !solutionFile)
    {
        fprintf(stderr, "Error opening output files.\n");
        return 1;
    }

    /* Write the dimension as the first line */
    fprintf(puzzleFile, "%d\n", n);
    fprintf(solutionFile, "%d\n", n);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fprintf(puzzleFile, "%d ", puzzle[i][j]);
        }
        fprintf(puzzleFile, "\n");
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fprintf(solutionFile, "%d ", board[i][j]);
        }
        fprintf(solutionFile, "\n");
    }

    fclose(puzzleFile);
    fclose(solutionFile);

    printf("\nPuzzle and solution have been written to '%s' and '%s'.\n", puzzleFileName, solutionFileName);

    /* Free allocated memory */
    free(indices);
    free(band_rows);
    free(band_cols);
    free(rows);
    free(cols);
    free(nums);
    for (int i = 0; i < n; i++)
    {
        free(board[i]);
        free(puzzle[i]);
    }
    free(board);
    free(puzzle);

    return 0;
}
