# cs631-group-parallel-project

### To generate Sudoku of size n (eg.100, default is 9)
```
make generate_sudoku n=100
```

### To check the sbatch memory usage
```
sacct -j JOBID --format=JobID,JobName,ReqMem,MaxRSS,Elapsed
```

### Check partition list
```
sinfo -e -O partition,cpus,memory,nodes
```