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
    [0.00005, 0.00027, 11.7087, 45.82583, 68, 295, 1431], # testing 1
]
openmp2_times = [
    [0.00019,0.00049,11.75687,44.89533,61,86, 1262],
    ]
openmp4_times = [
    [0.0003, 0.00047,11.68618, 45.907, 28.8557, 56.63862, 1276],
                 ]
openmp6_times = [
    [0.00057,0.0005,11.27572,44.73033,28.45861,56.502,1292],
    ]
openmp8_times = [
    [0.00047, 0.00056, 11.2521,45.14718, 29.52103, 48.47616, 1261],
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
