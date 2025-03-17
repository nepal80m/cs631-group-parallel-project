#include <stdio.h>
// CUDA header file
#include "cuda_runtime.h"
#include <device_launch_parameters.h>
#ifndef __CUDACC__
#define __CUDACC__
#endif
// UNASSIGNED is used for empty cells in Sudoku grid 
#define UNASSIGNED 0

// BOX_W is used for the length of one of the square sub-regions of the Sudoku grid.
// Overall length will be N * N.
#define BOX_W 4
#define N (BOX_W * BOX_W)
#define NSQRD  (N * N)
#define N_2 ( N * N  * 2)
#define CANS 1


// Function to check if a number can be placed in a cell
int is_valid(int board[N][N], int row, int col, int num, int BLOCK_SIZE) {
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
int solve_sudoku_serial(int board[N][N], int BLOCK_SIZE) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (board[row][col] == 0) {
                for (int num = 1; num <= N; num++) {
                    if (is_valid(board, row, col, num, BLOCK_SIZE)) {
                        board[row][col] = num;

                        if (solve_sudoku_serial(board, BLOCK_SIZE)) {
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

__global__ void solve(int* d_a, int* d_flag) {
	// Used to remember which row | col | box ( section ) have which values
	__shared__ bool rowHas[N][N];
	__shared__ bool colHas[N][N];
	__shared__ bool boxHas[N][N];

	// Used to ensure that the table has changed
	__shared__ bool changed;

	// Number of spaces which can place the number in each section
	__shared__ int rowCount[N][N];
	__shared__ int colCount[N][N];
	__shared__ int boxCount[N][N];

	__shared__ int sudoku[NSQRD];
	// Where the square is located in the Sudoku
	int row = threadIdx.x;
	int col = threadIdx.y;
	int box = col / BOX_W + (row / BOX_W) * BOX_W;

	// Square's location in the Sudoku
	int gridIdx = row * N + col;
	int Ngrid = N * N + gridIdx;

	sudoku[gridIdx] = d_a[gridIdx] - 1;
	// Unique identifier for each square in row, col, box
	// Corresponds to the generic Sudoku Solve
	// Using a Sudoku to solve a Sudoku !!!
	int offset = row + (col % BOX_W) * BOX_W + (box % BOX_W);

	bool notSeen[N];
	for (int i = 0; i < N; ++i)
		notSeen[i] = true;

	rowHas[row][col] = false;
	colHas[row][col] = false;
	boxHas[row][col] = false;
	__syncthreads(); 

	// Previous loop has not changed any values
	int loopCount = 0;
	do {
		// RESET counters
		rowCount[row][col] = 0;
		colCount[row][col] = 0;
		boxCount[row][col] = 0;

		int s_at = sudoku[gridIdx];
		if (s_at != -1) {
			rowHas[row][s_at] = true;
			colHas[col][s_at] = true;
			boxHas[box][s_at] = true;
		}
		

		__syncthreads();

		changed = false;
		int count = 0;  // number of values which can fit in this square
		int guess = 0; // last value found which can fit in this square
		for (int idx = 0; idx < N; ++idx) {
			// Ensures that every square in each section is working on a different number in the section
			int num = (idx + offset) % N;
			if (s_at == -1 && notSeen[num]) {
				if (rowHas[row][num] || boxHas[box][num] || colHas[col][num])
					notSeen[num] = false;
				else {
					++count;
					guess = num;
					rowCount[row][num] += Ngrid;
					colCount[col][num] += Ngrid;
					boxCount[box][num] += Ngrid;
				}
			}
			__syncthreads();
		}
		if (count == 1)
			sudoku[gridIdx] = guess;

		// Find values which can go in only one spot in the section
		guess = rowCount[row][col];
		if (guess > 0 && guess < N_2)
			sudoku[guess - NSQRD] = col;
		
		guess = boxCount[row][col];
		if (guess > 0 && guess < N_2)
			sudoku[guess - NSQRD] = col;
		
		guess = colCount[row][col];
		if (guess > 0 && guess < N_2)
			sudoku[guess - NSQRD] = col;

		if (sudoku[gridIdx] == -1)
			changed = true;
		__syncthreads();

	} while (changed && ++loopCount < N);

	if (row == 0 && col == 0) {
		// print the number of loops
		printf("Number of loops: %d\n", loopCount);
		}

	
	int s_at = sudoku[gridIdx];
	if (s_at != -1) {
		rowHas[row][s_at] = true;
		colHas[col][s_at] = true;
		boxHas[box][s_at] = true;
	}

	d_a[gridIdx] = sudoku[gridIdx] + 1;
	__syncthreads();
	if (!(rowHas[row][col] && colHas[row][col] && boxHas[row][col]))
		*d_flag = 1; //HAVE NOT SOLVED THE SUDOKU
		
}


// read file
int readMatrixFromFile(const char* filename, int matrix[N][N]) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Unable to open file %s\n", filename);
        return -1;  // fail
    }

    // obtain the data
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                printf("Error: Invalid matrix data at row %d, column %d\n", i + 1, j + 1);
                fclose(file);
                return -1;  // fail
            }
        }
    }

    fclose(file);
    return 0;  // success
}

void print(int result[N][N]) {
	for (int row = 0; row < N; row++) {
		for (int col = 0; col < N; col++)
			printf("%3d", result[row][col]);
		printf("\n");
	}
}

int main() {
    // List of file names
    const char* filenames[] = {
        "testcase/16_easy.txt",
        "testcase/16_medium.txt",
        "testcase/16_hard.txt"
    };
    int num_files = sizeof(filenames) / sizeof(filenames[0]);

    for (int file_idx = 0; file_idx < num_files; file_idx++) {
        const char* filename = filenames[file_idx];
        int h_a[N][N];

        // read matrix from file
        if (readMatrixFromFile(filename, h_a)) {
            printf("Failed to read matrix from file %s.\n", filename);
            continue; // Skip to the next file
        }

        // print matrix
        printf("Matrix read from file %s:\n", filename);
        print(h_a);

        int* d_a;      // Table
        int* d_flag;   // Flag to indicate if the Sudoku is solved
        int h_flag = 0; // Host flag, initialized to 1 (solved)
        cudaMalloc((void**)&d_a, N * N * sizeof(int));
        cudaMalloc((void**)&d_flag, sizeof(int));
        // Copy Sudoku to device
        cudaMemcpy(d_a, h_a, N * N * sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy(d_flag, &h_flag, sizeof(int), cudaMemcpyHostToDevice);

        // Set up the grid and block dimensions
        dim3 dBlock(N, N);

        // record start time
        cudaEvent_t start, stop;
        float elapsedTime;
		double time_taken = 0.;

        cudaEventCreate(&start);
        cudaEventCreate(&stop);
        cudaEventRecord(start, 0);

        solve<<<1, dBlock>>>(d_a, d_flag);

        // record end time
        cudaEventRecord(stop, 0);
        cudaEventSynchronize(stop);
        cudaEventElapsedTime(&elapsedTime, start, stop);

        // printf("Solve function execution time for file %s: %f ms\n", filename, elapsedTime);

        // Copy Sudoku and flag back to host
        cudaMemcpy(h_a, d_a, N * N * sizeof(int), cudaMemcpyDeviceToHost);
        cudaMemcpy(&h_flag, d_flag, sizeof(int), cudaMemcpyDeviceToHost);

        // if cuda is not able to solve the sudoku, let the remaining solve by serial
        if (h_flag == 1) {
            // serial solve
			print(h_a);

			printf("Solving the remaining Sudoku by serial...\n");
            clock_t c_start = clock();
            solve_sudoku_serial(h_a, BOX_W);
            clock_t c_end = clock();
            time_taken = ((double)(c_end - c_start)) / CLOCKS_PER_SEC;
            // printf("Serial solve execution time for file %s: %f ms\n", filename, time_taken * 1000);
        }

        // Check if solved
        if (h_a[0][0])
            print(h_a);
        else
            printf("No solution could be found for file %s.\n", filename);

        cudaFree(d_a);
        cudaFree(d_flag);
		
		// print total time elapsedTime + time_taken
		printf("Total execution time for file %s: %f ms\n", filename, elapsedTime+time_taken * 1000);
    }

    return 0;
}