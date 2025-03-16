from bs4 import BeautifulSoup
import argparse
import re

def parse_sudoku(html):
    soup = BeautifulSoup(html, "html.parser")
    td_elements = soup.find_all("td", id=re.compile(r"cell_\d+_\d+"))
    
    max_row = 0
    max_col = 0
    cell_values = {}
    for td in td_elements:
        cell_id = td.get("id")
        match = re.match(r"cell_(\d+)_(\d+)", cell_id)
        if match:
            row = int(match.group(1))
            col = int(match.group(2))
            max_row = max(max_row, row)
            max_col = max(max_col, col)
            text = td.get_text(strip=True)
            cell_values[(row, col)] = text if text != "" else "0"
    
    # Build the grid line by line
    grid_lines = []
    for i in range(max_row + 1):
        row_vals = []
        for j in range(max_col + 1):
            row_vals.append(cell_values.get((i, j), "0"))
        grid_lines.append(" ".join(row_vals))
    
    return "\n".join(grid_lines)

def main():
    parser = argparse.ArgumentParser(
        description="Convert sudoku grid from HTML to text format."
    )
    parser.add_argument("input_file", help="Path to the input HTML file")
    parser.add_argument("output_file", help="Path to the output text file")
    args = parser.parse_args()

    with open(args.input_file, "r", encoding="utf-8") as f:
        html_content = f.read()

    grid_text = parse_sudoku(html_content)

    with open(args.output_file, "w", encoding="utf-8") as f:
        f.write(grid_text)

if __name__ == '__main__':
    main()
