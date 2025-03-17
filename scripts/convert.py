def convert_to_c_array(filename):
    with open(filename, "r") as file:
        lines = file.readlines()

    # Convert each line into a list of integers
    grid = [list(map(int, line.split())) for line in lines]

    N = len(grid)  # Detect the size dynamically
    print(f"int grid[{N}][{N}] = {{")

    for i, row in enumerate(grid):
        formatted_row = ", ".join(f"{num:2}" for num in row)  # Format with spacing for readability
        if i < len(grid) - 1:
            print(f"    {{ {formatted_row} }},")
        else:
            print(f"    {{ {formatted_row} }}")

    print("};")

# Usage Example
convert_to_c_array("25x25easy.txt")  # Change "sudoku.txt" to your file name
