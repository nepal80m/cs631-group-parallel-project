import math

def read_sudoku(filename):
    """Read a sudoku puzzle from a file.
    
    The file should have the grid size n on the first line,
    followed by n lines each containing n space-separated integers.
    """
    with open(filename, 'r') as file:
        # First line contains the dimension (n)
        n = int(file.readline().strip())
        grid = []
        for _ in range(n):
            line = file.readline().strip()
            # Convert each number in the row from string to int
            row = list(map(int, line.split()))
            grid.append(row)
    return n, grid

def find_first_blank(grid):
    """Return the row and column indices of the first blank (0) cell."""
    n = len(grid)
    for i in range(n):
        for j in range(n):
            if grid[i][j] == 0:
                return i, j
    return None, None

def get_candidates(grid, row, col):
    """Return a sorted list of possible candidates for the blank cell at (row, col).
    
    A candidate is valid if it does not appear in the same row, column, or subgrid.
    """
    n = len(grid)
    # Start with all possible numbers from 1 to n.
    candidates = set(range(1, n + 1))
    
    # Remove numbers already in the row.
    candidates -= set(grid[row])
    
    # Remove numbers already in the column.
    candidates -= {grid[i][col] for i in range(n)}
    
    # Determine the size of the subgrid.
    block_size = int(math.sqrt(n))
    if block_size ** 2 != n:
        raise ValueError("Invalid sudoku grid: n must be a perfect square.")
    
    # Find the top-left indices of the subgrid containing (row, col)
    block_row_start = (row // block_size) * block_size
    block_col_start = (col // block_size) * block_size
    
    # Remove numbers already in the subgrid.
    block_numbers = {grid[i][j] 
                     for i in range(block_row_start, block_row_start + block_size)
                     for j in range(block_col_start, block_col_start + block_size)}
    candidates -= block_numbers
    
    return sorted(candidates)

def main():
    filename = "25x25_hard.txt"  # Name of the file containing the sudoku grid.
    n, grid = read_sudoku(filename)
    
    row, col = find_first_blank(grid)
    if row is None:
        print("No blank cells found in the puzzle.")
    else:
        candidates = get_candidates(grid, row, col)
        print(f"First blank cell found at (row {row}, col {col}).")
        print(f"Possible candidates ({len(candidates)}):", candidates)

if __name__ == "__main__":
    main()
