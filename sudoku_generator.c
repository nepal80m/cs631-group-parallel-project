#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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
        fprintf(stderr, "Usage: %s <sudoku size> [<removal_rate: 0.5>]\n", argv[0]);
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

    double removal_rate = 0.5; // Default removal rate
    if (argc > 2)
    {
        removal_rate = atof(argv[2]);
        if (removal_rate < 0 || removal_rate > 1)
        {
            fprintf(stderr, "Error: removal rate must be between 0 and 1.\n");
            return 1;
        }
    }

    srand((unsigned)time(NULL));

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

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            puzzle[i][j] = board[i][j];
        }
    }

    int total_cells = n * n;
    int cells_to_remove = (int)(removal_rate * total_cells);

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
        puzzle[r][c] = 0; // 0 represents an empty cell
    }
    free(cell_indices);

    char puzzleFileName[64];
    char solutionFileName[64];
    sprintf(puzzleFileName, "sudoku_puzzle_%d_%03.0f.txt", n, removal_rate * 100);
    sprintf(solutionFileName, "sudoku_solution_%d_%03.0f.txt", n, removal_rate * 100);

    FILE *puzzleFile = fopen(puzzleFileName, "w");
    FILE *solutionFile = fopen(solutionFileName, "w");
    if (!puzzleFile || !solutionFile)
    {
        fprintf(stderr, "Error opening output files.\n");
        return 1;
    }

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
