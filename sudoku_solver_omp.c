#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define PARALLEL_CUTOFF 2 // Only create tasks for recursion levels < this cutoff

void print_time()
{
    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s\n", buffer);
}

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

// Parallel backtracking solver using OpenMP tasks.
// The extra "depth" parameter is used to limit task creation.
int sudoku_solver_parallel(int *sudoku, int grid_size, int block_size, int depth)
{
    int row, col;
    if (!find_unassigned(sudoku, grid_size, &row, &col))
        return 1; // Puzzle solved

    int solved = 0;
    for (int num = 1; num <= grid_size && !solved; num++)
    {
        if (check_sudoku(sudoku, grid_size, block_size, num, row, col))
        {
            if (depth < PARALLEL_CUTOFF)
            {
                // Create a new task: work on a copy of the sudoku grid.
                int *sudoku_copy = malloc(grid_size * grid_size * sizeof(int));
                if (!sudoku_copy)
                {
                    perror("Memory allocation failed");
                    exit(1);
                }
                memcpy(sudoku_copy, sudoku, grid_size * grid_size * sizeof(int));
                sudoku_copy[row * grid_size + col] = num;

#pragma omp task shared(solved) firstprivate(sudoku_copy, grid_size, block_size, depth, row, col)
                {
                    if (sudoku_solver_parallel(sudoku_copy, grid_size, block_size, depth + 1))
                    {
#pragma omp critical
                        {
                            if (!solved)
                            {
                                solved = 1;
                                print_time();
                                // Copy found solution back to original sudoku
                                memcpy(sudoku, sudoku_copy, grid_size * grid_size * sizeof(int));
                            }
                        }
                    }
                    free(sudoku_copy);
                }
            }
            else
            {
                // For deeper recursion levels, proceed serially.
                sudoku[row * grid_size + col] = num;
                if (sudoku_solver_parallel(sudoku, grid_size, block_size, depth + 1))
                    return 1;
                sudoku[row * grid_size + col] = 0; // Backtrack
            }
        }
    }
#pragma omp taskwait
    return solved;
}

void solve_sudoku_parallel(int *sudoku, int grid_size, int block_size)
{
#pragma omp parallel
    {
#pragma omp single nowait
        {
            sudoku_solver_parallel(sudoku, grid_size, block_size, 0);
        }
    }
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
    // Read grid size from file (e.g., 9 for a 9x9 sudoku)
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
    solve_sudoku_parallel(sudoku, grid_size, block_size);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // clock_t end = clock();

    printf("Solution is:\n");
    print_time();
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
    printf("Time taken to solve (parallel): %02dhr: %02dmin: %02dsec: %06.2fms\n", hrs, mins, secs, ms);

    free(sudoku);
    return 0;
}
