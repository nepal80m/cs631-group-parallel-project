import matplotlib.pyplot as plt
import numpy as np

sudoku_types = [
    "9x9_easy", 
    "9x9_medium", 
    "16x16_easy",
    "16x16_medium", 
    "25x25_easy", 
    "25x25_medium",
    "25x25_hard"
]

# please use this order
# [9x9_easy, 9x9_medium, 16x16_easy, 16x16_medium, 25x25_easy, 25x25_medium, 25x25_hard]
serial_times = [
    [0.1, 0.2, 0.3, 0.5, 1.0, 2.0, 4.0], # testing 1
    [0.1, 0.2, 0.3, 0.5, 1.0, 2.0, 4.0], # testing 2
    [0.1, 0.2, 0.3, 0.5, 1.0, 2.0, 4.0], # and soon
    [0.1, 0.2, 0.3, 0.5, 1.0, 2.0, 4.0],
    [0.1, 0.2, 0.3, 0.5, 1.0, 2.0, 4.0],
]
openmp2_times = [
    [0.08, 0.16, 0.25, 0.4, 0.8, 1.6, 3.2],
    [0.08, 0.16, 0.25, 0.4, 0.8, 1.6, 3.2],
    ]
openmp4_times = [
    [0.06, 0.12, 0.20, 0.35, 0.7, 1.4, 2.8],
                 ]
openmp6_times = [
    [0.05, 0.10, 0.18, 0.30, 0.65, 1.2, 2.4],
    ]
openmp8_times = [
    [0.04, 0.08, 0.15, 0.28, 0.6, 1.1, 2.0],
    ]

# calculting avgs
serial_times   = [sum(times)/len(times) for times in zip(*serial_times)]
openmp2_times  = [sum(times)/len(times) for times in zip(*openmp2_times)]
openmp4_times  = [sum(times)/len(times) for times in zip(*openmp4_times)]
openmp6_times  = [sum(times)/len(times) for times in zip(*openmp6_times)]
openmp8_times  = [sum(times)/len(times) for times in zip(*openmp8_times)]


x = np.arange(len(sudoku_types))
plt.figure(figsize=(10, 6))

plt.plot(x, serial_times, marker='o', label='Serial')
plt.plot(x, openmp2_times, marker='o', label='OpenMP (2 threads)')
plt.plot(x, openmp4_times, marker='o', label='OpenMP (4 threads)')
plt.plot(x, openmp6_times, marker='o', label='OpenMP (6 threads)')
plt.plot(x, openmp8_times, marker='o', label='OpenMP (8 threads)')

plt.xticks(x, sudoku_types)
plt.xlabel('Sudoku Types')
plt.ylabel('Execution Time (seconds)')
plt.title('Execution Time of Solving Sudoku Puzzles')
plt.legend()

plt.tight_layout()
plt.savefig("omp_execution_time_graph.png")

plt.show()
