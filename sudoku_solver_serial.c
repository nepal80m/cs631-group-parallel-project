#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void print_sudoku(int *sudoku, int grid_size)
{
    printf("The Sudoku contains:\n");
    for (int i = 0; i < grid_size; i++)
    {
        for (int j = 0; j < grid_size; j++)
        {
            printf("%d  ", sudoku[i * grid_size + j]);
        }
        printf("\n");
    }
}

int check_square(int *sudoku, int grid_size, int block_size, int num, int row, int col)
{
    int row_start = row - row % block_size;
    int col_start = col - col % block_size;
    for (int i = row_start; i < row_start + block_size; i++)
    {
        for (int j = col_start; j < col_start + block_size; j++)
        {
            if (sudoku[i * grid_size + j] == num)
            {
                return 0;
            }
        }
    }
    return 1;
}

int check_sudoku(int *sudoku, int grid_size, int block_size, int num, int row, int col)
{
    for (int i = 0; i < grid_size; i++)
    {
        if (sudoku[row * grid_size + i] == num || sudoku[i * grid_size + col] == num)
        {
            return 0;
        }
    }
    return check_square(sudoku, grid_size, block_size, num, row, col);
}

int find_unassigned(int *sudoku, int grid_size, int *row, int *col)
{
    for (int i = 0; i < grid_size; i++)
    {
        for (int j = 0; j < grid_size; j++)
        {
            if (sudoku[i * grid_size + j] == 0)
            {
                *row = i;
                *col = j;
                return 1;
            }
        }
    }
    return 0;
}

int sudoku_solver_serial(int *sudoku, int grid_size, int block_size)
{
    int row, col;
    if (!find_unassigned(sudoku, grid_size, &row, &col))
        return 1; // Puzzle solved

    for (int num = 1; num <= grid_size; num++)
    {
        if (check_sudoku(sudoku, grid_size, block_size, num, row, col))
        {
            sudoku[row * grid_size + col] = num;
            if (sudoku_solver_serial(sudoku, grid_size, block_size))
                return 1;
            sudoku[row * grid_size + col] = 0;
        }
    }
    return 0;
}

void solve_sudoku_serial(int *sudoku, int grid_size, int block_size)
{
    sudoku_solver_serial(sudoku, grid_size, block_size);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp)
    {
        perror("Error opening file");
        return 1;
    }

    int grid_size;
    // File format: first integer is grid size (e.g., 9 for a 9x9 sudoku)
    if (fscanf(fp, "%d", &grid_size) != 1)
    {
        fprintf(stderr, "Error reading grid size from file.\n");
        fclose(fp);
        return 1;
    }

    int block_size = sqrt(grid_size);
    if (block_size * block_size != grid_size)
    {
        fprintf(stderr, "Grid size must be a perfect square (e.g., 4, 9, 16, ...).\n");
        fclose(fp);
        return 1;
    }

    int *sudoku = malloc(grid_size * grid_size * sizeof(int));
    if (!sudoku)
    {
        perror("Memory allocation failed");
        fclose(fp);
        return 1;
    }

    // Read the sudoku grid from the file.
    for (int i = 0; i < grid_size * grid_size; i++)
    {
        if (fscanf(fp, "%d", &sudoku[i]) != 1)
        {
            fprintf(stderr, "Error reading sudoku grid from file.\n");
            free(sudoku);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);

    printf("Input puzzle is:\n");
    print_sudoku(sudoku, grid_size);

    struct timespec start, end;

    // clock_t start = clock();
    clock_gettime(CLOCK_MONOTONIC, &start);
    solve_sudoku_serial(sudoku, grid_size, block_size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    // clock_t end = clock();

    printf("Solution is:\n");
    print_sudoku(sudoku, grid_size);

    long sec_diff = end.tv_sec - start.tv_sec;
    long nsec_diff = end.tv_nsec - start.tv_nsec;
    if (nsec_diff < 0)
    {
        sec_diff--;
        nsec_diff += 1000000000;
    }

    double total_ms = sec_diff * 1000.0 + nsec_diff / 1000000.0;
    int hrs = total_ms / (3600.0 * 1000.0);
    double remainder = total_ms - hrs * 3600.0 * 1000.0;
    int mins = remainder / (60.0 * 1000.0);
    remainder -= mins * 60.0 * 1000.0;
    int secs = remainder / 1000.0;
    double ms = remainder - secs * 1000.0;
    // double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Time taken to solve (serial): %02dhr: %02dmin: %02dsec: %06.2fms\n", hrs, mins, secs, ms);

    free(sudoku);
    return 0;
}
