#!/bin/bash
#SBATCH --partition=compute   ### Partition
#SBATCH --job-name=SudokuJob ### Job Name
#SBATCH --time=01:00:00     ### WallTime
#SBATCH --nodes=1           ### Number of Nodes
#SBATCH --ntasks-per-node=1 ### Number of tasks (MPI processes)
#SBATCH --cpus-per-task=28  ### Number of threads per task (OMP threads)
#SBATCH --mem=100g
#SBATCH --account=cs631    ### Account used for job submission

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
./b.out sudoku_puzzle_121_001.txt