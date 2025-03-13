import math
import argparse

def read_sudoku_from_file(file_path):
    """
    Reads a sudoku solution from a file and returns the board,
    the grid size (n), and the subgrid size (sqrt(n)).
    
    The file must contain exactly n*n integers separated by whitespace,
    where n is a perfect square.
    
    Raises:
        ValueError: If the number of integers is not a perfect square, or if n is not a perfect square.
    """
    with open(file_path, 'r') as f:
        data = f.read().split()
    
    numbers = list(map(int, data))
    total_numbers = len(numbers)
    
    # Determine grid side length
    n = int(math.sqrt(total_numbers))
    if n * n != total_numbers:
        raise ValueError(f"File does not contain a perfect square number of integers. Found {total_numbers} numbers.")
    
    # Determine subgrid size: the sudoku grid must be n = k^2.
    subgrid_size = int(math.sqrt(n))
    if subgrid_size * subgrid_size != n:
        raise ValueError(f"The sudoku grid size {n} is not a perfect square (n must be k^2).")
    
    # Build the n x n board
    board = [numbers[i * n:(i + 1) * n] for i in range(n)]
    return board, n, subgrid_size

def is_valid_sudoku(board, n, subgrid_size):
    """
    Checks if the given sudoku board is a valid solution.
    
    A valid solution has:
      - Each row contains every number from 1 to n exactly once.
      - Each column contains every number from 1 to n exactly once.
      - Each subgrid (of size subgrid_size x subgrid_size) contains every number from 1 to n exactly once.
    
    Returns:
        True if the sudoku solution is valid, False otherwise.
    """
    expected = set(range(1, n + 1))
    
    # Check rows
    for i, row in enumerate(board):
        if set(row) != expected:
            print(f"Row {i} is invalid.")
            return False
    
    # Check columns
    for col in range(n):
        column_values = {board[row][col] for row in range(n)}
        if column_values != expected:
            print(f"Column {col} is invalid.")
            return False
    
    # Check subgrids
    for block_row in range(0, n, subgrid_size):
        for block_col in range(0, n, subgrid_size):
            block = []
            for i in range(block_row, block_row + subgrid_size):
                for j in range(block_col, block_col + subgrid_size):
                    block.append(board[i][j])
            if set(block) != expected:
                print(f"Subgrid starting at ({block_row}, {block_col}) is invalid.")
                return False

    return True


def main():
    parser = argparse.ArgumentParser(
        description="Verify a sudoku solution from a file. The file must contain a perfect square number of integers (n*n), where n is the grid size."
    )
    parser.add_argument("input_file", help="Path to the input file")
    args = parser.parse_args()
    try:
        board, n, subgrid_size = read_sudoku_from_file(args.input_file)
        count = {}
        for i in board[0]:
            if i in count:
                count[i] += 1
            else:
                count[i] = 1
        for i in count:
            if count[i] > 1:
                print("Duplicate number found", i, count[i])

        if is_valid_sudoku(board, n, subgrid_size):
            print(f"The {n}x{n} sudoku grid is valid.")
        else:
            print(f"The {n}x{n} sudoku grid is not valid.")
    except Exception as e:
        print("Error reading sudoku file:", e)


if __name__ == '__main__':
    main()    
