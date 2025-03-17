n?=9
rr?=0.5
generate_sudoku: sudoku_generator.exe
	echo "Generating $(n)x$(n) Sudoku"
	./sudoku_generator.exe $(n) $(rr)
	
sudoku_generator.exe: sudoku_generator.c
	echo "Compiling and generating executable."
	gcc -O2 sudoku_generator.c -lm -o sudoku_generator.exe

clean:
	rm sudoku_generator.exe sudoku_puzzle*.txt sudoku_solution*.txt
